#include <Arduino.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/std_msgs/z_Int32.h>
#include <zenoh_ros/std_msgs/z_Float64.h>

using namespace z_std_msgs;

/**
 * Serial UART / USB CDC Zenoh Publisher Example
 * Demonstrates streaming ROS 2 topics over Serial UART with automatic baudrate presets.
 */

ZenohNode* node = nullptr;
ZenohPublisher<z_Int32>* pub_count = nullptr;
ZenohPublisher<z_Float64>* pub_temp = nullptr;

int counter = 0;

void setup() {
    // Select Zenoh Transport Configuration (UART0 Default USB port)
    ZenohConfig config;
    config.transport_mode = ZenohTransportMode::ZENOH_TRANSPORT_UART_DEFAULT;
    config.baudrate = (uint32_t)ZenohBaudRate::UART_HIGH_SPEED; // 921,600 baud for multi-topic throughput

    if (!ZenohNode::init(config)) {
        return;
    }

    node = new ZenohNode("serial_uart_mcu_node");
    pub_count = node->z_create_publisher<z_Int32>("serial/counter", 10);
    pub_temp  = node->z_create_publisher<z_Float64>("serial/temperature", 10);
}

void loop() {
    if (node) {
        node->z_spin();

        static unsigned long last_pub = 0;
        if (millis() - last_pub >= 1000) {
            last_pub = millis();

            z_Int32 count_msg;
            count_msg.data = counter++;
            pub_count->publish(count_msg);

            z_Float64 temp_msg;
            temp_msg.data = 25.0 + (random(0, 100) / 10.0);
            pub_temp->publish(temp_msg);
        }
    }
    delay(10);
}
