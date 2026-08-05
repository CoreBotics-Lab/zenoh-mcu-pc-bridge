import sys
import os
import time
import random

# Ensure the shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig, get_logger
from zenoh_ros.custom_srvs import z_SetLEDColor

def main():
    # 1. Target Host IP (can be passed via command line arg or default to SoftAP 192.168.4.1)
    host_ip = sys.argv[1] if len(sys.argv) > 1 else "192.168.4.1"
    config = ZenohConfig(host=host_ip, port=7447)
    ZenohNode.init(config)

    # 2. Create Zenoh ROS Node and obtain logger instance
    node = ZenohNode("led_client_python")
    logger = get_logger("led_client_python")

    logger.info("==================================================")
    logger.info("    Zenoh ROS 2 Service Client (Random RGB LED)   ")
    logger.info("==================================================")

    # 3. Create Service Client for SetLEDColor custom service
    service_name = "set_led_color"
    client = node.z_create_client(z_SetLEDColor, service_name)

    logger.info("Service client initialized on '%s' (%s:7447)...", service_name, host_ip)

    try:
        current_led = 0
        while True:
            try:
                led_num = current_led

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

                logger.info("[Service Request] Set LED #%d -> RGB(%d, %d, %d) Brightness=%d", led_num, r, g, b, brightness)

                # Call service asynchronously with ROS 2 Future pattern inside try/except block
                future = client.call_async(req)
                res = future.result(timeout_sec=3.0)

                if res and hasattr(res, 'success') and res.success:
                    logger.info("[Service Response] Success=%s | Message='%s'\n", res.success, res.message)
                    current_led = (current_led + 1) % 16
                else:
                    logger.warn("[Service Response] Call returned empty or failed response!\n")
            except Exception as e:
                logger.warn("[Service Response] Service call timed out or failed: %s\n", e)

            time.sleep(1.0)

    except KeyboardInterrupt:
        logger.info("Stopping client node...")
    finally:
        node.z_destroy()
        logger.info("Zenoh session cleanly closed.")

if __name__ == "__main__":
    main()
