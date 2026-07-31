#include <Arduino.h>
#include <FastLED.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/custom_srvs/z_SetLEDColor.h>

#define LED_PIN     18
#define NUM_LEDS    16
#define BRIGHTNESS  50
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

using namespace custom_srvs;

void setLed(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    if (index < NUM_LEDS) {
        leds[index] = CRGB(r, g, b);
        FastLED.setBrightness(brightness);
        FastLED.show();
    }
}

class WS2812B_Service_Server_Node : public ZenohNode {
public:
    WS2812B_Service_Server_Node() : ZenohNode("ws2812b_service_server") {
        Serial.printf("[Node] %s has been started.\n", this->z_get_name());

        // Create ROS 2 Service Server for SetLEDColor custom service
        service_ = this->z_create_service<z_SetLEDColor>(
            "set_led_color",
            [this](const z_SetLEDColor::Request& req, z_SetLEDColor::Response& res) {
                this->handle_set_led_color(req, res);
            }
        );
    }

private:
    ZenohService<z_SetLEDColor>* service_ = nullptr;

    void handle_set_led_color(const z_SetLEDColor::Request& req, z_SetLEDColor::Response& res) {
        Serial.printf("[SERVICE RECV] LED: %u | RGB: (%u, %u, %u) | Brightness: %u\n",
                      req.led_data.led_num, req.led_data.r, req.led_data.g, req.led_data.b, req.led_data.brightness);

        if (req.led_data.led_num < NUM_LEDS) {
            setLed(req.led_data.led_num, req.led_data.r, req.led_data.g, req.led_data.b, req.led_data.brightness);
            res.success = true;
            res.message = "LED color updated successfully!";
        } else {
            res.success = false;
            res.message = "Index out of range!";
        }
    }
};

WS2812B_Service_Server_Node* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    delay(1000);

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    ZenohConfig cfg;
    cfg.ssid = "Robot_AP";
    cfg.password = "robot1234";

    if (ZenohNode::init(cfg)) {
        node_instance = new WS2812B_Service_Server_Node();
    } else {
        Serial.println("Failed to initialize Zenoh Node!");
    }
}

void loop() {
    if (node_instance) {
        node_instance->z_spin();
    }
}
