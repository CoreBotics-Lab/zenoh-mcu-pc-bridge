#include <zenoh_ros/ZenohRosPC.h>
#include <zenoh_ros/custom_msgs/z_SetLED.h>
#include <cstdlib>
#include <ctime>

const int NUM_LEDS = 16;

class WS2812BPublisherNode : public ZenohNode {
public:
    WS2812BPublisherNode() : ZenohNode("ws2812b_publisher"), j_(0), r_(50), g_(0), b_(100) {
        ZLOG_INFO(this->get_logger(), "Node has been started");

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
            ZLOG_DEBUG(this->get_logger(), "Published LED[%d] r=%d g=%d b=%d", j_, r_, g_, b_);
        }

        j_++;
        if (j_ == NUM_LEDS) {
            j_ = 0;
            // Generate random values in range [0, 150]
            r_ = std::rand() % 151;
            g_ = std::rand() % 151;
            b_ = std::rand() % 151;
            ZLOG_INFO(this->get_logger(), "New color cycle — r=%d g=%d b=%d", r_, g_, b_);
        }
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // Connect to ESP32 SoftAP via Wi-Fi (default IP is 192.168.4.1)
    ZenohConfig config;
    config.communication_mode = ZenohConfig::ZENOH_COMM_WIFI;
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
