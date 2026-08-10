#include <Arduino.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/std_msgs/z_Int32.h>
#include <zenoh_ros/std_msgs/z_String.h>


// --- Configuration Struct ---
ZenohConfig cfg = ZenohConfig()
    .set_communication_mode(ZenohConfig::ZENOH_COMM_WIFI)
    .set_wifi("ESP32S3_Zenoh_AP", "zenoh1234", WIFI_AP)
    .set_port(7447);

class Counter_publisher_node_class : public ZenohNode {
public:
    Counter_publisher_node_class() 
        : ZenohNode("counter_publisher"), cnt_(0) {
        ZLOG_INFO(this->get_logger(), "Node %s has been started", this->z_get_name());
        
        // 1. Create typed publishers (exactly like in ROS 2)
        publisher_ = this->z_create_publisher<z_Int32>("robot/sim_counter", 10);
        string_publisher_ = this->z_create_publisher<z_String>("robot/hello_string", 10);

        // 2. Create the timers (triggers callback_timer every 1000ms, string_timer every 500ms)
        timer_ = this->z_create_timer(1000, [this]() -> void {
            this->callback_timer();
        });
        string_timer_ = this->z_create_timer(500, [this]() -> void {
            this->callback_string_timer();
        });
    }

private:
    int cnt_;
    ZenohPublisher<z_Int32>* publisher_;
    ZenohPublisher<z_String>* string_publisher_;
    ZenohTimer* timer_;
    ZenohTimer* string_timer_;
    
    // Pre-allocated message structure
    z_Int32 msg; 

    void callback_timer() {
        this->cnt_++;
        if (this->publisher_) {
            // Populate and publish the message structure
            msg.data = this->cnt_;
            this->publisher_->publish(msg);
            
            ZLOG_INFO(this->get_logger(), "Publishing Counter: %d", msg.data);
        }
    }

    void callback_string_timer() {
        if (this->string_publisher_) {
            z_String str_msg;
            char str_buf[64];
            snprintf(str_buf, sizeof(str_buf), "HelloWorld_%d", this->cnt_);
            str_msg.data = str_buf;
            this->string_publisher_->publish(str_msg);
            
            ZLOG_INFO(this->get_logger(), "Publishing String: %s", str_msg.data.c_str());
        }
    }
};

// Pointer to our node instance
Counter_publisher_node_class* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000); // USB CDC Serial delay for ESP32-S3

    ZLOG_INFO(z_get_logger("system"), "==========================================");
    ZLOG_INFO(z_get_logger("system"), "  ESP32-S3 Zenoh SoftAP Counter Testbench");
    ZLOG_INFO(z_get_logger("system"), "==========================================");

    // 1. Initialize the global Zenoh client context (analogous to rclcpp::init)
    ZLOG_INFO(z_get_logger("system"), "Initializing the Zenoh Client...");
    if (ZenohNode::init(cfg)) {
        // 2. Spin up the Node instance
        ZLOG_INFO(z_get_logger("system"), "Starting the Zenoh Node...");
        node_instance = new Counter_publisher_node_class();
    } else {
        ZLOG_ERROR(z_get_logger("system"), "CRITICAL Error: Zenoh Client initialization failed!");
        while (1) { z_delay(1000); }
    }
}

void loop() {
    // Spin the node to keep it alive (analogous to rclcpp::spin)
    if (node_instance) {
        node_instance->z_spin();
    }
}