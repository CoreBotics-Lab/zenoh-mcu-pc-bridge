/**
 * All-In-One Master Test Firmware (Dynamic Memory Stress Test)
 * =============================================================
 * Publishes changing dynamic data on 34 topics every 1 second.
 * Serves 4 custom services with dynamic response payloads.
 */

#include <Arduino.h>
#include <math.h>
#include <zenoh_ros/ZenohRos.h>

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

#include "credential.h"

ZenohConfig cfg = ZenohConfig()
    .set_communication_mode(ZenohConfig::ZENOH_COMM_WIFI)
    .set_wifi(WIFI_SSID, WIFI_PASSWORD, WIFI_STA)
    .set_static_ip("10.42.0.50")
    .set_port(7447);

class MasterTestNode : public ZenohNode {
public:
    MasterTestNode() : ZenohNode("master_test_node") {
        ZLOG_INFO(this->get_logger(), "Node %s initialized!", z_get_name());

        // Standard Publishers
        pub_bool_     = z_create_publisher<z_Bool>("test/bool", 10);
        pub_int8_     = z_create_publisher<z_Int8>("test/int8", 10);
        pub_uint8_    = z_create_publisher<z_UInt8>("test/uint8", 10);
        pub_int16_    = z_create_publisher<z_Int16>("test/int16", 10);
        pub_uint16_   = z_create_publisher<z_UInt16>("test/uint16", 10);
        pub_int32_    = z_create_publisher<z_Int32>("test/int32", 10);
        pub_uint32_   = z_create_publisher<z_UInt32>("test/uint32", 10);
        pub_int64_    = z_create_publisher<z_Int64>("test/int64", 10);
        pub_uint64_   = z_create_publisher<z_UInt64>("test/uint64", 10);
        pub_float32_  = z_create_publisher<z_Float32>("test/float32", 10);
        pub_float64_  = z_create_publisher<z_Float64>("test/float64", 10);
        pub_string_   = z_create_publisher<z_String>("test/string", 10);
        pub_header_   = z_create_publisher<z_Header>("test/header", 10);
        pub_i32arr_   = z_create_publisher<z_Int32MultiArray>("test/int32_array", 10);
        pub_f64arr_   = z_create_publisher<z_Float64MultiArray>("test/float64_array", 10);
        pub_imu_      = z_create_publisher<z_Imu>("test/imu", 10);
        pub_temp_     = z_create_publisher<z_Temperature>("test/temperature", 10);
        pub_range_    = z_create_publisher<z_Range>("test/range", 10);
        pub_hum_      = z_create_publisher<z_RelativeHumidity>("test/humidity", 10);
        pub_bat_      = z_create_publisher<z_BatteryState>("test/battery", 10);
        pub_gps_      = z_create_publisher<z_NavSatFix>("test/navsatfix", 10);
        pub_joy_      = z_create_publisher<z_JoyFeedback>("test/joyfeedback", 10);
        pub_joint_    = z_create_publisher<z_JointState>("test/jointstate", 10);
        pub_vec3_     = z_create_publisher<z_Vector3>("test/vector3", 10);
        pub_quat_     = z_create_publisher<z_Quaternion>("test/quaternion", 10);
        pub_pose_     = z_create_publisher<z_Pose>("test/pose", 10);
        pub_twist_    = z_create_publisher<z_Twist>("test/twist", 10);

        // Custom Publishers
        pub_set_led_    = z_create_publisher<z_SetLED>("test_custom/set_led", 10);
        pub_mpu_        = z_create_publisher<z_MPU6050Data>("test_custom/mpu6050", 10);
        pub_robot_st_   = z_create_publisher<z_RobotState>("test_custom/robot_state", 10);
        pub_motor_st_   = z_create_publisher<z_MotorStatus>("test_custom/motor_status", 10);
        pub_telemetry_  = z_create_publisher<z_SensorTelemetry>("test_custom/telemetry", 10);
        pub_robot_diag_ = z_create_publisher<z_RobotDiagnostic>("test_custom/robot_diag", 10);
        pub_cmplx_st_   = z_create_publisher<z_ComplexRobotState>("test_custom/complex_state", 10);

        // Custom Services
        srv_set_led_clr_ = z_create_service<z_SetLEDColor>("srv_custom/set_led_color",
            [this](const z_SetLEDColor::Request& req, z_SetLEDColor::Response& res) {
                res.success = (req.led_data.r > 0 || req.led_data.brightness > 0);
                res.message = "SetLEDColor OK Count " + std::to_string(count_);
            }
        );

        srv_set_clr_ = z_create_service<z_SetColor>("srv_custom/set_color",
            [this](const z_SetColor::Request& req, z_SetColor::Response& res) {
                res.success = (req.r > 0 || req.g > 0 || req.b > 0);
                res.message = "SetColor OK Count " + std::to_string(count_);
            }
        );

        srv_cfg_robot_ = z_create_service<z_ConfigureRobot>("srv_custom/configure_robot",
            [this](const z_ConfigureRobot::Request& req, z_ConfigureRobot::Response& res) {
                res.success = true;
                res.status_message = "Motor " + std::to_string(req.target_status.motor_id) + " Configured Count " + std::to_string(count_);
            }
        );

        srv_full_ctrl_ = z_create_service<z_FullSystemControl>("srv_custom/full_system_control",
            [this](const z_FullSystemControl::Request& req, z_FullSystemControl::Response& res) {
                res.current_telemetry.sensor_id = (int32_t)(count_ % 100);
                res.current_telemetry.temp = 35.0f + (float)(count_ % 10);
                res.current_telemetry.status_ok = true;
                res.ack = true;
                res.status_details = "FullSystemControl Code " + std::to_string(req.command_code) + " OK Count " + std::to_string(count_);
            }
        );

        timer_ = z_create_timer(1000, [this]() { publish_all(); });
    }

private:
    ZenohPublisher<z_Bool>* pub_bool_;
    ZenohPublisher<z_Int8>* pub_int8_;
    ZenohPublisher<z_UInt8>* pub_uint8_;
    ZenohPublisher<z_Int16>* pub_int16_;
    ZenohPublisher<z_UInt16>* pub_uint16_;
    ZenohPublisher<z_Int32>* pub_int32_;
    ZenohPublisher<z_UInt32>* pub_uint32_;
    ZenohPublisher<z_Int64>* pub_int64_;
    ZenohPublisher<z_UInt64>* pub_uint64_;
    ZenohPublisher<z_Float32>* pub_float32_;
    ZenohPublisher<z_Float64>* pub_float64_;
    ZenohPublisher<z_String>* pub_string_;
    ZenohPublisher<z_Header>* pub_header_;
    ZenohPublisher<z_Int32MultiArray>* pub_i32arr_;
    ZenohPublisher<z_Float64MultiArray>* pub_f64arr_;
    ZenohPublisher<z_Imu>* pub_imu_;
    ZenohPublisher<z_Temperature>* pub_temp_;
    ZenohPublisher<z_Range>* pub_range_;
    ZenohPublisher<z_RelativeHumidity>* pub_hum_;
    ZenohPublisher<z_BatteryState>* pub_bat_;
    ZenohPublisher<z_NavSatFix>* pub_gps_;
    ZenohPublisher<z_JoyFeedback>* pub_joy_;
    ZenohPublisher<z_JointState>* pub_joint_;
    ZenohPublisher<z_Vector3>* pub_vec3_;
    ZenohPublisher<z_Quaternion>* pub_quat_;
    ZenohPublisher<z_Pose>* pub_pose_;
    ZenohPublisher<z_Twist>* pub_twist_;

