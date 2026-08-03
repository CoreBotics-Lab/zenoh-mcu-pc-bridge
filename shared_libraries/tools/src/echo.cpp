/**
 * echo.cpp — Shared C++ CLI Echo Tool for zenoh_ros
 *
 * Subscribes to any topic (e.g. 'ws2812b_service_server/log', 'robot/mpu6050')
 * and prints live structured payloads with colour formatting.
 *
 * Payload decoding priority:
 *   1. Plain text log strings  → coloured text output
 *   2. MessagePack binary      → pretty-printed JSON (standalone decoder, no deps)
 *   3. Printable ASCII         → raw text
 *   4. Unknown binary          → "[binary payload: N bytes]"
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <csignal>
#include <zenoh.h>

static bool g_running = true;

void echo_signal_handler(int signum) {
    (void)signum;
    g_running = false;
}

// ─── Standalone MessagePack → JSON converter ────────────────────────────────
// Implements the complete MessagePack spec.  No external libraries required.

struct MsgpackReader {
    const uint8_t* buf;
    size_t         len;
    size_t         pos;

    bool eof() const { return pos >= len; }

    uint8_t read_u8() {
        if (pos >= len) throw std::runtime_error("msgpack: truncated u8");
        return buf[pos++];
    }
    uint16_t read_u16() {
        if (pos + 2 > len) throw std::runtime_error("msgpack: truncated u16");
        uint16_t v = (uint16_t(buf[pos]) << 8) | buf[pos + 1];
        pos += 2; return v;
    }
    uint32_t read_u32() {
        if (pos + 4 > len) throw std::runtime_error("msgpack: truncated u32");
        uint32_t v = (uint32_t(buf[pos]) << 24) | (uint32_t(buf[pos+1]) << 16)
                   | (uint32_t(buf[pos+2]) << 8)  |  uint32_t(buf[pos+3]);
        pos += 4; return v;
    }
    uint64_t read_u64() {
        if (pos + 8 > len) throw std::runtime_error("msgpack: truncated u64");
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i) v = (v << 8) | buf[pos++];
        return v;
    }
    float read_f32() {
        uint32_t bits = read_u32();
        float v; std::memcpy(&v, &bits, 4); return v;
    }
    double read_f64() {
        uint64_t bits = read_u64();
        double v; std::memcpy(&v, &bits, 8); return v;
    }
    std::string read_str(size_t n) {
        if (pos + n > len) throw std::runtime_error("msgpack: truncated str");
        std::string s(reinterpret_cast<const char*>(buf + pos), n);
        pos += n; return s;
    }
    void skip(size_t n) {
        if (pos + n > len) throw std::runtime_error("msgpack: truncated skip");
        pos += n;
    }
};

// Forward declaration
static std::string msgpack_to_json(MsgpackReader& r, int indent, int depth);

static std::string json_indent(int indent, int depth) {
    if (indent < 0) return "";
    return std::string(size_t(indent * depth), ' ');
}

static std::string json_string(const std::string& s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else if (c < 0x20) {
            char buf[8];
            snprintf(buf, sizeof(buf), "\\u%04x", c);
            out += buf;
        } else {
            out += c;
        }
    }
    return out + "\"";
}

static std::string json_double(double v) {
    if (std::isnan(v))  return "null";
    if (std::isinf(v))  return v > 0 ? "1e308" : "-1e308";
    std::ostringstream ss;
    ss << std::setprecision(10) << v;
    std::string s = ss.str();
    if (s.find('.') == std::string::npos && s.find('e') == std::string::npos)
        s += ".0";
    return s;
}

static std::string parse_array(MsgpackReader& r, uint32_t n, int indent, int depth) {
    std::string nl    = (indent >= 0) ? "\n" : "";
    std::string inner = json_indent(indent, depth + 1);
    std::string outer = json_indent(indent, depth);

    if (n == 0) return "[]";

    std::string out = "[" + nl;
    for (uint32_t i = 0; i < n; ++i) {
        out += inner + msgpack_to_json(r, indent, depth + 1);
        if (i + 1 < n) out += ",";
        out += nl;
    }
    return out + outer + "]";
}

static std::string parse_map(MsgpackReader& r, uint32_t n, int indent, int depth) {
    std::string nl    = (indent >= 0) ? "\n" : "";
    std::string inner = json_indent(indent, depth + 1);
    std::string outer = json_indent(indent, depth);
    std::string colon = (indent >= 0) ? ": " : ":";

    if (n == 0) return "{}";

    std::string out = "{" + nl;
    for (uint32_t i = 0; i < n; ++i) {
        // Key
        std::string key;
        uint8_t kb = r.buf[r.pos];
        if ((kb & 0xe0) == 0xa0) {          // fixstr
            r.pos++;
            key = json_string(r.read_str(kb & 0x1f));
        } else if (kb == 0xd9) {            // str8
            r.pos++;
            key = json_string(r.read_str(r.read_u8()));
        } else if (kb == 0xda) {            // str16
            r.pos++;
            key = json_string(r.read_str(r.read_u16()));
        } else if (kb == 0xdb) {            // str32
            r.pos++;
            key = json_string(r.read_str(r.read_u32()));
        } else {
            // Non-string key: parse and wrap in quotes
            std::string v = msgpack_to_json(r, -1, 0);
            key = "\"" + v + "\"";
        }

        // Value
        std::string val = msgpack_to_json(r, indent, depth + 1);

        out += inner + key + colon + val;
        if (i + 1 < n) out += ",";
        out += nl;
    }
    return out + outer + "}";
}

static std::string msgpack_to_json(MsgpackReader& r, int indent, int depth) {
    if (r.eof()) throw std::runtime_error("msgpack: unexpected end of data");

    uint8_t b = r.read_u8();

    if (b <= 0x7f)          return std::to_string(b);                     // positive fixint
    if ((b & 0xf0) == 0x80) return parse_map(r, b & 0x0f, indent, depth); // fixmap
    if ((b & 0xf0) == 0x90) return parse_array(r, b & 0x0f, indent, depth);// fixarray
    if ((b & 0xe0) == 0xa0) return json_string(r.read_str(b & 0x1f));     // fixstr
    if (b >= 0xe0)          return std::to_string(static_cast<int8_t>(b)); // negative fixint

    switch (b) {
        case 0xc0: return "null";
        case 0xc2: return "false";
        case 0xc3: return "true";

        case 0xc4: { uint8_t  n = r.read_u8();  r.skip(n); return "\"<bin>\""; }
        case 0xc5: { uint16_t n = r.read_u16(); r.skip(n); return "\"<bin>\""; }
        case 0xc6: { uint32_t n = r.read_u32(); r.skip(n); return "\"<bin>\""; }

        case 0xca: return json_double(r.read_f32());
        case 0xcb: return json_double(r.read_f64());

        case 0xcc: return std::to_string(r.read_u8());
        case 0xcd: return std::to_string(r.read_u16());
        case 0xce: return std::to_string(r.read_u32());
        case 0xcf: return std::to_string(r.read_u64());

        case 0xd0: return std::to_string(static_cast<int8_t>(r.read_u8()));
        case 0xd1: return std::to_string(static_cast<int16_t>(r.read_u16()));
        case 0xd2: return std::to_string(static_cast<int32_t>(r.read_u32()));
        case 0xd3: return std::to_string(static_cast<int64_t>(r.read_u64()));

        // fixext
        case 0xd4: { r.skip(2);  return "\"<ext1>\""; }
        case 0xd5: { r.skip(3);  return "\"<ext2>\""; }
        case 0xd6: { r.skip(5);  return "\"<ext4>\""; }
        case 0xd7: { r.skip(9);  return "\"<ext8>\""; }
        case 0xd8: { r.skip(17); return "\"<ext16>\""; }

        case 0xd9: return json_string(r.read_str(r.read_u8()));
        case 0xda: return json_string(r.read_str(r.read_u16()));
        case 0xdb: return json_string(r.read_str(r.read_u32()));

        case 0xdc: return parse_array(r, r.read_u16(), indent, depth);
        case 0xdd: return parse_array(r, r.read_u32(), indent, depth);

        case 0xde: return parse_map(r, r.read_u16(), indent, depth);
        case 0xdf: return parse_map(r, r.read_u32(), indent, depth);

        case 0xc7: { uint8_t  n = r.read_u8();  r.read_u8(); r.skip(n); return "\"<ext>\""; }
        case 0xc8: { uint16_t n = r.read_u16(); r.read_u8(); r.skip(n); return "\"<ext>\""; }
        case 0xc9: { uint32_t n = r.read_u32(); r.read_u8(); r.skip(n); return "\"<ext>\""; }

        default:
            throw std::runtime_error("msgpack: unknown type byte");
    }
}

/**
 * Try to decode data[offset..len] as a single top-level MessagePack value.
 * Returns JSON string on success, empty on failure.
 * Only accepts map or array at root level (structured message check).
 */
