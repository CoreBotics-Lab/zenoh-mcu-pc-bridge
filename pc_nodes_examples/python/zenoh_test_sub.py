#!/usr/bin/env python3
import sys
import os

# Ensure shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.std_msgs import z_Int32

class CounterSubscriberNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("counter_subscriber")
        self.get_logger().info("Node has been started")

        # Create subscription to standard ROS 2 message type (z_Int32) and depth 10
        self.sub = self.z_create_subscription(
            z_Int32,
            "robot/sim_counter",
            self.listener_callback,
            10
        )

    def listener_callback(self, msg: z_Int32) -> None:
        self.get_logger().info(f"[RECV FROM MCU] sim_counter: {msg.data}")


def main() -> None:
    cfg = ZenohConfig(communication_mode=ZenohConfig.ZENOH_COMM_WIFI, host="10.42.0.50", port=7447)
    if not ZenohNode.init(cfg):
        return

    node_instance = None
    try:
        node_instance = CounterSubscriberNode()
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