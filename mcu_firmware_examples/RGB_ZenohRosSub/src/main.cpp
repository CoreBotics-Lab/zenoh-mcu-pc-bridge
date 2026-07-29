#include <Arduino.h>
#include <FastLED.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/custom_msgs/SetLED.h>


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

class WS2812B_Subscriber_Node : public ZenohNode {
public:
    WS2812B_Subscriber_Node() : ZenohNode("ws2812b_subscriber") {
        Serial.printf("[Node] %s has been started.\n", this->z_get_name());
        
        // Create subscriber for /ws2812b topic
        sub_ = this->z_create_subscription<custom_msgs::z_SetLED>(
            "ws2812b", 
            [this](const custom_msgs::z_SetLED& msg) -> void {
                this->callback_set_led(msg);
            }, 
            10
        );
    }

    ~WS2812B_Subscriber_Node() {
        if (sub_) {
            delete sub_;
        }
    }

private:
    ZenohSubscription<custom_msgs::z_SetLED>* sub_ = nullptr;

    void callback_set_led(const custom_msgs::z_SetLED& msg) {
        // Set the led color and show it
        setLed(msg.led_num, msg.r, msg.g, msg.b, msg.brightness);
        FastLED.show();
    }
};

WS2812B_Subscriber_Node* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000); // USB CDC Serial delay for ESP32-S3

    Serial.println("\n==========================================");
    Serial.println("  ESP32-S3 Zenoh WS2812B Subscriber Test");
    Serial.println("==========================================");

    // Initialize Zenoh Client SoftAP
    Serial.println("[System] Initializing the Zenoh Client...");
    if (ZenohNode::init(cfg)) {
        Serial.println("[System] Starting the Zenoh Subscriber Node...");
        FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
        // Clear leds
        FastLED.clear(true);
        node_instance = new WS2812B_Subscriber_Node();
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