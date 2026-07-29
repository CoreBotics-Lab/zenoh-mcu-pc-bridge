#!/usr/bin/env python3
import sys
import os
import time
import colorsys
import random

# Ensure the shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.custom_msgs import z_SetLED

NUM_LEDS = 16

class WS2812BPublisherNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("ws2812b_publisher")
        print(f"[Node] {self.z_get_name()} has been started")

        # 1. Create a publisher on "/ws2812b" topic
        self.pub = self.z_create_publisher(z_SetLED, "ws2812b", 10)

        # Offset for the rainbow animation
        self.j = 0
        
        # Initial color values
        self.r = 50
        self.g = 0
        self.b = 100

        # 2. Create the timer (triggers callback every 1000ms)
        self.timer = self.z_create_timer(100, self.timer_callback)

    def timer_callback(self) -> None:
        msg = z_SetLED()
        msg.r = self.r
        msg.g = self.g
        msg.b = self.b
        msg.brightness = 50
        msg.led_num = self.j
        
        self.pub.publish(msg)
        self.j += 1
        if self.j == NUM_LEDS:
            self.j = 0
            # Generate a new random color for the next cycle
            self.r = random.randint(0, 150)
            self.g = random.randint(0, 150)
            self.b = random.randint(0, 150)


def main() -> None:
    # Connect to ESP32 SoftAP (ESP32-S3 IP is 192.168.4.1 by default in AP mode)
    config = ZenohConfig(host="192.168.4.1", port=7447)
    ZenohNode.init(config)
    node_instance = None
    try:
        node_instance = WS2812BPublisherNode()
        node_instance.z_spin()

    except KeyboardInterrupt:
        print("\n[CTRL+C]>>> Interrupted by the User.")
        
    except Exception as e:
        print(f"Critical Error: {e}")
        
    finally:
        if node_instance is not None:
            print("Destroying the Zenoh Node...")
            node_instance.z_destroy()


if __name__ == '__main__':
    main()