    // Custom Publishers
    ZenohPublisher<z_SetLED>* pub_set_led_;
    ZenohPublisher<z_MPU6050Data>* pub_mpu_;
    ZenohPublisher<z_RobotState>* pub_robot_st_;
    ZenohPublisher<z_MotorStatus>* pub_motor_st_;
    ZenohPublisher<z_SensorTelemetry>* pub_telemetry_;
    ZenohPublisher<z_RobotDiagnostic>* pub_robot_diag_;
    ZenohPublisher<z_ComplexRobotState>* pub_cmplx_st_;

    // Custom Services
    ZenohService<z_SetLEDColor>* srv_set_led_clr_;
    ZenohService<z_SetColor>* srv_set_clr_;
    ZenohService<z_ConfigureRobot>* srv_cfg_robot_;
    ZenohService<z_FullSystemControl>* srv_full_ctrl_;

    ZenohTimer* timer_ = nullptr;
    uint32_t count_ = 0;

    void publish_all() {
        count_++;

        z_Bool msg_bool; msg_bool.data = (count_ % 2 == 0); pub_bool_->publish(msg_bool);
        z_Int8 msg_int8; msg_int8.data = (int8_t)(-50 + (count_ % 100)); pub_int8_->publish(msg_int8);
        z_UInt8 msg_uint8; msg_uint8.data = (uint8_t)((count_ * 13) % 255); pub_uint8_->publish(msg_uint8);
        z_Int16 msg_int16; msg_int16.data = (int16_t)(-2000 + (count_ % 4000)); pub_int16_->publish(msg_int16);
        z_UInt16 msg_uint16; msg_uint16.data = (uint16_t)((count_ * 100) % 65000); pub_uint16_->publish(msg_uint16);
        z_Int32 msg_int32; msg_int32.data = (int32_t)(-100000 + count_ * 100); pub_int32_->publish(msg_int32);
        z_UInt32 msg_uint32; msg_uint32.data = (uint32_t)(3141592 + count_); pub_uint32_->publish(msg_uint32);
        z_Int64 msg_int64; msg_int64.data = (int64_t)(-9876543210LL + count_); pub_int64_->publish(msg_int64);
        z_UInt64 msg_uint64; msg_uint64.data = (uint64_t)(18446744073709551600ULL + count_); pub_uint64_->publish(msg_uint64);
        z_Float32 msg_float32; msg_float32.data = 3.14159f + (float)(count_ % 100) * 0.01f; pub_float32_->publish(msg_float32);
        z_Float64 msg_float64; msg_float64.data = 2.718281828459 + (double)count_ * 0.001; pub_float64_->publish(msg_float64);

        z_String msg_string;
        msg_string.data = "dyn_string_cycle_" + std::to_string(count_);
        pub_string_->publish(msg_string);

        z_Header msg_header;
        msg_header.stamp.sec = millis() / 1000;
        msg_header.stamp.nanosec = (millis() % 1000) * 1000000;
        msg_header.frame_id = "frame_" + std::to_string(count_);
        pub_header_->publish(msg_header);

        z_Int32MultiArray msg_i32arr;
        msg_i32arr.data = { (int32_t)count_, (int32_t)(count_ * 2), (int32_t)(count_ * 3) };
        pub_i32arr_->publish(msg_i32arr);

        z_Float64MultiArray msg_f64arr;
        msg_f64arr.data = { (double)count_ * 0.1, (double)count_ * 0.2, (double)count_ * 0.3 };
        pub_f64arr_->publish(msg_f64arr);

        z_Imu msg_imu;
        msg_imu.linear_acceleration.x = sin(count_ * 0.1);
        msg_imu.linear_acceleration.y = cos(count_ * 0.1);
        msg_imu.linear_acceleration.z = 9.81;
        pub_imu_->publish(msg_imu);

        z_Temperature msg_temp; msg_temp.temperature = 25.0f + (float)(count_ % 20); pub_temp_->publish(msg_temp);
        z_Range msg_range; msg_range.range = 1.0f + (float)(count_ % 10) * 0.1f; pub_range_->publish(msg_range);
        z_RelativeHumidity msg_hum; msg_hum.relative_humidity = 0.5f + (float)(count_ % 50) * 0.01f; pub_hum_->publish(msg_hum);
        z_BatteryState msg_bat; msg_bat.voltage = 11.0f + (float)(count_ % 30) * 0.1f; pub_bat_->publish(msg_bat);
        z_NavSatFix msg_gps; msg_gps.latitude = 24.8607 + count_ * 0.0001; msg_gps.longitude = 67.0011 + count_ * 0.0001; pub_gps_->publish(msg_gps);
        z_JoyFeedback msg_joy; msg_joy.id = count_ % 4; msg_joy.intensity = (float)(count_ % 10) * 0.1f; pub_joy_->publish(msg_joy);

        z_JointState msg_joint;
        msg_joint.name = {"joint1_" + std::to_string(count_), "joint2_" + std::to_string(count_)};
        msg_joint.position = { (double)(count_ % 10) * 0.1, (double)(count_ % 20) * 0.1 };
        pub_joint_->publish(msg_joint);

        z_Vector3 msg_vec3; msg_vec3.x = count_ * 0.1; msg_vec3.y = count_ * 0.2; msg_vec3.z = count_ * 0.3; pub_vec3_->publish(msg_vec3);
        z_Quaternion msg_quat; msg_quat.x = sin(count_ * 0.05); msg_quat.y = 0.0; msg_quat.z = 0.0; msg_quat.w = cos(count_ * 0.05); pub_quat_->publish(msg_quat);
        z_Pose msg_pose; msg_pose.position.x = count_ * 0.5; msg_pose.orientation.w = 1.0; pub_pose_->publish(msg_pose);
        z_Twist msg_twist; msg_twist.linear.x = 1.0 + (count_ % 5) * 0.1; msg_twist.angular.z = 0.1 * (count_ % 10); pub_twist_->publish(msg_twist);

        // Custom Msgs
        z_SetLED msg_led; msg_led.r = (count_ * 10) % 255; msg_led.brightness = count_ % 100; pub_set_led_->publish(msg_led);
        z_MPU6050Data msg_mpu; msg_mpu.accel_x = sin(count_ * 0.1f); msg_mpu.accel_y = cos(count_ * 0.1f); msg_mpu.accel_z = 9.81f; pub_mpu_->publish(msg_mpu);
        z_RobotState msg_rst; msg_rst.name = "ZenohBot_Cycle_" + std::to_string(count_); msg_rst.position.x = count_ * 0.1; msg_rst.velocity.linear.x = 0.5; pub_robot_st_->publish(msg_rst);
        z_MotorStatus msg_mot; msg_mot.motor_id = (count_ % 4) + 1; msg_mot.speed = 100.0f + (count_ % 200); msg_mot.is_active = (count_ % 2 == 0); pub_motor_st_->publish(msg_mot);

        z_SensorTelemetry msg_telem;
        msg_telem.sensor_id = count_ % 50; msg_telem.temp = 30.0f + (float)(count_ % 20); msg_telem.status_ok = true;
        pub_telemetry_->publish(msg_telem);

        z_RobotDiagnostic msg_diag;
        msg_diag.robot_name = "ZenohBot_V" + std::to_string(count_); msg_diag.left_motor.motor_id = 1; msg_diag.right_motor.motor_id = 2;
        pub_robot_diag_->publish(msg_diag);

        z_ComplexRobotState msg_cmplx;
        msg_cmplx.robot_mode = (count_ % 2 == 0) ? "NAVIGATING" : "CHARGING";
        msg_cmplx.cycle_count = count_;
        pub_cmplx_st_->publish(msg_cmplx);

        ZLOG_INFO(this->get_logger(), "All 34 dynamic topics published! Cycle: %u", count_);
    }
};

MasterTestNode* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000);
    ZLOG_INFO(z_get_logger("system"), "==========================================");
    ZLOG_INFO(z_get_logger("system"), "  DYNAMIC DATA MEMORY STRESS FIRMWARE");
    ZLOG_INFO(z_get_logger("system"), "==========================================");
    if (ZenohNode::init(cfg)) {
        node_instance = new MasterTestNode();
    } else {
        ZLOG_ERROR(z_get_logger("system"), "[CRITICAL] Zenoh init failed!");
        while(1) { z_delay(1000); }
    }
}

void loop() {
    if (node_instance) node_instance->z_spin();
}
