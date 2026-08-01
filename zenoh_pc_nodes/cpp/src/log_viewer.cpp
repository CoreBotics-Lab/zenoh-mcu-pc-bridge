/**
 * log_viewer.cpp - High-performance C++ CLI node to monitor live logs published on 'zenoh_ros/log'.
 */

#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <zenoh_ros/ZenohRosPC.h>
#include <zenoh_ros/z_logger.h>

int main(int argc, char** argv) {
    std::cout << "\033[36m==========================================\033[0m\n";
    std::cout << "\033[36m  zenoh_ros C++ High-Performance Log Viewer\033[0m\n";
    std::cout << "\033[36m==========================================\033[0m\n";

    ZenohConfig config;
    config.host = (argc > 1) ? argv[1] : "192.168.4.1";
    config.port = 7447;

    if (!ZenohNode::init(config)) {
        std::cerr << "\033[31m[LOG VIEWER C++] ERROR: Failed to initialize Zenoh session!\033[0m\n";
        return 1;
    }

    std::cout << "\033[36m[LOG VIEWER C++] Connected to " << config.host << "! Listening live on 'zenoh_ros/log'...\033[0m\n\n";

    ZenohNode node("log_viewer_cpp");

    z_owned_closure_sample_t closure;
    z_closure_sample(
        &closure,
        [](z_loaned_sample_t* sample, void* context) {
            (void)context;
            try {
                const z_loaned_bytes_t* payload = z_sample_payload(sample);
                if (!payload) return;

                z_owned_slice_t slice;
                z_bytes_to_slice(payload, &slice);
                const uint8_t* data = z_slice_data(z_slice_loan(&slice));
                size_t len = z_slice_len(z_slice_loan(&slice));

                std::vector<uint8_t> buf(data, data + len);
                z_slice_drop(z_slice_move(&slice));

                nlohmann::json j = nlohmann::json::from_msgpack(buf);

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

                std::cerr << color << "[" << severity << "] [" << ts_buf << "] [" << name << "]: " 
                          << message << "\033[0m\n";
            }
            catch (const std::exception& e) {
                std::cerr << "\033[31m[LOG VIEWER C++ ERROR] Parse error: " << e.what() << "\033[0m\n";
            }
        },
        NULL,
        NULL
    );

    z_owned_subscriber_t sub;
    z_view_keyexpr_t keyexpr;
    z_view_keyexpr_from_str(&keyexpr, "zenoh_ros/log");

    if (z_declare_subscriber(ZenohNode::get_session(), &sub, z_view_keyexpr_loan(&keyexpr), z_closure_sample_move(&closure), NULL) < 0) {
        std::cerr << "\033[31m[LOG VIEWER C++] ERROR: Failed to declare subscriber on 'zenoh_ros/log'!\033[0m\n";
        ZenohNode::shutdown();
        return 1;
    }

    // Spin using the framework's spin loop (which handles Ctrl+C cleanly)
    node.z_spin();

    z_undeclare_subscriber(z_subscriber_move(&sub));
    ZenohNode::shutdown();
    std::cout << "[LOG VIEWER C++] Session closed cleanly.\n";

    return 0;
}