static std::string try_msgpack_decode(const uint8_t* data, size_t len, size_t offset, int indent) {
    if (offset >= len) return "";
    uint8_t first = data[offset];
    // Only try if the root looks like a map or array (structured data)
    bool looks_structured = ((first & 0xf0) == 0x80) || ((first & 0xf0) == 0x90)
                         || (first == 0xde) || (first == 0xdf)
                         || (first == 0xdc) || (first == 0xdd);
    if (!looks_structured) return "";

    MsgpackReader r{data, len, offset};
    try {
        std::string json = msgpack_to_json(r, indent, 0);
        // Accept if we consumed at least 80% of the remaining bytes
        size_t remaining = len - offset;
        if (r.pos - offset >= remaining * 8 / 10)
            return json;
    } catch (...) {}
    return "";
}

// ─── Zenoh sample callback ───────────────────────────────────────────────────

void echo_sample_callback(z_loaned_sample_t* sample, void* arg) {
    (void)arg;
    try {
        const z_loaned_bytes_t* payload = z_sample_payload(sample);
        if (!payload) return;

        z_owned_slice_t slice;
        z_bytes_to_slice(payload, &slice);
        size_t len = z_slice_len(z_slice_loan(&slice));
        // Copy into std::string immediately; raw pointer becomes invalid after drop
        std::string text(reinterpret_cast<const char*>(z_slice_data(z_slice_loan(&slice))), len);
        z_slice_drop(z_slice_move(&slice));
        // Use text's buffer for binary decoding (owns the data, always valid)
        const uint8_t* data = reinterpret_cast<const uint8_t*>(text.data());

        // 1. Plain-text log string: "[SEVERITY] [name]: message"
        if (text.size() > 2 && text[0] == '[') {
            const char* color = "\033[37m";
            if      (text.rfind("[DEBUG]", 0) == 0) color = "\033[36m";
            else if (text.rfind("[INFO]",  0) == 0) color = "\033[37m";
            else if (text.rfind("[WARN]",  0) == 0) color = "\033[33m";
            else if (text.rfind("[ERROR]", 0) == 0) color = "\033[31m";
            else if (text.rfind("[FATAL]", 0) == 0) color = "\033[1;31m";
            std::cout << color << text << "\033[0m\n";
            return;
        }

        // 2. Standalone MessagePack decoder
        //    Try offset 0, then scan 1..16 (handles small framing prefixes)
        {
            std::string json = try_msgpack_decode(data, len, 0, 2);
            if (!json.empty()) { std::cout << json << "\n"; return; }

            for (size_t off = 1; off <= 16 && off < len; ++off) {
                json = try_msgpack_decode(data, len, off, 2);
                if (!json.empty()) { std::cout << json << "\n"; return; }
            }
        }

        // 3. Printable ASCII fallback
        bool is_printable = true;
        for (size_t i = 0; i < len; ++i) {
            if (data[i] < 32 && data[i] != '\n' && data[i] != '\r' && data[i] != '\t') {
                is_printable = false; break;
            }
        }
        if (is_printable) { std::cout << text << "\n"; return; }

        // 4. Unknown binary
        std::cout << "\033[33m[binary payload: " << len << " bytes]\033[0m\n";
    }
    catch (const std::exception& e) {
        std::cerr << "\033[31m[echo error] " << e.what() << "\033[0m\n";
    }
}

