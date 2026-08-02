/**
 * echo.cpp — Shared C++ CLI Echo Tool for zenoh_ros
 *
 * Subscribes to any topic (e.g. 'ws2812b_service_server/log', 'robot/mpu6050')
 * and prints live structured payloads with color formatting.
 */

#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <nlohmann/json.hpp>
#include <zenoh.h>

static bool g_running = true;

void echo_signal_handler(int signum) {
    (void)signum;
    g_running = false;
}

void echo_sample_callback(z_loaned_sample_t* sample, void* arg) {
    (void)arg;
    try {
        const z_loaned_bytes_t* payload = z_sample_payload(sample);
        if (!payload) return;

        z_owned_slice_t slice;
        z_bytes_to_slice(payload, &slice);
        const uint8_t* data = z_slice_data(z_slice_loan(&slice));
        size_t len = z_slice_len(z_slice_loan(&slice));

        std::string text(reinterpret_cast<const char*>(data), len);
        z_slice_drop(z_slice_move(&slice));

        // 1. Check if payload is a formatted log string: "[SEVERITY] [name]: message"
        if (text.size() > 2 && text[0] == '[') {
            const char* color = "\033[37m"; // Default white (INFO)
            if (text.rfind("[DEBUG]", 0) == 0)      color = "\033[36m";     // Cyan
            else if (text.rfind("[INFO]", 0) == 0)  color = "\033[37m";     // White
            else if (text.rfind("[WARN]", 0) == 0)  color = "\033[33m";     // Yellow
            else if (text.rfind("[ERROR]", 0) == 0) color = "\033[31m";     // Red
            else if (text.rfind("[FATAL]", 0) == 0) color = "\033[1;31m";   // Bold Red

            std::cout << color << text << "\033[0m\n";
            return;
        }

        // 2. Try MsgPack JSON formatting for structured messages (e.g. sensor_msgs)
        try {
            nlohmann::json j = nlohmann::json::from_msgpack(data, data + len, true, false);
            if (!j.is_discarded() && !j.is_null()) {
                std::cout << j.dump(2) << "\n";
                return;
            }
        }
        catch (...) {}

        // Fallback: Check if string has printable characters
        bool is_printable = true;
        for (size_t i = 0; i < len; ++i) {
            if (data[i] < 32 && data[i] != '\n' && data[i] != '\r' && data[i] != '\t') {
                is_printable = false;
                break;
            }
        }

        if (is_printable) {
            std::cout << text << "\n";
        } else {
            std::cout << "\033[33m[binary payload: " << len << " bytes]\033[0m\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "\033[31m[echo error] " << e.what() << "\033[0m\n";
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: echo <topic_name> [host_ip]\n";
        std::cout << "Example: echo ws2812b_service_server/log\n";
        std::cout << "Example: echo robot/mpu6050\n";
        return 1;
    }

    std::string topic = argv[1];
    const char* host = (argc > 2) ? argv[2] : nullptr;

    signal(SIGINT, echo_signal_handler);
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

    if (z_declare_subscriber(z_session_loan(&session), &sub, z_view_keyexpr_loan(&keyexpr), z_closure_sample_move(&closure), NULL) < 0) {
        std::cerr << "\033[31m[echo] ERROR: Failed to declare subscriber on '" << topic << "'!\033[0m\n";
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
