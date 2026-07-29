#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 1

#define DATA_PIN 48

CRGB leds[NUM_LEDS];

void setLed(int led, int r, int g, int b, int brightness){
    //Convert brightness percentage to 0-255
    brightness = (brightness * 255) / 100;

    if(led < NUM_LEDS){ 
    CRGB color = CRGB(r, g, b);
    color.nscale8_video(brightness);
    leds[led] = color;
    FastLED.show();
}
}
void setup() { 

    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
}

void loop() { 
  // Turn the LED on, then pause
  setLed(0, 0, 255, 0, 5);
  delay(500);
  // Now turn the LED off, then pause
  setLed(0, 0, 255, 0, 100);
  delay(500);
}