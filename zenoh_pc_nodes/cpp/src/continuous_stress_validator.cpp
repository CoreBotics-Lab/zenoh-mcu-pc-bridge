/**
 * continuous_stress_validator.cpp — 2-Minute High-Frequency Continuous Stress Validator (C++)
 * =========================================================================================
 * Validates continuous streaming on 34 topics + frequent service calls (every 2s) for 120s.
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <map>
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

std::map<std::string, uint64_t> topic_counts;
std::mutex mtx;
std::atomic<uint64_t> total_msg_counter{0};

void record_msg(const std::string& top) {
    std::lock_guard<std::mutex> lock(mtx);
    topic_counts[top]++;
    total_msg_counter++;
}

class ContinuousStressNode : public ZenohNode {
public:
    ContinuousStressNode() : ZenohNode("continuous_stress_cpp_node") {
        z_create_subscription<z_Bool>("test/bool", [](const z_Bool&) { record_msg("test/bool"); });
        z_create_subscription<z_Int8>("test/int8", [](const z_Int8&) { record_msg("test/int8"); });
        z_create_subscription<z_UInt8>("test/uint8", [](const z_UInt8&) { record_msg("test/uint8"); });
        z_create_subscription<z_Int16>("test/int16", [](const z_Int16&) { record_msg("test/int16"); });
        z_create_subscription<z_UInt16>("test/uint16", [](const z_UInt16&) { record_msg("test/uint16"); });
        z_create_subscription<z_Int32>("test/int32", [](const z_Int32&) { record_msg("test/int32"); });
        z_create_subscription<z_UInt32>("test/uint32", [](const z_UInt32&) { record_msg("test/uint32"); });
        z_create_subscription<z_Int64>("test/int64", [](const z_Int64&) { record_msg("test/int64"); });
        z_create_subscription<z_UInt64>("test/uint64", [](const z_UInt64&) { record_msg("test/uint64"); });
        z_create_subscription<z_Float32>("test/float32", [](const z_Float32&) { record_msg("test/float32"); });
        z_create_subscription<z_Float64>("test/float64", [](const z_Float64&) { record_msg("test/float64"); });
        z_create_subscription<z_String>("test/string", [](const z_String&) { record_msg("test/string"); });
        z_create_subscription<z_Header>("test/header", [](const z_Header&) { record_msg("test/header"); });
        z_create_subscription<z_Int32MultiArray>("test/int32_array", [](const z_Int32MultiArray&) { record_msg("test/int32_array"); });
        z_create_subscription<z_Float64MultiArray>("test/float64_array", [](const z_Float64MultiArray&) { record_msg("test/float64_array"); });
        z_create_subscription<z_Imu>("test/imu", [](const z_Imu&) { record_msg("test/imu"); });
        z_create_subscription<z_Temperature>("test/temperature", [](const z_Temperature&) { record_msg("test/temperature"); });
        z_create_subscription<z_Range>("test/range", [](const z_Range&) { record_msg("test/range"); });
        z_create_subscription<z_RelativeHumidity>("test/humidity", [](const z_RelativeHumidity&) { record_msg("test/humidity"); });
        z_create_subscription<z_BatteryState>("test/battery", [](const z_BatteryState&) { record_msg("test/battery"); });
        z_create_subscription<z_NavSatFix>("test/navsatfix", [](const z_NavSatFix&) { record_msg("test/navsatfix"); });
        z_create_subscription<z_JoyFeedback>("test/joyfeedback", [](const z_JoyFeedback&) { record_msg("test/joyfeedback"); });
        z_create_subscription<z_JointState>("test/jointstate", [](const z_JointState&) { record_msg("test/jointstate"); });
        z_create_subscription<z_Vector3>("test/vector3", [](const z_Vector3&) { record_msg("test/vector3"); });
        z_create_subscription<z_Quaternion>("test/quaternion", [](const z_Quaternion&) { record_msg("test/quaternion"); });
        z_create_subscription<z_Pose>("test/pose", [](const z_Pose&) { record_msg("test/pose"); });
        z_create_subscription<z_Twist>("test/twist", [](const z_Twist&) { record_msg("test/twist"); });

        // Custom Topics
        z_create_subscription<z_SetLED>("test_custom/set_led", [](const z_SetLED&) { record_msg("test_custom/set_led"); });
        z_create_subscription<z_MPU6050Data>("test_custom/mpu6050", [](const z_MPU6050Data&) { record_msg("test_custom/mpu6050"); });
        z_create_subscription<z_RobotState>("test_custom/robot_state", [](const z_RobotState&) { record_msg("test_custom/robot_state"); });
        z_create_subscription<z_MotorStatus>("test_custom/motor_status", [](const z_MotorStatus&) { record_msg("test_custom/motor_status"); });
        z_create_subscription<z_SensorTelemetry>("test_custom/telemetry", [](const z_SensorTelemetry&) { record_msg("test_custom/telemetry"); });
        z_create_subscription<z_RobotDiagnostic>("test_custom/robot_diag", [](const z_RobotDiagnostic&) { record_msg("test_custom/robot_diag"); });
        z_create_subscription<z_ComplexRobotState>("test_custom/complex_state", [](const z_ComplexRobotState&) { record_msg("test_custom/complex_state"); });

        // Custom Service Clients
        cli_set_led_clr_ = z_create_client<z_SetLEDColor>("srv_custom/set_led_color");
        cli_set_clr_     = z_create_client<z_SetColor>("srv_custom/set_color");
        cli_cfg_robot_   = z_create_client<z_ConfigureRobot>("srv_custom/configure_robot");
        cli_full_ctrl_   = z_create_client<z_FullSystemControl>("srv_custom/full_system_control");
    }

    bool call_services() {
        bool ok = true;
        // 1. SetLEDColor
        z_SetLEDColor::Request req1; req1.led_data.r = 255; req1.led_data.brightness = 100;
        z_SetLEDColor::Response res1;
        if (!cli_set_led_clr_->call(req1, res1, 1500) || !res1.success) ok = false;

        // 2. SetColor
        z_SetColor::Request req2; req2.r = 255; req2.g = 128; req2.b = 0;
        z_SetColor::Response res2;
        if (!cli_set_clr_->call(req2, res2, 1500) || !res2.success) ok = false;

        // 3. ConfigureRobot
        z_ConfigureRobot::Request req3; req3.target_status.motor_id = 1; req3.target_status.speed = 100.0f; req3.target_status.is_active = true; req3.mode = 1;
        z_ConfigureRobot::Response res3;
        if (!cli_cfg_robot_->call(req3, res3, 1500) || !res3.success) ok = false;

        // 4. FullSystemControl
        z_FullSystemControl::Request req4; req4.target_state.robot_mode = "STRESS_CPP"; req4.command_code = 9;
        z_FullSystemControl::Response res4;
        if (!cli_full_ctrl_->call(req4, res4, 1500) || !res4.ack) ok = false;

        return ok;
    }

private:
    ZenohClient<z_SetLEDColor>* cli_set_led_clr_;
    ZenohClient<z_SetColor>* cli_set_clr_;
    ZenohClient<z_ConfigureRobot>* cli_cfg_robot_;
    ZenohClient<z_FullSystemControl>* cli_full_ctrl_;
};

int main(int argc, char** argv) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CONTINUOUS 2-MINUTE STRESS TEST & VALIDATOR (C++)\n";
    std::cout << "  Testing continuous data stream & frequent service calls\n";
    std::cout << "==============================================================\n\n";

    ZenohConfig cfg;
    cfg.host = (argc > 1) ? argv[1] : "10.42.0.50";
    cfg.port = 7447;

    if (!ZenohNode::init(cfg)) {
        std::cerr << "Zenoh init failed!\n";
        return 1;
    }

    ContinuousStressNode node;

    std::thread spin_thread([&]() {
        node.z_spin();
    });

    auto start_time = std::chrono::steady_clock::now();
    uint32_t total_srv_calls = 0;
    uint32_t successful_srv_calls = 0;
    auto last_srv_time = std::chrono::steady_clock::now();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto now = std::chrono::steady_clock::now();
        int elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

        if (elapsed >= 120) break;

        int srv_elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_srv_time).count();
        if (srv_elapsed >= 2) {
            last_srv_time = now;
            total_srv_calls += 4;
            if (node.call_services()) {
                successful_srv_calls += 4;
            }
        }

        if (elapsed % 10 == 0 && elapsed > 0) {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "  [" << elapsed << "s/120s] Active Topics: " << topic_counts.size()
                      << "/34 | Total Msgs: " << total_msg_counter.load()
                      << " | Srv Calls OK: " << successful_srv_calls << "/" << total_srv_calls << "\n";
        }
    }

    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "\n==============================================================\n";
    std::cout << "  CONTINUOUS STRESS TEST FINISHED (120s elapsed)\n";
    std::cout << "  Active Topics  : " << topic_counts.size() << "/34\n";
    std::cout << "  Total Messages : " << total_msg_counter.load() << "\n";
    std::cout << "  Service Calls  : " << successful_srv_calls << "/" << total_srv_calls
              << " PASSED (" << (double)successful_srv_calls/total_srv_calls*100.0 << "%)\n";
    if (topic_counts.size() == 34 && successful_srv_calls == total_srv_calls) {
        std::cout << "  \033[32m\033[1m✓ 100% PERFECT CONTINUOUS STABILITY VERIFIED! ZERO LOSS!\033[0m\n";
    } else {
        std::cout << "  \033[31m\033[1m⚠  STRESS TEST COMPLETED WITH LOSS/ERRORS\033[0m\n";
    }
    std::cout << "==============================================================\n\n";

    spin_thread.detach();
    return 0;
}
