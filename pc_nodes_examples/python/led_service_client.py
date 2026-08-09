#!/usr/bin/env python3
import sys
import os
import random

# Ensure the shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.custom_srvs import z_SetLEDColor

class LEDServiceClientNode(ZenohNode):
    def __init__(self, service_name: str = "set_led_color") -> None:
        super().__init__("led_client_python")

        self.get_logger().info("==================================================")
        self.get_logger().info("    Zenoh ROS 2 Service Client (Random RGB LED)   ")
        self.get_logger().info("==================================================")

        self.service_name = service_name
        self.client = self.z_create_client(z_SetLEDColor, self.service_name)
        self.get_logger().info("Service client initialized on '%s'...", self.service_name)

        self.current_led = 0
        self._first_time = True

        # Create ROS 2 timer triggering callback every 1000ms (1.0s)
        self.timer = self.z_create_timer(1000, self.timer_callback)

    def timer_callback(self) -> None:
        try:
            if self._first_time:
                self.get_logger().info("Resetting all 16 LEDs off on startup...")
                for i in range(16):
                    reset_req = z_SetLEDColor.Request()
                    reset_req.led_data.led_num = i
                    reset_req.led_data.r = 0
                    reset_req.led_data.g = 0
                    reset_req.led_data.b = 0
                    reset_req.led_data.brightness = 0
                    self.client.call_async(reset_req)
                self._first_time = False
                self.z_delay(200)

            led_num = self.current_led

            r = random.randint(0, 255)
            g = random.randint(0, 255)
            b = random.randint(0, 255)
            brightness = 60

            # Construct service request with nested led_data
            req = z_SetLEDColor.Request()
            req.led_data.led_num = led_num
            req.led_data.r = r
            req.led_data.g = g
            req.led_data.b = b
            req.led_data.brightness = brightness

            self.get_logger().info("[Service Request] Set LED #%d -> RGB(%d, %d, %d) Brightness=%d", led_num, r, g, b, brightness)

            # Call service asynchronously with ROS 2 Future pattern inside try/except block
            future = self.client.call_async(req)
            res = future.result(timeout_sec=3.0)

            if res and hasattr(res, 'success') and res.success:
                self.get_logger().info("[Service Response] Success=%s | Message='%s'\n", res.success, res.message)
                self.current_led = (self.current_led + 1) % 16
            else:
                self.get_logger().warn("[Service Response] Call returned empty or failed response!\n")
        except Exception as e:
            self.get_logger().warn("[Service Response] Service call timed out or failed: %s\n", e)


def main() -> None:
    host_ip = sys.argv[1] if len(sys.argv) > 1 else "192.168.4.1"
    cfg = ZenohConfig(host=host_ip, port=7447)
    if not ZenohNode.init(cfg):
        return

    node_instance = None
    try:
        node_instance = LEDServiceClientNode()
        node_instance.z_spin()
    except KeyboardInterrupt:
        if node_instance:
            node_instance.get_logger().info("Stopping client node...")
    except Exception as e:
        if node_instance:
            node_instance.get_logger().error(f"Critical Error: {e}")
    finally:
        ZenohNode.shutdown()
        if node_instance:
            node_instance.get_logger().info("Zenoh session cleanly closed.")

if __name__ == "__main__":
    main()
