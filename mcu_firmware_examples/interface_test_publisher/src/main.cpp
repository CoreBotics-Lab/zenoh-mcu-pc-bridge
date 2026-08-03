/**
 * Rigorous Multi-Level Custom Interface Firmware
 * ===============================================
 */

#include <Arduino.h>
#include <zenoh_ros/ZenohRos.h>

#include <zenoh_ros/custom_msgs/z_ComplexRobotState.h>
#include <zenoh_ros/custom_srvs/z_FullSystemControl.h>

#include "credential.h"

ZenohConfig cfg = {
    .ssid      = WIFI_SSID,
    .password  = WIFI_PASSWORD,
    .port      = 7447,
    .local_ip  = "10.42.0.50",
    .wifi_mode = WIFI_STA
};

class ComplexSystemNode : public ZenohNode {
public:
    ComplexSystemNode() : ZenohNode("complex_system_node") {
        Serial.printf("[Node] %s started\n", z_get_name());

        pub_state_ = z_create_publisher<z_ComplexRobotState>("system/state", 5);

        srv_control_ = z_create_service<z_FullSystemControl>("system/control",
            [this](const z_FullSystemControl::Request& req, z_FullSystemControl::Response& res) {
                Serial.printf("[Service] Request received! Target mode: %s, Cmd: %d\n",
                              req.target_state.robot_mode.c_str(), req.command_code);

                res.current_telemetry.header.stamp.sec = 1700000000;
                res.current_telemetry.header.frame_id = "sensor_link";
                res.current_telemetry.accel.x = 0.05f;
                res.current_telemetry.accel.y = 0.01f;
                res.current_telemetry.accel.z = 9.81f;
                res.current_telemetry.orientation.w = 1.0f;
                res.current_telemetry.sensor_id = 99;
                res.current_telemetry.temp = 37.8f;
                res.current_telemetry.status_ok = true;

                res.ack = true;
                res.status_details = "Command " + std::to_string(req.command_code) + " Executed OK!";
            }
        );

        timer_ = z_create_timer(1000, [this]() { publish_state(); });
    }

private:
    ZenohPublisher<z_ComplexRobotState>* pub_state_ = nullptr;
    ZenohService<z_FullSystemControl>* srv_control_ = nullptr;
    ZenohTimer* timer_ = nullptr;
    uint64_t cycle_ = 0;

    void publish_state() {
        z_ComplexRobotState state;
        state.header.stamp.sec = 1700000000 + (uint32_t)cycle_;
        state.header.stamp.nanosec = 123456;
        state.header.frame_id = "map";

        state.pose.position.x = 2.5;
        state.pose.position.y = 3.8;
        state.pose.position.z = 0.1;
        state.pose.orientation.w = 1.0;

        state.telemetry.header.stamp.sec = state.header.stamp.sec;
        state.telemetry.header.frame_id = "imu_link";
        state.telemetry.accel.x = 0.1f;
        state.telemetry.accel.y = -0.2f;
        state.telemetry.accel.z = 9.80f;
        state.telemetry.orientation.x = 0.0f;
        state.telemetry.orientation.y = 0.0f;
        state.telemetry.orientation.z = 0.0f;
        state.telemetry.orientation.w = 1.0f;
        state.telemetry.sensor_id = 42;
        state.telemetry.temp = 36.5f;
        state.telemetry.status_ok = true;

        state.robot_mode = "AUTONOMOUS_NAV";
        state.cycle_count = ++cycle_;

        pub_state_->publish(state);
        Serial.printf("[Pub] ComplexRobotState cycle #%llu published!\n", cycle_);
    }
};

ComplexSystemNode* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000);
    Serial.println("\n==========================================");
    Serial.println("  Rigorous Multi-Level Custom Test Firmware");
    Serial.println("==========================================");
    if (ZenohNode::init(cfg)) {
        node_instance = new ComplexSystemNode();
    } else {
        Serial.println("[CRITICAL] Zenoh init failed!");
        while(1) { z_delay(1000); }
    }
}

void loop() {
    if (node_instance) node_instance->z_spin();
}
