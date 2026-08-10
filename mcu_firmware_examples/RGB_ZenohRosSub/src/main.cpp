#include <Arduino.h>
#include <FastLED.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/custom_msgs/z_SetLED.h>

#define NUM_LEDS 16
#define DATA_PIN 14

CRGB leds[NUM_LEDS];

// --- Configuration Struct (AP Mode) ---
ZenohConfig cfg = ZenohConfig()
    .set_communication_mode(ZenohConfig::ZENOH_COMM_WIFI)
    .set_wifi("ESP32S3_Zenoh_AP", "zenoh1234", WIFI_AP)
    .set_port(7447);

void setLed(size_t led, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    if (led < NUM_LEDS) { 
        uint8_t scaled_brightness = (brightness * 255) / 100;
        CRGB color = CRGB(r, g, b);
        color.nscale8_video(scaled_brightness);
        leds[led] = color;
    }
}

class WS2812B_Subscriber_Node : public ZenohNode {
public:
    WS2812B_Subscriber_Node() : ZenohNode("ws2812b_subscriber") {
        ZLOG_INFO(this->get_logger(), "Node %s has been started.", this->z_get_name());
        
        // Create subscriber for ws2812b topic
        sub_ = this->z_create_subscription<z_SetLED>(
            "ws2812b", 
            [this](const z_SetLED& msg) -> void {
                this->callback_set_led(msg);
            }, 
            10
        );
    }

private:
    ZenohSubscription<z_SetLED>* sub_ = nullptr;

    void callback_set_led(const z_SetLED& msg) {
        setLed(msg.led_num, msg.r, msg.g, msg.b, msg.brightness);
        FastLED.show();
        ZLOG_INFO(this->get_logger(), "[LED RECV] Set LED[%d] r=%d g=%d b=%d brightness=%d",
                  msg.led_num, msg.r, msg.g, msg.b, msg.brightness);
    }
};

WS2812B_Subscriber_Node* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000); // USB CDC Serial delay for ESP32-S3

    ZLOG_INFO(z_get_logger("system"), "==========================================");
    ZLOG_INFO(z_get_logger("system"), "  ESP32-S3 Zenoh WS2812B Subscriber Test");
    ZLOG_INFO(z_get_logger("system"), "==========================================");

    ZLOG_INFO(z_get_logger("system"), "Initializing the Zenoh Client...");
    if (ZenohNode::init(cfg)) {
        ZLOG_INFO(z_get_logger("system"), "Starting the Zenoh Subscriber Node...");
        FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
        FastLED.clear(true);
        node_instance = new WS2812B_Subscriber_Node();
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