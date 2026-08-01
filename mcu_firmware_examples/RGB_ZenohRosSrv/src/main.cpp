#include <Arduino.h>
#include <FastLED.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/z_logger.h>
#include <zenoh_ros/custom_srvs/z_SetLEDColor.h>

#define NUM_LEDS 16
#define DATA_PIN 14

CRGB leds[NUM_LEDS];

// --- Configuration Struct (AP Mode) ---
ZenohConfig cfg = {
    .ssid = "ESP32S3_Zenoh_AP",
    .password = "zenoh1234",
    .port = 7447
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
        : ZenohNode("ws2812b_service_server"),
          logger_("ws2812b_service_server") {
        
        // Attach node so logger publishes messages live to Zenoh topic 'zenoh_ros/log'
        logger_.z_attach(this);

        ZLOG_INFO(logger_, "Node '%s' has been started.", this->z_get_name());
        ZLOG_INFO_ONCE(logger_, "WS2812B FastLED initialized (NUM_LEDS=%d, DATA_PIN=%d)", NUM_LEDS, DATA_PIN);

        // Create ROS 2 Service Server for SetLEDColor custom service
        service_ = this->z_create_service<custom_srvs::z_SetLEDColor>(
            "set_led_color",
            [this](const custom_srvs::z_SetLEDColor::Request& req, custom_srvs::z_SetLEDColor::Response& res) -> void {
                this->handle_set_led_color(req, res);
            }
        );
    }

private:
    ZLogger logger_;
    ZenohService<custom_srvs::z_SetLEDColor>* service_ = nullptr;

    void handle_set_led_color(const custom_srvs::z_SetLEDColor::Request& req, custom_srvs::z_SetLEDColor::Response& res) {
        ZLOG_INFO(logger_, "[SERVICE RECV] LED: %u | RGB: (%u, %u, %u) | Brightness: %u%%",
                  req.led_data.led_num, req.led_data.r, req.led_data.g, req.led_data.b, req.led_data.brightness);

        if (req.led_data.led_num < NUM_LEDS) {
            setLed(req.led_data.led_num, req.led_data.r, req.led_data.g, req.led_data.b, req.led_data.brightness);
            FastLED.show();
            res.success = true;
            res.message = "LED color updated successfully!";
            ZLOG_INFO(logger_, "LED %u updated successfully", req.led_data.led_num);
        } else {
            res.success = false;
            res.message = "Index out of range!";
            ZLOG_ERROR(logger_, "Failed to update LED %u: Index out of range (max: %d)", req.led_data.led_num, NUM_LEDS - 1);
        }
    }
};

WS2812B_Service_Server_Node* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000); // USB CDC Serial delay for ESP32-S3

    Serial.println("\n==========================================");
    Serial.println("  ESP32-S3 Zenoh WS2812B Service Server Test");
    Serial.println("==========================================");

    // Initialize Zenoh Client SoftAP
    Serial.println("[System] Initializing the Zenoh Client...");
    if (ZenohNode::init(cfg)) {
        Serial.println("[System] Starting the Zenoh Service Server Node...");
        FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
        FastLED.clear(true);
        node_instance = new WS2812B_Service_Server_Node();
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
