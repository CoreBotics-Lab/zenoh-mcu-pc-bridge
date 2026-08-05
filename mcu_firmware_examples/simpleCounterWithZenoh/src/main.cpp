#include <Arduino.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/std_msgs/z_Int32.h>

#include "credential.h"

// --- Configuration Struct (AP Mode) ---
/*
ZenohConfig cfg = {
    .ssid = "ESP32S3_Zenoh_AP",
    .password = "zenoh1234",
    .port = 7447
};
*/

// --- Configuration Struct (STA Mode) ---
ZenohConfig cfg = {
    .ssid = WIFI_SSID,
    .password = WIFI_PASSWORD,
    .port = 7447,
    .local_ip = "10.42.0.50",
    .wifi_mode = WIFI_STA,
    .communication_mode = ZenohCommunicationMode::ZENOH_COMM_WIFI
};

class Counter_publisher_node_class : public ZenohNode {
public:
    Counter_publisher_node_class() 
        : ZenohNode("counter_publisher"), cnt_(random(255, 1024)) {
        Serial.printf("[Node] %s has been started (starting counter at: %d)\n", this->z_get_name(), cnt_);
        
        // 1. Create a typed publisher with custom queue depth (exactly like in ROS 2)
        publisher_ = this->z_create_publisher<z_Int32>("robot/sim_counter", 10);

        // 2. Create the timer (triggers callback_timer every 1000ms)
        timer_ = this->z_create_timer(100, [this]() -> void {
            this->callback_timer();
        });
    }

private:
    int cnt_;
    ZenohPublisher<z_Int32>* publisher_;
    ZenohTimer* timer_;
    
    // Pre-allocated message structure (exactly like String::SharedPtr msg in ROS 2)
    z_Int32 msg; 

    void callback_timer() {
        this->cnt_++;
        if (this->cnt_ % 7 == 0) {
            int old_val = this->cnt_;
            this->cnt_ = random(255, 1024);
        }
        if (this->publisher_) {
            // Populate and publish the message structure
            msg.data = this->cnt_;
            this->publisher_->publish(msg);
            
            Serial.printf("[%s] Publishing: %d\n", this->z_get_name(), msg.data);
        }
    }
};

// Pointer to our node instance
Counter_publisher_node_class* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000); // USB CDC Serial delay for ESP32-S3

    Serial.println("\n==========================================");
    Serial.println("  ESP32-S3 Zenoh SoftAP Counter Testbench");
    Serial.println("==========================================");

    // 1. Initialize the global Zenoh client context (analogous to rclcpp::init)
    Serial.println("[System] Initializing the Zenoh Client...");
    if (ZenohNode::init(cfg)) {
        // 2. Spin up the Node instance (analogous to make_shared)
        Serial.println("[System] Starting the Zenoh Node...");
        node_instance = new Counter_publisher_node_class();
    } else {
        Serial.println("[System] CRITICAL Error: Zenoh Client initialization failed!");
        while (1) { z_delay(1000); }
    }

    Serial.println("==========================================\n");
}

void loop() {
    // Spin the node to keep it alive (analogous to rclcpp::spin)
    if (node_instance) {
        node_instance->z_spin();
    }
}