/**
 * custom_nested_validator.cpp — PC C++ Custom Nested Interface Validator
 * =========================================================================
 * Tests receiving custom nested message `z_RobotDiagnostic` and invoking custom
 * nested service `z_ConfigureRobot` against the MCU live.
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

#include <zenoh_ros/ZenohRosPC.h>

#include <zenoh_ros/custom_msgs/z_RobotDiagnostic.h>
#include <zenoh_ros/custom_srvs/z_ConfigureRobot.h>

std::atomic<bool> received_msg{false};
std::atomic<bool> service_success{false};

class CustomNestedCppValidator : public ZenohNode {
public:
    CustomNestedCppValidator() : ZenohNode("cpp_custom_nested_validator") {
        z_create_subscription<z_RobotDiagnostic>("robot/diagnostics", [](const z_RobotDiagnostic& msg) {
            if (!received_msg.exchange(true)) {
                std::cout << "\n\033[32m\033[1m✓ PASS  [PC C++ Nested Custom Msg Received]\033[0m\n";
                std::cout << "  Robot Name : " << msg.robot_name << "\n";
                std::cout << "  Header     : frame_id='" << msg.header.frame_id << "' sec=" << msg.header.stamp.sec << "\n";
                std::cout << "  Velocity   : [" << msg.velocity.x << ", " << msg.velocity.y << ", " << msg.velocity.z << "]\n";
                std::cout << "  Left Motor : ID=" << msg.left_motor.motor_id << " speed=" << msg.left_motor.speed << "\n";
                std::cout << "  Right Motor: ID=" << msg.right_motor.motor_id << " speed=" << msg.right_motor.speed << "\n";
            }
        });

        cli_config_ = z_create_client<z_ConfigureRobot>("robot/configure");
    }

    bool call_configure_service() {
        z_ConfigureRobot::Request req;
        req.target_status.motor_id = 1;
        req.target_status.speed = 120.0f;
        req.target_status.temperature = 38.5f;
        req.target_status.is_active = true;
        req.mode = 2;

        z_ConfigureRobot::Response res;
        bool ok = cli_config_->call(req, res, 3000);
        if (ok && res.success) {
            service_success = true;
            std::cout << "\n\033[32m\033[1m✓ PASS  [PC C++ Nested Custom Service Response]\033[0m\n";
            std::cout << "  Success       : " << (res.success ? "true" : "false") << "\n";
            std::cout << "  Status Message: '" << res.status_message << "'\n";
            return true;
        } else {
            std::cout << "\n\033[31m✗ FAIL  Service call failed\033[0m\n";
            return false;
        }
    }

private:
    ZenohClient<z_ConfigureRobot>* cli_config_ = nullptr;
};

int main(int argc, char** argv) {
    std::cout << "\n==========================================\n";
    std::cout << "  PC C++ Custom Nested Interface Validator\n";
    std::cout << "==========================================\n\n";

    ZenohConfig cfg;
    cfg.host = (argc > 1) ? argv[1] : "10.42.0.50";
    cfg.port = 7447;

    if (!ZenohNode::init(cfg)) {
        std::cerr << "Zenoh init failed!\n";
        return 1;
    }

    CustomNestedCppValidator node;

    std::thread spin_thread([&]() {
        node.z_spin();
    });

    auto start = std::chrono::steady_clock::now();
    bool srv_called = false;

    while (!received_msg || !service_success) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (received_msg && !srv_called) {
            srv_called = true;
            node.call_configure_service();
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 15) {
            std::cout << "\nTimed out after 15s!\n";
            break;
        }
    }

    std::cout << "\n==========================================\n";
    if (received_msg && service_success) {
        std::cout << "  \033[32m\033[1m✓ PC C++ NESTED CUSTOM INTERFACES VERIFIED SUCCESSFULLY!\033[0m\n";
    } else {
        std::cout << "  \033[31m\033[1m⚠  PC C++ TEST FAILED\033[0m\n";
    }
    std::cout << "==========================================\n\n";

    spin_thread.detach();
    return 0;
}
