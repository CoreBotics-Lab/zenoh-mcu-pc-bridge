/**
 * echo.cpp - Universal C++ topic echo CLI tool for zenoh_ros.
 *
 * Usage:
 *   ./shared_libraries/tools/build/echo <topic_or_node_log_topic> [connect_ip]
 *
 * Examples:
 *   // Subscribe to specific node's log topic with auto-scouting:
 *   ./shared_libraries/tools/build/echo ws2812b_service_server/log
 *
 *   // Subscribe to any custom or pre-defined topic:
 *   ./shared_libraries/tools/build/echo robot/mpu6050
 *
 *   // Connect to specific IP (e.g. ESP32 AP or STA IP):
 *   ./shared_libraries/tools/build/echo ws2812b_service_server/log 192.168.4.1
 *   ./shared_libraries/tools/build/echo /zenoh_ros/log 192.168.1.105
 */

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <csignal>
#include <nlohmann/json.hpp>
#include <zenoh_ros/ZenohRosPC.h>
#include <zenoh_ros/z_logger.h>

static bool g_running = true;

void signal_handler_echo(int signum) {
    (void)signum;
    g_running = false;
}

void render_payload(const uint8_t* data, size_t len) {
    std::vector<uint8_t> buf(data, data + len);

    // Try MsgPack JSON decoding
    try {
        nlohmann::json j = nlohmann::json::from_msgpack(buf);

        // Check if this payload is a ZenohLogger structured record
        if (j.contains("severity") && j.contains("message")) {
            std::string severity = j.value("severity", "INFO");
            std::string name     = j.value("name", "unknown");
            std::string message  = j.value("message", "");
            uint64_t ts_ns       = j.value("timestamp_ns", (uint64_t)0);

            double sec = static_cast<double>(ts_ns) / 1e9;

            const char* color = "\033[37m";
            if (severity == "DEBUG")      color = "\033[36m";
            else if (severity == "WARN")  color = "\033[33m";
            else if (severity == "ERROR") color = "\033[31m";
            else if (severity == "FATAL") color = "\033[1;31m";

            char ts_buf[64];
            snprintf(ts_buf, sizeof(ts_buf), "%.9f", sec);

            std::cout << color << "[" << severity << "] [" << ts_buf << "] [" << name << "]: " 
                      << message << "\033[0m\n";
            return;
        }

        // Generic MsgPack JSON dump
        std::cout << "\033[36m[DATA]\033[0m " << j.dump(2) << "\n";
        return;
    }
    catch (...) {
        // Not MsgPack JSON
    }

    // Print as ASCII text if printable
    bool is_printable = true;
    for (size_t i = 0; i < len; ++i) {
        if (!isprint(data[i]) && data[i] != '\n' && data[i] != '\r' && data[i] != '\t') {
            is_printable = false;
            break;
        }
    }

    if (is_printable && len > 0) {
        std::cout << "\033[36m[TEXT]\033[0m " << std::string((const char*)data, len) << "\n";
        return;
    }

    // Hex dump fallback
    std::cout << "\033[36m[HEX (" << len << " bytes)]\033[0m ";
    for (size_t i = 0; i < len; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
    }
    std::cout << std::dec << "\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "\033[33mUsage: zenoh_ros echo <topic_name> [ip_address]\033[0m\n";
        std::cout << "Examples:\n";
        std::cout << "  ./shared_libraries/tools/build/echo ws2812b_service_server/log\n";
        std::cout << "  ./shared_libraries/tools/build/echo /zenoh_ros/log\n";
        std::cout << "  ./shared_libraries/tools/build/echo robot/mpu6050 192.168.4.1\n";
        return 1;
    }

    std::string target_topic = argv[1];
    // Remove leading slash if any for consistency
    std::string topic_clean = target_topic;

    signal(SIGINT, signal_handler_echo);
    signal(SIGTERM, signal_handler_echo);

    ZenohConfig config;
    if (argc >= 3) {
        config.host = argv[2];
        config.port = 7447;
        std::cout << "\033[36m[zenoh_ros echo] Connecting to specified endpoint: tcp/" << config.host << ":7447...\033[0m\n";
    } else {
        // Modular IP: auto-scout mode (works across STA and AP modes without hardcoded IP)
        config.connect_endpoint = "";
        std::cout << "\033[36m[zenoh_ros echo] Auto-scouting network peers (STA/AP modular mode)...\033[0m\n";
    }

    if (!ZenohNode::init(config)) {
        std::cerr << "\033[31m[zenoh_ros echo] ERROR: Failed to initialize Zenoh session!\033[0m\n";
        return 1;
    }

    ZenohNode node("zenoh_ros_echo");

    std::cout << "\033[32m[zenoh_ros echo] Subscribed live on '" << topic_clean << "'. Press Ctrl+C to exit.\033[0m\n\n";

    z_owned_closure_sample_t closure;
    z_closure_sample(
        &closure,
        [](z_loaned_sample_t* sample, void* context) {
            (void)context;
            const z_loaned_bytes_t* payload = z_sample_payload(sample);
            if (!payload) return;

            z_owned_slice_t slice;
            z_bytes_to_slice(payload, &slice);
            const uint8_t* data = z_slice_data(z_slice_loan(&slice));
            size_t len = z_slice_len(z_slice_loan(&slice));

            render_payload(data, len);

            z_slice_drop(z_slice_move(&slice));
        },
        NULL,
        NULL
    );

    z_owned_subscriber_t sub;
    z_view_keyexpr_t keyexpr;
    z_view_keyexpr_from_str(&keyexpr, topic_clean.c_str());

    if (z_declare_subscriber(ZenohNode::get_session(), &sub, z_view_keyexpr_loan(&keyexpr), z_closure_sample_move(&closure), NULL) < 0) {
        std::cerr << "\033[31m[zenoh_ros echo] ERROR: Failed to declare subscriber on '" << topic_clean << "'!\033[0m\n";
        ZenohNode::shutdown();
        return 1;
    }

    while (g_running) {
        z_sleep_ms(100);
    }

    std::cout << "\n[zenoh_ros echo] Closing subscriber...\n";
    z_undeclare_subscriber(z_subscriber_move(&sub));
    ZenohNode::shutdown();
    return 0;
}
