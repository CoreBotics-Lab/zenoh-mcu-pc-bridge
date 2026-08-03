#!/usr/bin/env python3
"""
custom_nested_validator.py — PC Python Custom Nested Interface Validator
==========================================================================
Tests receiving custom nested message `z_RobotDiagnostic` and invoking custom
nested service `z_ConfigureRobot` against the MCU live.
"""

import sys, os, time, threading
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.custom_msgs import z_RobotDiagnostic, z_MotorStatus
from zenoh_ros.custom_srvs import z_ConfigureRobot

GRN  = "\033[32m"; RED  = "\033[31m"; YLW = "\033[33m"; CYN = "\033[36m"; BOLD = "\033[1m"; RST = "\033[0m"

received_msg = False
service_success = False

def cb_diag(msg: z_RobotDiagnostic):
    global received_msg
    if not received_msg:
        received_msg = True
        print(f"\n{GRN}{BOLD}✓ PASS  [Nested Custom Msg Received]{RST}")
        print(f"  Robot Name : {msg.robot_name}")
        print(f"  Header     : frame_id='{msg.header.frame_id}' sec={msg.header.stamp.sec}")
        print(f"  Velocity   : [{msg.velocity.x}, {msg.velocity.y}, {msg.velocity.z}]")
        print(f"  Left Motor : ID={msg.left_motor.motor_id} speed={msg.left_motor.speed} temp={msg.left_motor.temperature:.1f}")
        print(f"  Right Motor: ID={msg.right_motor.motor_id} speed={msg.right_motor.speed} temp={msg.right_motor.temperature:.1f}")

class CustomNestedValidator(ZenohNode):
    def __init__(self):
        super().__init__("python_custom_nested_validator")
        print(f"\n{CYN}{BOLD}╔══════════════════════════════════════════════════════╗")
        print(f"║  PC Python Custom Nested Interface Validator         ║")
        print(f"╚══════════════════════════════════════════════════════╝{RST}\n")

        self.z_create_subscription(z_RobotDiagnostic, "robot/diagnostics", cb_diag)
        self.cli_config = self.z_create_client(z_ConfigureRobot, "robot/configure")
        print(f"{CYN}Subscribed to 'robot/diagnostics' & Client created for 'robot/configure'{RST}\n")

def main():
    global service_success
    config = ZenohConfig(host="10.42.0.50", port=7447)
    ZenohNode.init(config)
    node = CustomNestedValidator()

    t = threading.Thread(target=node.z_spin, daemon=True)
    t.start()

    start = time.time()
    srv_called = False

    while time.time() - start < 10:
        time.sleep(0.1)

        # Call service once subscriber receives initial message
        if received_msg and not srv_called:
            srv_called = True
            print(f"{YLW}Invoking service 'robot/configure'...{RST}")
            req = z_ConfigureRobot.Request(
                target_status=z_MotorStatus(motor_id=1, speed=120.0, temperature=38.5, is_active=True),
                mode=2
            )
            res = node.cli_config.call(req, timeout_sec=3.0)
            if res and res.success:
                service_success = True
                print(f"{GRN}{BOLD}✓ PASS  [Nested Custom Service Call Response]{RST}")
                print(f"  Success       : {res.success}")
                print(f"  Status Message: '{res.status_message}'")
            else:
                print(f"{RED}✗ FAIL  Service call failed or timed out{RST}")

        if received_msg and service_success:
            break

    print(f"\n{BOLD}{'═'*54}")
    if received_msg and service_success:
        print(f"  {GRN}{BOLD}✓ ALL NESTED CUSTOM INTERFACES VERIFIED SUCCESSFULLY!{RST}")
    else:
        print(f"  {RED}{BOLD}⚠  TEST COMPLETED (Msg: {received_msg}, Srv: {service_success}){RST}")
    print(f"{'═'*54}{RST}\n")

    node.z_destroy()

if __name__ == "__main__":
    main()
