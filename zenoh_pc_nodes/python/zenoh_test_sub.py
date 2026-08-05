#!/usr/bin/env python3
import sys
import os

# Ensure the shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.std_msgs import z_Int32

class CounterSubscriberNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("counter_subscriber")
        print(f"[Node] {self.z_get_name()} has been started")

        # Create subscription with standard ROS 2 message type (Int32) and depth 10
        self.sub = self.z_create_subscription(
            z_Int32,
            "robot/sim_counter",
            self.listener_callback,
            10
        )

    def listener_callback(self, msg: z_Int32) -> None:
        # Message is already deserialized into standard z_Int32 class!
        print(f"[RECV FROM ESP32-S3] sim_counter: {msg.data}")


def main() -> None:
    cfg = ZenohConfig(host="10.42.0.50", port=7447)
    if not ZenohNode.init(cfg):
        return
    node_instance = None
    try:
        node_instance = CounterSubscriberNode()
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