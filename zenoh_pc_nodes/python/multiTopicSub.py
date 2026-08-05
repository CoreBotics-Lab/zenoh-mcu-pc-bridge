#!/usr/bin/env python3
import sys
import os

# Ensure shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.std_msgs import z_Int32, z_String

class MultiTopicSubscriberNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("multi_topic_subscriber")
        self.get_logger().info("Node has been started")

        # Create subscription to robot/sim_counter (z_Int32)
        self.counter_sub = self.z_create_subscription(
            z_Int32,
            "robot/sim_counter",
            self.counter_listener_callback,
            10
        )

        # Create subscription to robot/hello_string (z_String)
        self.string_sub = self.z_create_subscription(
            z_String,
            "robot/hello_string",
            self.string_listener_callback,
            10
        )

    def counter_listener_callback(self, msg: z_Int32) -> None:
        self.get_logger().info(f"[RECV COUNTER] sim_counter: {msg.data}")

    def string_listener_callback(self, msg: z_String) -> None:
        self.get_logger().info(f"[RECV STRING] hello_string: {msg.data}")


def main() -> None:
    cfg = ZenohConfig(host="192.168.4.1", port=7447)
    if not ZenohNode.init(cfg):
        return

    node_instance = None
    try:
        node_instance = MultiTopicSubscriberNode()
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