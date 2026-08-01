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

        std::vector<uint8_t> buf(data, data + len);
        z_slice_drop(z_slice_move(&slice));

        try {
            nlohmann::json j = nlohmann::json::from_msgpack(buf);
            if (j.is_object() && j.contains("severity") && j.contains("message")) {
                std::string severity = j.value("severity", "INFO");
                std::string name     = j.value("name", "node");
                std::string message  = j.value("message", "");
                uint64_t ts_ns       = j.value("timestamp_ns", (uint64_t)0);
                double sec           = static_cast<double>(ts_ns) / 1e9;

                const char* color = "\033[37m";
                if (severity == "DEBUG")      color = "\033[36m";
                else if (severity == "WARN")  color = "\033[33m";
                else if (severity == "ERROR") color = "\033[31m";
                else if (severity == "FATAL") color = "\033[1;31m";

                char ts_buf[64];
                snprintf(ts_buf, sizeof(ts_buf), "%.9f", sec);

                std::cerr << color << "[" << severity << "] [" << ts_buf << "] [" << name << "]: " 
                          << message << "\033[0m\n";
            } else {
                std::cout << j.dump(2) << "\n";
            }
        }
        catch (...) {
            std::string text(reinterpret_cast<const char*>(data), len);
            std::cout << text << "\n";
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

    if (host && std::string(host).length() > 0) {
        std::string endpoint = std::string("[\"tcp/") + host + ":7447\"]";
        zc_config_insert_json5(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, endpoint.c_str());
        std::cout << "[echo] Connecting directly to " << host << ":7447...\n";
    } else {
        std::cout << "[echo] Auto-discovering peers on network...\n";
    }

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
