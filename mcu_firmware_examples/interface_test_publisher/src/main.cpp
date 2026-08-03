/**
 * Custom Nested Interface & Service Test Firmware
 * =================================================
 * Includes direct custom headers:
 *   #include <zenoh_ros/custom_msgs/z_RobotDiagnostic.h>
 *   #include <zenoh_ros/custom_srvs/z_ConfigureRobot.h>
 */

#include <Arduino.h>
#include <zenoh_ros/ZenohRos.h>

#include <zenoh_ros/custom_msgs/z_RobotDiagnostic.h>
#include <zenoh_ros/custom_srvs/z_ConfigureRobot.h>

#include "credential.h"

ZenohConfig cfg = {
    .ssid      = WIFI_SSID,
    .password  = WIFI_PASSWORD,
    .port      = 7447,
    .local_ip  = "10.42.0.50",
    .wifi_mode = WIFI_STA
};

class CustomNestedNode : public ZenohNode {
public:
    CustomNestedNode() : ZenohNode("custom_nested_node") {
        Serial.printf("[Node] %s started\n", z_get_name());

        // Publisher for nested custom message
        pub_diag_ = z_create_publisher<z_RobotDiagnostic>("robot/diagnostics", 5);

        // Service server for nested custom service
        srv_config_ = z_create_service<z_ConfigureRobot>("robot/configure",
            [this](const z_ConfigureRobot::Request& req, z_ConfigureRobot::Response& res) {
                Serial.printf("[Service] Request received for Motor ID: %d, Speed: %.1f\n",
                              req.target_status.motor_id, req.target_status.speed);
                
                res.success = true;
                res.status_message = "Motor " + std::to_string(req.target_status.motor_id) + " configured OK!";
            }
        );

        // Timer to publish nested custom diagnostic message every 1 second
        timer_ = z_create_timer(1000, [this]() { publish_diagnostics(); });
    }

private:
    ZenohPublisher<z_RobotDiagnostic>* pub_diag_ = nullptr;
    ZenohService<z_ConfigureRobot>* srv_config_ = nullptr;
    ZenohTimer* timer_ = nullptr;
    uint32_t seq_ = 0;

    void publish_diagnostics() {
        z_RobotDiagnostic msg;
        msg.header.stamp.sec = 1690000000 + seq_;
        msg.header.stamp.nanosec = 500000;
        msg.header.frame_id = "base_link";

        msg.velocity.x = 1.2;
        msg.velocity.y = -0.5;
        msg.velocity.z = 0.0;

        msg.left_motor.motor_id = 1;
        msg.left_motor.speed = 100.5f;
        msg.left_motor.temperature = 42.1f;
        msg.left_motor.is_active = true;

        msg.right_motor.motor_id = 2;
        msg.right_motor.speed = 98.2f;
        msg.right_motor.temperature = 44.3f;
        msg.right_motor.is_active = true;

        msg.robot_name = "ZenohBot_01";

        pub_diag_->publish(msg);
        Serial.printf("[Pub] RobotDiagnostics #%u published OK!\n", ++seq_);
    }
};

CustomNestedNode* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000);
    Serial.println("\n==========================================");
    Serial.println("  Custom Nested Msg & Srv Test Firmware");
    Serial.println("==========================================");
    if (ZenohNode::init(cfg)) {
        node_instance = new CustomNestedNode();
    } else {
        Serial.println("[CRITICAL] Zenoh init failed!");
        while(1) { z_delay(1000); }
    }
}

void loop() {
    if (node_instance) node_instance->z_spin();
}
