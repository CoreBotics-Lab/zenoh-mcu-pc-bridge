#include <Arduino.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/std_msgs/z_Int32.h>
#include <zenoh_ros/std_msgs/z_Float64.h>

/**
 * Serial UART / USB CDC Zenoh Publisher Example
 * Demonstrates streaming ROS 2 topics over Native USB CDC with automatic baudrate presets.
 */

// Configuration Struct for Native USB CDC (High-Speed USB OTG PHY at 12 Mbps)
ZenohConfig cfg = ZenohConfig()
    .set_communication_mode(ZenohConfig::ZENOH_COMM_UART_USB_CDC)
    .set_baudrate(ZenohConfig::USB_HIGH_SPEED);

class SerialUARTPublisherNode : public ZenohNode {
public:
    SerialUARTPublisherNode() 
        : ZenohNode("serial_uart_mcu_node"), cnt_(0) {
        ZLOG_INFO(this->get_logger(), "Node %s initialized on Native USB CDC!", this->z_get_name());
        
        // 1. Create typed publishers
        pub_count_ = this->z_create_publisher<z_Int32>("serial/counter", 10);
        pub_temp_  = this->z_create_publisher<z_Float64>("serial/temperature", 10);

        // 2. Create ROS 2 timer (triggers callback every 1000ms)
        timer_ = this->z_create_timer(1000, [this]() -> void {
            this->timer_callback();
        });
    }

private:
    int cnt_;
    ZenohPublisher<z_Int32>* pub_count_;
    ZenohPublisher<z_Float64>* pub_temp_;
    ZenohTimer* timer_;
    
    z_Int32 count_msg_;
    z_Float64 temp_msg_;

    void timer_callback() {
        if (pub_count_ && pub_temp_) {
            count_msg_.data = cnt_++;
            pub_count_->publish(count_msg_);

            temp_msg_.data = 25.0 + (random(0, 100) / 10.0);
            pub_temp_->publish(temp_msg_);

            ZLOG_INFO(this->get_logger(), "Published Counter: %d | Temperature: %.2f °C",
                      count_msg_.data, temp_msg_.data);
        }
    }
};

// Global Pointer to Node Instance
SerialUARTPublisherNode* node_instance = nullptr;

void setup() {
    Serial.begin(12000000);
    z_delay(1000);

    ZLOG_INFO(z_get_logger("system"), "==========================================");
    ZLOG_INFO(z_get_logger("system"), "  ESP32-S3 Zenoh USB CDC Serial Publisher ");
    ZLOG_INFO(z_get_logger("system"), "==========================================");

    ZLOG_INFO(z_get_logger("system"), "Initializing Zenoh Client...");
    if (ZenohNode::init(cfg)) {
        ZLOG_INFO(z_get_logger("system"), "Starting Zenoh Node...");
        node_instance = new SerialUARTPublisherNode();
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
