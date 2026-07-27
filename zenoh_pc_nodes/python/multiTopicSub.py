#!/usr/bin/env python3
import sys
import os

# Ensure the local ros2_zenoh package can be imported
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from ros2_zenoh import ZenohNode, z_std_msgs

class MultiTopicSubscriberNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("multi_topic_subscriber")
        print(f"[Node] {self.z_get_name()} has been started")

        # Create subscription with standard ROS 2 message type (Int32) and depth 10
        self.counter_sub = self.z_create_subscription(
            z_std_msgs.Int32,
            "robot/sim_counter",
            self.counter_listener_callback,
            10
        )

        # Create subscription with standard ROS 2 message type (String) and depth 10
        self.string_sub = self.z_create_subscription(
            z_std_msgs.String,
            "robot/hello_string",
            self.string_listener_callback,
            10
        )

    def counter_listener_callback(self, msg: z_std_msgs.Int32) -> None:
        print(f"[RECV COUNTER] sim_counter: {msg.data}")

    def string_listener_callback(self, msg: z_std_msgs.String) -> None:
        print(f"[RECV STRING] hello_string: {msg.data}")


def main() -> None:
    node_instance = None
    try:
        node_instance = MultiTopicSubscriberNode()
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