#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 16

#define DATA_PIN 14

CRGB leds[NUM_LEDS];

void setLed(size_t led, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    if (led < NUM_LEDS) { 
        uint8_t scaled_brightness = (brightness * 255) / 100;
        
        CRGB color = CRGB(r, g, b);
        color.nscale8_video(scaled_brightness);
        leds[led] = color;
    }
}

void rainbowCycle(uint8_t brightness, uint8_t wait) {
    static uint16_t j = 0;
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV((i * 256 / NUM_LEDS + j) & 255, 255, ((brightness * 255) / 100));
        // leds[i].nscale8_video((brightness * 255) / 100);
    }
    FastLED.show();
    delay(wait);
    j++;
}

void setup() { 

    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
}

void loop() { 
//   setLed(0, 0, 255, 0, 5);
//   FastLED.show();
//   delay(500);
//   setLed(0, 0, 255, 0, 100);
//   FastLED.show();
//   delay(500);
    rainbowCycle(25, 1);
}