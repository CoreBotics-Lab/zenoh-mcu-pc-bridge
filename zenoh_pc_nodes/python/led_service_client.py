import sys
import os
import time
import random

# Ensure the shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode
from zenoh_ros.custom_srvs import z_SetLEDColor

def main():
    print("==================================================")
    print("    Zenoh ROS 2 Service Client (Random RGB LED)   ")
    print("==================================================")

    # 1. Create Zenoh ROS Node
    node = ZenohNode("led_client_python")

    # 2. Create Service Client for SetLEDColor
    client = node.z_create_client(z_SetLEDColor, "set_led_color")

    print("[Client] Waiting for MCU Service Server on 'set_led_color'...")
    if not client.wait_for_service(timeout_sec=10.0):
        print("[Client] ERROR: Service server on 'set_led_color' unavailable!")
        node.z_destroy()
        return

    print("[Client] Service connected! Beginning sequential RGB LED command loop (0-15)...")

    try:
        current_led = 0
        while True:
            led_num = current_led
            current_led = (current_led + 1) % 16

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

            print(f"[Client -> Service Request] Set LED #{led_num} -> RGB({r}, {g}, {b}) Brightness={brightness}")

            # Call service asynchronously with ROS 2 Future pattern
            future = client.call_async(req)
            res = future.result(timeout_sec=3.0)

            if res:
                print(f"[Service Response] Success={res.success} | Message='{res.message}'\n")
            else:
                print("[Service Response] Call timed out or failed!\n")

            time.sleep(1.0)

    except KeyboardInterrupt:
        print("\n[Client] Stopping client node...")
    finally:
        node.z_destroy()
        print("[Client] Zenoh session cleanly closed.")

if __name__ == "__main__":
    main()
