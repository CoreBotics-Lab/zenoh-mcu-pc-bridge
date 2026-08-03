/**
 * rigorous_nested_validator.cpp — PC C++ Rigorous Multi-Level Custom Interface Validator
 * ======================================================================================
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

#include <zenoh_ros/ZenohRosPC.h>

#include <zenoh_ros/custom_msgs/z_ComplexRobotState.h>
#include <zenoh_ros/custom_srvs/z_FullSystemControl.h>

std::atomic<bool> received_msg{false};
std::atomic<bool> service_success{false};

class RigorousCppValidator : public ZenohNode {
public:
    RigorousCppValidator() : ZenohNode("cpp_rigorous_validator") {
        z_create_subscription<z_ComplexRobotState>("system/state", [](const z_ComplexRobotState& msg) {
            if (!received_msg.exchange(true)) {
                std::cout << "\n\033[32m\033[1m✓ PASS  [PC C++ Multi-Level Custom Msg Received]\033[0m\n";
                std::cout << "  Robot Mode : " << msg.robot_mode << " (Cycle: " << msg.cycle_count << ")\n";
                std::cout << "  Header     : frame_id='" << msg.header.frame_id << "' sec=" << msg.header.stamp.sec << "\n";
                std::cout << "  Pose       : Pos=[" << msg.pose.position.x << ", " << msg.pose.position.y << ", " << msg.pose.position.z << "]\n";
                std::cout << "  Telemetry  : Sensor ID=" << msg.telemetry.sensor_id << " Temp=" << msg.telemetry.temp << " StatusOK=" << (msg.telemetry.status_ok ? "true" : "false") << "\n";
                std::cout << "               Telemetry Header frame_id='" << msg.telemetry.header.frame_id << "'\n";
                std::cout << "               Accel=[" << msg.telemetry.accel.x << ", " << msg.telemetry.accel.y << ", " << msg.telemetry.accel.z << "]\n";
            }
        });

        cli_control_ = z_create_client<z_FullSystemControl>("system/control");
    }

    bool call_control_service() {
        z_FullSystemControl::Request req;
        req.target_state.robot_mode = "TEST_MODE_EXEC";
        req.target_state.cycle_count = 999;
        req.command_code = 7;

        z_FullSystemControl::Response res;
        bool ok = cli_control_->call(req, res, 3000);
        if (ok && res.ack) {
            service_success = true;
            std::cout << "\n\033[32m\033[1m✓ PASS  [PC C++ Multi-Level Custom Service Response]\033[0m\n";
            std::cout << "  Ack           : " << (res.ack ? "true" : "false") << "\n";
            std::cout << "  Status Details: '" << res.status_details << "'\n";
            std::cout << "  Curr Telemetry: Sensor ID=" << res.current_telemetry.sensor_id << " Temp=" << res.current_telemetry.temp << "\n";
            return true;
        } else {
            std::cout << "\n\033[31m✗ FAIL  Service call failed\033[0m\n";
            return false;
        }
    }

private:
    ZenohClient<z_FullSystemControl>* cli_control_ = nullptr;
};

int main(int argc, char** argv) {
    std::cout << "\n==========================================\n";
    std::cout << "  PC C++ Rigorous Multi-Level Custom Validator\n";
    std::cout << "==========================================\n\n";

    ZenohConfig cfg;
    cfg.host = (argc > 1) ? argv[1] : "10.42.0.50";
    cfg.port = 7447;

    if (!ZenohNode::init(cfg)) {
        std::cerr << "Zenoh init failed!\n";
        return 1;
    }

    RigorousCppValidator node;

    std::thread spin_thread([&]() {
        node.z_spin();
    });

    auto start = std::chrono::steady_clock::now();
    bool srv_called = false;

    while (!received_msg || !service_success) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (received_msg && !srv_called) {
            srv_called = true;
            srv_called = true;
            node.call_control_service();
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 15) {
            std::cout << "\nTimed out after 15s!\n";
            break;
        }
    }

    std::cout << "\n==========================================\n";
    if (received_msg && service_success) {
        std::cout << "  \033[32m\033[1m✓ PC C++ MULTI-LEVEL CUSTOM INTERFACES VERIFIED 100%!\033[0m\n";
    } else {
        std::cout << "  \033[31m\033[1m⚠  PC C++ TEST FAILED\033[0m\n";
    }
    std::cout << "==========================================\n\n";

    spin_thread.detach();
    return 0;
}
