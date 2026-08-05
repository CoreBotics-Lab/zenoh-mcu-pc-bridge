#!/usr/bin/env python3
import sys
import os

# Ensure the shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig, CommunicationMode, USB_HIGH_SPEED
from zenoh_ros.z_std_msgs import z_Int32, z_Float32

class SerialUARTListenerNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("serial_uart_listener_python")

        self.get_logger().info("==================================================")
        self.get_logger().info("    Zenoh ROS 2 Serial UART Subscriber Node       ")
        self.get_logger().info("==================================================")

        # Subscribe to MCU counter topic
        self.sub_count = self.z_create_subscription(
            z_Int32,
            "serial/counter",
            self.counter_callback,
            10
        )

        # Subscribe to MCU temperature topic
        self.sub_temp = self.z_create_subscription(
            z_Float32,
            "serial/temperature",
            self.temp_callback,
            10
        )

    def counter_callback(self, msg: z_Int32) -> None:
        self.get_logger().info("[Received] Counter: %d", msg.data)

    def temp_callback(self, msg: z_Float32) -> None:
        self.get_logger().info("[Received] Temperature: %.2f °C", msg.data)


def main() -> None:
    # Auto-detect connected MCU serial port (/dev/ttyACM0, /dev/ttyUSB0, or COM*) at USB_HIGH_SPEED (12 Mbps)
    config = ZenohConfig(
        communication_mode=CommunicationMode.SERIAL,
        uart_port="auto",
        baudrate=USB_HIGH_SPEED
    )
    ZenohNode.init(config)

    node_instance = None
    try:
        node_instance = SerialUARTListenerNode()
        node_instance.z_spin()
    except KeyboardInterrupt:
        if node_instance:
            node_instance.get_logger().info("Stopping serial listener node...")
    except Exception as e:
        print(f"Critical Error: {e}")
    finally:
        if node_instance:
            node_instance.z_destroy()
            node_instance.get_logger().info("Zenoh session cleanly closed.")

if __name__ == "__main__":
    main()
