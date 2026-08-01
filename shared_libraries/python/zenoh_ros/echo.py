"""
zenoh_ros.echo — Flexible topic subscriber tool (ROS 2 'ros2 topic echo' equivalent).

Subscribes to any topic (e.g., 'ws2812b_service_server/log', 'robot/mpu6050') and renders:
- Structured MsgPack log payloads (formatted with ANSI ROS 2 severity colors)
- Generic MsgPack payloads or string messages
- Connects automatically via Zenoh peer discovery (or connects to host/IP if specified)
"""

from __future__ import annotations

import sys
import os
import argparse
import msgpack
import zenoh
from typing import Optional, Dict

class _Color:
    RESET    = "\033[0m"
    CYAN     = "\033[36m"
    WHITE    = "\033[37m"
    YELLOW   = "\033[33m"
    RED      = "\033[31m"
    BOLD_RED = "\033[1;31m"

def echo_topic(topic: str, host: Optional[str] = None, port: int = 7447) -> None:
    # Ensure standard ROS-like prefixing if needed or keep exact
    topic_clean = topic.strip()
    if not topic_clean:
        print("\033[31m[zenoh_ros echo] Error: Topic name cannot be empty.\033[0m")
        return

    print("\033[36m==========================================\033[0m")
    print(f"\033[36m  zenoh_ros Echo Tool — Topic: '{topic_clean}'\033[0m")
    print("\033[36m==========================================\033[0m")

    target_host = host if host else "192.168.4.1"
    z_conf = zenoh.Config()
    endpoints = [f"tcp/{target_host}:{port}"]
    z_conf.insert_json5("connect/endpoints", str(endpoints).replace("'", '"'))
    print(f"[zenoh_ros echo] Connecting to {target_host}:{port}...")

    try:
        session = zenoh.open(z_conf)
    except Exception as e:
        print(f"\033[31m[zenoh_ros echo] Error opening Zenoh session: {e}\033[0m")
        return

    print(f"\033[36m[zenoh_ros echo] Subscribed live on topic '{topic_clean}'. Press Ctrl+C to stop.\033[0m\n")

    def _callback(sample: zenoh.Sample) -> None:
        try:
            payload_bytes = bytes(sample.payload)
            # 1. Try MsgPack deserialization
            try:
                data = msgpack.unpackb(payload_bytes, raw=False)
                if isinstance(data, dict) and "severity" in data and "message" in data:
                    # Formatted Log Event
                    severity = str(data.get("severity", "INFO"))
                    name     = str(data.get("name", "node"))
                    message  = str(data.get("message", ""))
                    ts_ns    = data.get("timestamp_ns", 0)
                    sec      = ts_ns / 1_000_000_000

                    color = _Color.WHITE
                    if severity == "DEBUG":      color = _Color.CYAN
                    elif severity == "WARN":     color = _Color.YELLOW
                    elif severity == "ERROR":    color = _Color.RED
                    elif severity == "FATAL":    color = _Color.BOLD_RED

                    print(f"{color}[{severity}] [{sec:.9f}] [{name}]: {message}{_Color.RESET}")
                else:
                    # Structured message dict
                    print(f"--- [{sample.key_expr}] ---")
                    print(data)
            except Exception:
                # 2. Plain string fallback
                text = payload_bytes.decode('utf-8', errors='replace')
                print(f"[{sample.key_expr}]: {text}")

        except Exception as err:
            print(f"\033[31m[echo error]: {err}\033[0m")

    sub = session.declare_subscriber(topic_clean, _callback)

    try:
        import time
        while True:
            time.sleep(1.0)
    except KeyboardInterrupt:
        print("\n\033[36m[zenoh_ros echo] Stopped by user.\033[0m")
    finally:
        sub.undeclare()
        session.close()

def main() -> None:
    parser = argparse.ArgumentParser(description="zenoh_ros Topic Echo Tool (ROS 2 style)")
    parser.add_argument("topic", type=str, help="Topic name to subscribe and echo (e.g. 'ws2812b_service_server/log', 'robot/mpu6050')")
    parser.add_argument("--host", type=str, default=None, help="Optional target IP (if omitted, auto-discovers peers on AP/STA)")
    parser.add_argument("--port", type=int, default=7447, help="Target port (default: 7447)")
    args = parser.parse_args()

    echo_topic(args.topic, host=args.host, port=args.port)

if __name__ == '__main__':
    main()
