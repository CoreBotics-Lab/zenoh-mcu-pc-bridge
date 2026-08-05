#include <Arduino.h>
#include <FastLED.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/custom_srvs/z_SetLEDColor.h>

#define NUM_LEDS 16
#define DATA_PIN 14

CRGB leds[NUM_LEDS];

// --- Configuration Struct (AP Mode) ---
ZenohConfig cfg = {
    .communication_mode = ZenohCommunicationMode::ZENOH_COMM_WIFI,
    .ssid               = "ESP32S3_Zenoh_AP",
    .password           = "zenoh1234",
    .port               = 7447,
    .wifi_mode          = WIFI_AP
};

void setLed(size_t led, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    if (led < NUM_LEDS) { 
        uint8_t scaled_brightness = (brightness * 255) / 100;
        CRGB color = CRGB(r, g, b);
        color.nscale8_video(scaled_brightness);
        leds[led] = color;
    }
}

class WS2812B_Service_Server_Node : public ZenohNode {
public:
    WS2812B_Service_Server_Node()
        : ZenohNode("ws2812b_service_server") {

        ZLOG_INFO(this->get_logger(), "Node '%s' has been started.", this->z_get_name());
        ZLOG_INFO_ONCE(this->get_logger(), "WS2812B FastLED initialized (NUM_LEDS=%d, DATA_PIN=%d)", NUM_LEDS, DATA_PIN);

        // Create ROS 2 Service Server for SetLEDColor custom service
        service_ = this->z_create_service<z_SetLEDColor>(
            "set_led_color",
            [this](const z_SetLEDColor::Request& req, z_SetLEDColor::Response& res) -> void {
                this->handle_set_led_color(req, res);
            }
        );
    }

private:
    ZenohService<z_SetLEDColor>* service_ = nullptr;

    void handle_set_led_color(const z_SetLEDColor::Request& req, z_SetLEDColor::Response& res) {
        ZLOG_INFO(this->get_logger(), "[SERVICE RECV] LED: %u | RGB: (%u, %u, %u) | Brightness: %u%%",
                  req.led_data.led_num, req.led_data.r, req.led_data.g, req.led_data.b, req.led_data.brightness);

        if (req.led_data.led_num < NUM_LEDS) {
            setLed(req.led_data.led_num, req.led_data.r, req.led_data.g, req.led_data.b, req.led_data.brightness);
            FastLED.show();
            res.success = true;
            res.message = "LED color updated successfully!";
        } else {
            res.success = false;
            res.message = "Invalid LED index!";
            ZLOG_WARN(this->get_logger(), "[SERVICE ERROR] Invalid LED index: %u (max: %d)", req.led_data.led_num, NUM_LEDS - 1);
        }
    }
};

WS2812B_Service_Server_Node* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000); // USB CDC Serial delay for ESP32-S3

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.clear();
    FastLED.show();

    ZLOG_INFO(z_get_logger("system"), "==========================================");
    ZLOG_INFO(z_get_logger("system"), "  ESP32-S3 Zenoh RGB Service Server Demo ");
    ZLOG_INFO(z_get_logger("system"), "==========================================");

    ZLOG_INFO(z_get_logger("system"), "Initializing Zenoh Client...");
    if (ZenohNode::init(cfg)) {
        ZLOG_INFO(z_get_logger("system"), "Starting WS2812B Service Server Node...");
        node_instance = new WS2812B_Service_Server_Node();
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
