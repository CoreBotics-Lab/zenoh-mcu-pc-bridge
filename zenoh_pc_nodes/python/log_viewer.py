#!/usr/bin/env python3
"""
log_viewer.py - Standalone Python node to monitor live logs published to '/zenoh_ros/log'.

Receives msgpack-serialized log events from MCU or PC nodes over Zenoh and prints
them formatted with matching ROS 2 ANSI severity colors.
"""

import sys
import os
import msgpack

# Ensure the shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.logging import _Color, LOG_TOPIC


class LogViewerNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("log_viewer")
        print(f"[{_Color.CYAN}LOG VIEWER{_Color.RESET}] Listening live on Zenoh topic '{LOG_TOPIC}'...\n")

        # Subscribe directly to raw bytes on topic 'zenoh_ros/log'
        self.sub = self.z_create_raw_subscription(
            LOG_TOPIC,
            self.log_callback,
            10
        )

    def log_callback(self, raw_payload: bytes) -> None:
        try:
            data = msgpack.unpackb(raw_payload, raw=False)
            severity = data.get("severity", "INFO")
            name = data.get("name", "unknown")
            msg = data.get("message", "")
            ts_ns = data.get("timestamp_ns", 0)
            sec = ts_ns / 1_000_000_000

            # Color mapping
            color = _Color.WHITE
            if severity == "DEBUG":
                color = _Color.CYAN
            elif severity == "WARN":
                color = _Color.YELLOW
            elif severity == "ERROR":
                color = _Color.RED
            elif severity == "FATAL":
                color = _Color.BOLD_RED

            print(f"{color}[{severity}] [{sec:.9f}] [{name}]: {msg}{_Color.RESET}")

        except Exception as e:
            print(f"[LOG VIEWER ERROR] Failed to unpack log message: {e}")


def main() -> None:
    # Connect to ESP32-S3 SoftAP IP (192.168.4.1) or local peer
    host = os.getenv("ZENOH_HOST", "192.168.4.1")
    port = int(os.getenv("ZENOH_PORT", "7447"))
    config = ZenohConfig(host=host, port=port)

    ZenohNode.init(config)
    node_instance = None
    try:
        node_instance = LogViewerNode()
        node_instance.z_spin()

    except KeyboardInterrupt:
        print("\n[CTRL+C] Log Viewer stopped by user.")

    except Exception as e:
        print(f"Critical Error: {e}")

    finally:
        if node_instance is not None:
            node_instance.z_destroy()


if __name__ == '__main__':
    main()
