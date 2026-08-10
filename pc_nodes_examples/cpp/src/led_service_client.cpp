#include <zenoh_ros/ZenohRosPC.h>
#include <zenoh_ros/custom_srvs/z_SetLEDColor.h>
#include <cstdlib>
#include <ctime>

class LEDServiceClientNode : public ZenohNode {
public:
    LEDServiceClientNode(const char* service_name = "set_led_color")
        : ZenohNode("led_client_cpp"),
          service_name_(service_name),
          current_led_(0),
          first_time_(true) {

        ZLOG_INFO(this->get_logger(), "==================================================");
        ZLOG_INFO(this->get_logger(), "    Zenoh ROS 2 Service Client (Random RGB LED)   ");
        ZLOG_INFO(this->get_logger(), "==================================================");

        // Seed random number generator
        std::srand(std::time(nullptr));

        // Create service client for set_led_color service
        client_ = this->z_create_client<z_SetLEDColor>(service_name_);
        ZLOG_INFO(this->get_logger(), "Service client initialized on '%s'...", service_name_);

        // Create ROS 2 timer triggering callback every 1000ms (1.0s)
        timer_ = this->z_create_timer(1000, [this]() -> void {
            this->timer_callback();
        });
    }

private:
    const char* service_name_;
    ZenohClient<z_SetLEDColor>* client_ = nullptr;
    ZenohTimer* timer_ = nullptr;
    int current_led_;
    bool first_time_;

    void timer_callback() {
        if (!client_ || shutdown_requested) return;

        if (first_time_) {
            ZLOG_INFO(this->get_logger(), "Resetting all 16 LEDs off on startup...");
            for (int i = 0; i < 16; ++i) {
                if (shutdown_requested) return;
                z_SetLEDColor::Request reset_req;
                reset_req.led_data.led_num = i;
                reset_req.led_data.r = 0;
                reset_req.led_data.g = 0;
                reset_req.led_data.b = 0;
                reset_req.led_data.brightness = 0;

                z_SetLEDColor::Response reset_res;
                client_->call(reset_req, reset_res, 1000);
            }
            first_time_ = false;
            this->z_delay(200);
        }

        if (shutdown_requested) return;

        int led_num = current_led_;
        uint8_t r = std::rand() % 256;
        uint8_t g = std::rand() % 256;
        uint8_t b = std::rand() % 256;
        uint8_t brightness = 60;

        // Construct service request with nested led_data
        z_SetLEDColor::Request req;
        req.led_data.led_num = led_num;
        req.led_data.r = r;
        req.led_data.g = g;
        req.led_data.b = b;
        req.led_data.brightness = brightness;

        ZLOG_INFO(this->get_logger(),
                  "[Service Request] Set LED #%d -> RGB(%d, %d, %d) Brightness=%d",
                  led_num, r, g, b, brightness);

        z_SetLEDColor::Response res;
        bool success = client_->call(req, res, 3000);

        if (shutdown_requested) return;

        if (success && res.success) {
            ZLOG_INFO(this->get_logger(),
                      "[Service Response] Success=True | Message='%s'\n",
                      res.message.c_str());
            current_led_ = (current_led_ + 1) % 16;
        } else {
            ZLOG_WARN(this->get_logger(),
                      "[Service Response] Call returned empty or failed response!\n");
        }
    }
};

int main(int argc, char** argv) {
    ZenohConfig config;
    if (argc > 1) {
        config.communication_mode = ZenohConfig::ZENOH_COMM_WIFI;
        config.host = argv[1];
        config.port = 7447;
    } else {
        config.communication_mode = ZenohConfig::ZENOH_COMM_WIFI;
        config.host = "192.168.4.1";
        config.port = 7447;
    }

    if (!ZenohNode::init(config)) {
        return -1;
    }

    {
        LEDServiceClientNode node;
        node.z_spin();
    }

    return 0;
}
