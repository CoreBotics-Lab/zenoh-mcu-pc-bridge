#!/usr/bin/env python3
import sys
import os

# Ensure the shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.custom_msgs import z_MPU6050Data

class MPU6050SubscriberNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("mpu6050_subscriber")
        print(f"[Node] {self.z_get_name()} has been started")

        # Create subscription to topic "robot/mpu6050" with z_MPU6050Data type
        self.sub = self.z_create_subscription(
            z_MPU6050Data,
            "robot/mpu6050",
            self.listener_callback,
            10
        )

    def listener_callback(self, msg: z_MPU6050Data) -> None:
        print(
            f"[IMU RECV] Accel: ({msg.accel_x:6.2f}, {msg.accel_y:6.2f}, {msg.accel_z:6.2f}) m/s² | "
            f"Gyro: ({msg.gyro_x:6.2f}, {msg.gyro_y:6.2f}, {msg.gyro_z:6.2f}) rad/s"
        )


def main() -> None:
    # Connect to ESP32-S3 SoftAP IP (192.168.4.1)
    config = ZenohConfig(host="192.168.4.1", port=7447)
    ZenohNode.init(config)
    node_instance = None
    try:
        node_instance = MPU6050SubscriberNode()
        node_instance.z_spin()

    except KeyboardInterrupt:
        print("\n[CTRL+C] Interrupted by user.")

    except Exception as e:
        print(f"Critical Error: {e}")

    finally:
        if node_instance is not None:
            print("Destroying the Zenoh Node...")
            node_instance.z_destroy()


if __name__ == '__main__':
    main()
