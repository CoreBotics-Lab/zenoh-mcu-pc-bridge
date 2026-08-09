#include <Arduino.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/std_msgs/z_Int32.h>

#include "credential.h"

// --- Configuration Struct (STA Mode) ---
ZenohConfig cfg = ZenohConfig()
    .set_wifi(WIFI_SSID, WIFI_PASSWORD, WIFI_STA)
    .set_static_ip("10.42.0.50")
    .set_port(7447);

class Counter_publisher_node_class : public ZenohNode {
public:
    Counter_publisher_node_class() 
        : ZenohNode("counter_publisher"), cnt_(random(255, 1024)) {
        ZLOG_INFO(this->get_logger(), "Node %s started at counter: %d", this->z_get_name(), cnt_);
        
        // 1. Create a typed publisher with custom queue depth
        publisher_ = this->z_create_publisher<z_Int32>("robot/sim_counter", 10);

        // 2. Create the timer (triggers callback_timer every 100ms)
        timer_ = this->z_create_timer(100, [this]() -> void {
            this->callback_timer();
        });
    }

private:
    int cnt_;
    ZenohPublisher<z_Int32>* publisher_;
    ZenohTimer* timer_;
    
    z_Int32 msg; 

    void callback_timer() {
        this->cnt_++;
        if (this->cnt_ % 7 == 0) {
            this->cnt_ = random(255, 1024);
        }
        if (this->publisher_) {
            msg.data = this->cnt_;
            this->publisher_->publish(msg);
            
            ZLOG_INFO(this->get_logger(), "Publishing: %d", msg.data);
        }
    }
};

// Pointer to our node instance
Counter_publisher_node_class* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000); // USB CDC Serial delay for ESP32-S3

    ZLOG_INFO(z_get_logger("system"), "==========================================");
    ZLOG_INFO(z_get_logger("system"), "  ESP32-S3 Zenoh STA Counter Testbench");
    ZLOG_INFO(z_get_logger("system"), "==========================================");

    ZLOG_INFO(z_get_logger("system"), "Initializing the Zenoh Client...");
    if (ZenohNode::init(cfg)) {
        ZLOG_INFO(z_get_logger("system"), "Starting the Zenoh Node...");
        node_instance = new Counter_publisher_node_class();
    } else {
        ZLOG_ERROR(z_get_logger("system"), "CRITICAL Error: Zenoh Client initialization failed!");
        while (1) { z_delay(1000); }
    }
}

void loop() {
    if (node_instance) {
        node_instance->z_spin();
    }
}