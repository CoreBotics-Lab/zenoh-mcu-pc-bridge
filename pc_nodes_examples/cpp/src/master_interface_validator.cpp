/**
 * master_interface_validator.cpp — Monolithic All-In-One Interface Validator (C++)
 * =================================================================================
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <set>
#include <mutex>
#include <atomic>

#include <zenoh_ros/ZenohRosPC.h>

// Standard ROS 2 Includes
#include <zenoh_ros/std_msgs/z_Bool.h>
#include <zenoh_ros/std_msgs/z_Int8.h>
#include <zenoh_ros/std_msgs/z_UInt8.h>
#include <zenoh_ros/std_msgs/z_Int16.h>
#include <zenoh_ros/std_msgs/z_UInt16.h>
#include <zenoh_ros/std_msgs/z_Int32.h>
#include <zenoh_ros/std_msgs/z_UInt32.h>
#include <zenoh_ros/std_msgs/z_Int64.h>
#include <zenoh_ros/std_msgs/z_UInt64.h>
#include <zenoh_ros/std_msgs/z_Float32.h>
#include <zenoh_ros/std_msgs/z_Float64.h>
#include <zenoh_ros/std_msgs/z_String.h>
#include <zenoh_ros/std_msgs/z_Header.h>
#include <zenoh_ros/std_msgs/z_Int32MultiArray.h>
#include <zenoh_ros/std_msgs/z_Float64MultiArray.h>
#include <zenoh_ros/sensor_msgs/z_Imu.h>
#include <zenoh_ros/sensor_msgs/z_Temperature.h>
#include <zenoh_ros/sensor_msgs/z_Range.h>
#include <zenoh_ros/sensor_msgs/z_RelativeHumidity.h>
#include <zenoh_ros/sensor_msgs/z_BatteryState.h>
#include <zenoh_ros/sensor_msgs/z_NavSatFix.h>
#include <zenoh_ros/sensor_msgs/z_JoyFeedback.h>
#include <zenoh_ros/sensor_msgs/z_JointState.h>
#include <zenoh_ros/geometry_msgs/z_Vector3.h>
#include <zenoh_ros/geometry_msgs/z_Quaternion.h>
#include <zenoh_ros/geometry_msgs/z_Pose.h>
#include <zenoh_ros/geometry_msgs/z_Twist.h>

// Custom Interface Includes
#include <zenoh_ros/custom_msgs/z_SetLED.h>
#include <zenoh_ros/custom_msgs/z_MPU6050Data.h>
#include <zenoh_ros/custom_msgs/z_RobotState.h>
#include <zenoh_ros/custom_msgs/z_MotorStatus.h>
#include <zenoh_ros/custom_msgs/z_SensorTelemetry.h>
#include <zenoh_ros/custom_msgs/z_RobotDiagnostic.h>
#include <zenoh_ros/custom_msgs/z_ComplexRobotState.h>

#include <zenoh_ros/custom_srvs/z_SetLEDColor.h>
#include <zenoh_ros/custom_srvs/z_SetColor.h>
#include <zenoh_ros/custom_srvs/z_ConfigureRobot.h>
#include <zenoh_ros/custom_srvs/z_FullSystemControl.h>

std::set<std::string> passed_topics;
std::set<std::string> passed_services;
std::atomic<int> topic_count{0};
std::atomic<int> service_count{0};
std::mutex print_mtx;

void record(const std::string& top) {
    std::lock_guard<std::mutex> lock(print_mtx);
    if (passed_topics.find(top) == passed_topics.end()) {
        passed_topics.insert(top);
        topic_count++;
        std::cout << "  \033[32m✓ PASS\033[0m [" << topic_count.load() << "/34] " << top << "\n";
    }
}

class MasterCppValidator : public ZenohNode {
public:
    MasterCppValidator() : ZenohNode("master_cpp_validator") {
        ZLOG_INFO(this->get_logger(), "Validator node initialized");

        z_create_subscription<z_Bool>("test/bool", [](const z_Bool&) { record("test/bool"); });
        z_create_subscription<z_Int8>("test/int8", [](const z_Int8&) { record("test/int8"); });
        z_create_subscription<z_UInt8>("test/uint8", [](const z_UInt8&) { record("test/uint8"); });
        z_create_subscription<z_Int16>("test/int16", [](const z_Int16&) { record("test/int16"); });
        z_create_subscription<z_UInt16>("test/uint16", [](const z_UInt16&) { record("test/uint16"); });
        z_create_subscription<z_Int32>("test/int32", [](const z_Int32&) { record("test/int32"); });
        z_create_subscription<z_UInt32>("test/uint32", [](const z_UInt32&) { record("test/uint32"); });
        z_create_subscription<z_Int64>("test/int64", [](const z_Int64&) { record("test/int64"); });
        z_create_subscription<z_UInt64>("test/uint64", [](const z_UInt64&) { record("test/uint64"); });
        z_create_subscription<z_Float32>("test/float32", [](const z_Float32&) { record("test/float32"); });
        z_create_subscription<z_Float64>("test/float64", [](const z_Float64&) { record("test/float64"); });
        z_create_subscription<z_String>("test/string", [](const z_String&) { record("test/string"); });
        z_create_subscription<z_Header>("test/header", [](const z_Header&) { record("test/header"); });
        z_create_subscription<z_Int32MultiArray>("test/int32_array", [](const z_Int32MultiArray&) { record("test/int32_array"); });
        z_create_subscription<z_Float64MultiArray>("test/float64_array", [](const z_Float64MultiArray&) { record("test/float64_array"); });
        z_create_subscription<z_Imu>("test/imu", [](const z_Imu&) { record("test/imu"); });
        z_create_subscription<z_Temperature>("test/temperature", [](const z_Temperature&) { record("test/temperature"); });
        z_create_subscription<z_Range>("test/range", [](const z_Range&) { record("test/range"); });
        z_create_subscription<z_RelativeHumidity>("test/humidity", [](const z_RelativeHumidity&) { record("test/humidity"); });
        z_create_subscription<z_BatteryState>("test/battery", [](const z_BatteryState&) { record("test/battery"); });
        z_create_subscription<z_NavSatFix>("test/navsatfix", [](const z_NavSatFix&) { record("test/navsatfix"); });
        z_create_subscription<z_JoyFeedback>("test/joyfeedback", [](const z_JoyFeedback&) { record("test/joyfeedback"); });
        z_create_subscription<z_JointState>("test/jointstate", [](const z_JointState&) { record("test/jointstate"); });
        z_create_subscription<z_Vector3>("test/vector3", [](const z_Vector3&) { record("test/vector3"); });
        z_create_subscription<z_Quaternion>("test/quaternion", [](const z_Quaternion&) { record("test/quaternion"); });
        z_create_subscription<z_Pose>("test/pose", [](const z_Pose&) { record("test/pose"); });
        z_create_subscription<z_Twist>("test/twist", [](const z_Twist&) { record("test/twist"); });

        // Custom Topics
        z_create_subscription<z_SetLED>("test_custom/set_led", [](const z_SetLED&) { record("test_custom/set_led"); });
        z_create_subscription<z_MPU6050Data>("test_custom/mpu6050", [](const z_MPU6050Data&) { record("test_custom/mpu6050"); });
        z_create_subscription<z_RobotState>("test_custom/robot_state", [](const z_RobotState&) { record("test_custom/robot_state"); });
        z_create_subscription<z_MotorStatus>("test_custom/motor_status", [](const z_MotorStatus&) { record("test_custom/motor_status"); });
        z_create_subscription<z_SensorTelemetry>("test_custom/telemetry", [](const z_SensorTelemetry&) { record("test_custom/telemetry"); });
        z_create_subscription<z_RobotDiagnostic>("test_custom/robot_diag", [](const z_RobotDiagnostic&) { record("test_custom/robot_diag"); });
        z_create_subscription<z_ComplexRobotState>("test_custom/complex_state", [](const z_ComplexRobotState&) { record("test_custom/complex_state"); });

        // Custom Service Clients
        cli_set_led_clr_ = z_create_client<z_SetLEDColor>("srv_custom/set_led_color");
        cli_set_clr_     = z_create_client<z_SetColor>("srv_custom/set_color");
        cli_cfg_robot_   = z_create_client<z_ConfigureRobot>("srv_custom/configure_robot");
        cli_full_ctrl_   = z_create_client<z_FullSystemControl>("srv_custom/full_system_control");
    }

    void test_services() {
        ZLOG_INFO(this->get_logger(), "Testing 4 Custom Services (C++)...");

        // 1. SetLEDColor
        z_SetLEDColor::Request req1; req1.led_data.r = 255; req1.led_data.brightness = 100;
        z_SetLEDColor::Response res1;
        if (cli_set_led_clr_->call(req1, res1, 2000) && res1.success) {
            service_count++;
            std::cout << "  \033[32m✓ PASS\033[0m Service 'srv_custom/set_led_color'\n";
        }

        // 2. SetColor
        z_SetColor::Request req2; req2.r = 255; req2.g = 128; req2.b = 0;
        z_SetColor::Response res2;
        if (cli_set_clr_->call(req2, res2, 2000) && res2.success) {
            service_count++;
            std::cout << "  \033[32m✓ PASS\033[0m Service 'srv_custom/set_color'\n";
        }

        // 3. ConfigureRobot
        z_ConfigureRobot::Request req3; req3.target_status.motor_id = 1; req3.target_status.speed = 100.0f; req3.target_status.is_active = true; req3.mode = 1;
        z_ConfigureRobot::Response res3;
        if (cli_cfg_robot_->call(req3, res3, 2000) && res3.success) {
            service_count++;
            std::cout << "  \033[32m✓ PASS\033[0m Service 'srv_custom/configure_robot'\n";
        }

        // 4. FullSystemControl
        z_FullSystemControl::Request req4; req4.target_state.robot_mode = "TEST"; req4.command_code = 5;
        z_FullSystemControl::Response res4;
        if (cli_full_ctrl_->call(req4, res4, 2000) && res4.ack) {
            service_count++;
            std::cout << "  \033[32m✓ PASS\033[0m Service 'srv_custom/full_system_control'\n";
        }
    }

private:
    ZenohClient<z_SetLEDColor>* cli_set_led_clr_;
    ZenohClient<z_SetColor>* cli_set_clr_;
    ZenohClient<z_ConfigureRobot>* cli_cfg_robot_;
    ZenohClient<z_FullSystemControl>* cli_full_ctrl_;
};

int main(int argc, char** argv) {
    std::cout << "\n==========================================\n";
    std::cout << "  ALL-IN-ONE MASTER INTERFACE VALIDATOR (C++)\n";
    std::cout << "  Testing 34 Topics & 4 Custom Services Live\n";
    std::cout << "==========================================\n\n";

    ZenohConfig cfg;
    cfg.communication_mode = ZenohCommunicationMode::ZENOH_COMM_WIFI;
    cfg.host = (argc > 1) ? argv[1] : "10.42.0.50";
    cfg.port = 7447;

    if (!ZenohNode::init(cfg)) {
        std::cerr << "Zenoh init failed!\n";
        return 1;
    }

    MasterCppValidator node;

    std::thread spin_thread([&]() {
        node.z_spin();
    });

    auto start = std::chrono::steady_clock::now();
    bool srv_done = false;

    while (topic_count.load() < 34 || service_count.load() < 4) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (topic_count.load() >= 30 && !srv_done) {
            srv_done = true;
            node.test_services();
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 25) {
            break;
        }
    }

    std::cout << "\n==========================================\n";
    std::cout << "  RESULTS: " << topic_count.load() << "/34 Topics Passed | "
              << service_count.load() << "/4 Services Passed\n";
    if (topic_count.load() == 34 && service_count.load() == 4) {
        std::cout << "  \033[32m\033[1m✓ ALL PRE-DEFINED & CUSTOM INTERFACES VERIFIED 100%!\033[0m\n";
    } else {
        std::cout << "  \033[31m\033[1m⚠  PARTIAL FAILURE (Topics: " << topic_count.load()
                  << "/34, Services: " << service_count.load() << "/4)\033[0m\n";
    }
    std::cout << "==========================================\n\n";

    spin_thread.detach();
    return 0;
}
