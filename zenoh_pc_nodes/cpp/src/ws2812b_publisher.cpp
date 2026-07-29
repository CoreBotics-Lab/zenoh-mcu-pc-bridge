#include <zenoh_ros/ZenohRosPC.h>
#include <zenoh_ros/custom_msgs/z_SetLED.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

const int NUM_LEDS = 16;

class WS2812BPublisherNode : public ZenohNode {
public:
    WS2812BPublisherNode() : ZenohNode("ws2812b_publisher"), j_(0), r_(50), g_(0), b_(100) {
        std::cout << "[Node] " << this->z_get_name() << " has been started\n";

        // Seed random
        std::srand(std::time(nullptr));

        // Create publisher on "ws2812b"
        pub_ = this->z_create_publisher<z_SetLED>("ws2812b", 10);

        // Create timer triggering callback every 100ms
        timer_ = this->z_create_timer(100, [this]() -> void {
            this->timer_callback();
        });
    }


private:
    ZenohPublisher<z_SetLED>* pub_ = nullptr;
    ZenohTimer* timer_ = nullptr;
    int j_;
    uint8_t r_, g_, b_;

    void timer_callback() {
        z_SetLED msg;
        msg.r = r_;
        msg.g = g_;
        msg.b = b_;
        msg.brightness = 50;
        msg.led_num = j_;

        if (pub_) {
            pub_->publish(msg);
        }

        j_++;
        if (j_ == NUM_LEDS) {
            j_ = 0;
            // Generate random values in range [0, 150]
            r_ = std::rand() % 151;
            g_ = std::rand() % 151;
            b_ = std::rand() % 151;
        }
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // Connect to ESP32 SoftAP (default IP is 192.168.4.1)
    ZenohConfig config;
    config.host = "192.168.4.1";
    config.port = 7447;

    if (!ZenohNode::init(config)) {
        return -1;
    }

    {
        WS2812BPublisherNode node;
        node.z_spin();
    }

    return 0;
}
