"""
zenoh_ros.logging.viewer — Internal log viewer tool for zenoh_ros.
"""

import sys
import os
import msgpack
import zenoh
from zenoh_ros.node import ZenohNode, ZenohConfig
from zenoh_ros.logging import _Color, LOG_TOPIC


class LogViewerNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("ros2_log_viewer")
        print(f"[{_Color.CYAN}LOG VIEWER{_Color.RESET}] Listening live on Zenoh topic '{LOG_TOPIC}'...\n")

        # Declare a raw Zenoh subscriber directly on the underlying session
        self._raw_sub = self._session.declare_subscriber(LOG_TOPIC, self.log_callback)

    def log_callback(self, sample: zenoh.Sample) -> None:
        try:
            raw_payload = bytes(sample.payload)
            data = msgpack.unpackb(raw_payload, raw=False)
            severity = data.get("severity", "INFO")
            name = data.get("name", "unknown")
            msg = data.get("message", "")
            ts_ns = data.get("timestamp_ns", 0)
            sec = ts_ns / 1_000_000_000

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
