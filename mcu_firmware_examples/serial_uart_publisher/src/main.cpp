#include <Arduino.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/std_msgs/z_Int32.h>
#include <zenoh_ros/std_msgs/z_Float64.h>

using namespace z_std_msgs;

/**
 * Serial UART / USB CDC Zenoh Publisher Example
 * Demonstrates streaming ROS 2 topics over Serial UART with automatic baudrate presets.
 */

// Configuration Struct for UART0 (Default USB flashing port at High Speed 921600 baud)
ZenohConfig cfg = {
    .transport_mode = ZenohTransportMode::ZENOH_TRANSPORT_UART_DEFAULT,
    .baudrate       = (uint32_t)ZenohBaudRate::UART_HIGH_SPEED
};

class SerialUARTPublisherNode : public ZenohNode {
public:
    SerialUARTPublisherNode() 
        : ZenohNode("serial_uart_mcu_node"), cnt_(0) {
        Serial.printf("[Node] %s initialized on Serial UART!\n", this->z_get_name());
        
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

            Serial.printf("[%s] Published Counter: %d | Temperature: %.2f °C\n",
                          this->z_get_name(), count_msg_.data, temp_msg_.data);
        }
    }
};

// Global Pointer to Node Instance
SerialUARTPublisherNode* node_instance = nullptr;

void setup() {
    Serial.begin(921600);
    z_delay(1000);

    Serial.println("\n==========================================");
    Serial.println("  ESP32-S3 Zenoh Serial UART Publisher    ");
    Serial.println("==========================================");

    Serial.println("[System] Initializing Zenoh Client...");
    if (ZenohNode::init(cfg)) {
        Serial.println("[System] Starting Zenoh Node...");
        node_instance = new SerialUARTPublisherNode();
    } else {
        Serial.println("[System] CRITICAL Error: Zenoh Client initialization failed!");
        while (1) { z_delay(1000); }
    }

    Serial.println("==========================================\n");
}

void loop() {
    if (node_instance) {
        node_instance->z_spin();
    }
}
