#!/usr/bin/env python3
import sys
import os
import random

# Ensure shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.custom_msgs import z_SetLED

NUM_LEDS = 16

class WS2812BPublisherNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("ws2812b_publisher")
        self.get_logger().info("Node has been started")

        self.j = 0
        self.r = 50
        self.g = 0
        self.b = 100

        # Create publisher on "ws2812b"
        self.pub = self.z_create_publisher(z_SetLED, "ws2812b", 10)

        # Create timer triggering callback every 100ms
        self.timer = self.z_create_timer(0.1, self.timer_callback)

    def timer_callback(self) -> None:
        msg = z_SetLED(
            r=self.r,
            g=self.g,
            b=self.b,
            brightness=50,
            led_num=self.j
        )
        self.pub.publish(msg)

        self.j += 1
        if self.j == NUM_LEDS:
            self.j = 0
            self.r = random.randint(0, 150)
            self.g = random.randint(0, 150)
            self.b = random.randint(0, 150)
            self.get_logger().info(f"New color cycle — r={self.r} g={self.g} b={self.b}")


def main() -> None:
    cfg = ZenohConfig(communication_mode=ZenohConfig.ZENOH_COMM_WIFI, host="192.168.4.1", port=7447)
    if not ZenohNode.init(cfg):
        return

    node_instance = None
    try:
        node_instance = WS2812BPublisherNode()
        node_instance.z_spin()

    except KeyboardInterrupt:
        if node_instance:
            node_instance.get_logger().info("Shutdown requested via KeyboardInterrupt.")

    except Exception as e:
        if node_instance:
            node_instance.get_logger().error(f"Critical Error: {e}")

    finally:
        ZenohNode.shutdown()



if __name__ == "__main__":
    main()
