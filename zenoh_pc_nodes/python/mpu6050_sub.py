#!/usr/bin/env python3
import sys
import os

# Ensure the shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.sensor_msgs import z_Imu

class MPU6050SubscriberNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("mpu6050_subscriber")
        print(f"[Node] {self.z_get_name()} has been started")

        # Create subscription to topic "robot/mpu6050" with standard ROS 2 z_Imu type
        self.sub = self.z_create_subscription(
            z_Imu,
            "robot/mpu6050",
            self.listener_callback,
            10
        )

    def listener_callback(self, msg: z_Imu) -> None:
        print(
            f"[IMU RECV] [stamp: {msg.header.stamp.sec}.{msg.header.stamp.nanosec:09d}] "
            f"Accel: ({msg.linear_acceleration.x:6.2f}, {msg.linear_acceleration.y:6.2f}, {msg.linear_acceleration.z:6.2f}) m/s² | "
            f"Gyro: ({msg.angular_velocity.x:6.2f}, {msg.angular_velocity.y:6.2f}, {msg.angular_velocity.z:6.2f}) rad/s"
        )


def main() -> None:
    # Connect to ESP32-S3 SoftAP IP (192.168.4.1)
    ZenohNode.init(host="192.168.4.1", port=7447)
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