// ─── main ────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: echo <topic_name> [host_ip]\n";
        std::cout << "Example: echo ws2812b_service_server/log\n";
        std::cout << "Example: echo robot/mpu6050\n";
        return 1;
    }

    std::string topic = argv[1];
    const char* host  = (argc > 2) ? argv[2] : nullptr;

    signal(SIGINT,  echo_signal_handler);
    signal(SIGTERM, echo_signal_handler);

    std::cout << "\033[36m==========================================\033[0m\n";
    std::cout << "\033[36m  zenoh_ros C++ Echo — Topic: '" << topic << "'\033[0m\n";
    std::cout << "\033[36m==========================================\033[0m\n";

    z_owned_config_t config;
    z_config_default(&config);

    std::string target_host = (host && std::string(host).length() > 0) ? host : "192.168.4.1";
    std::string endpoint = std::string("[\"tcp/") + target_host + ":7447\"]";
    zc_config_insert_json5(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, endpoint.c_str());
    std::cout << "[echo] Connecting to endpoint: " << target_host << ":7447...\n";

    z_owned_session_t session;
    if (z_open(&session, z_config_move(&config), NULL) < 0) {
        std::cerr << "\033[31m[echo] ERROR: Failed to open Zenoh session!\033[0m\n";
        return 1;
    }

    std::cout << "\033[36m[echo] Subscribed live on '" << topic << "'. Press Ctrl+C to exit.\033[0m\n\n";

    z_owned_closure_sample_t closure;
    z_closure_sample(&closure, echo_sample_callback, NULL, NULL);

    z_owned_subscriber_t sub;
    z_view_keyexpr_t keyexpr;
    z_view_keyexpr_from_str(&keyexpr, topic.c_str());

    if (z_declare_subscriber(z_session_loan(&session), &sub,
                             z_view_keyexpr_loan(&keyexpr),
                             z_closure_sample_move(&closure), NULL) < 0) {
        std::cerr << "\033[31m[echo] ERROR: Failed to declare subscriber on '"
                  << topic << "'!\033[0m\n";
        z_close(z_session_loan_mut(&session), NULL);
        return 1;
    }

    while (g_running) {
        z_sleep_ms(100);
    }

    std::cout << "\n[echo] Unsubscribing and shutting down...\n";
    z_undeclare_subscriber(z_subscriber_move(&sub));
    z_close(z_session_loan_mut(&session), NULL);
    std::cout << "[echo] Session closed cleanly.\n";

    return 0;
}
