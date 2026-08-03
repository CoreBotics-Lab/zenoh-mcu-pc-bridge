#!/usr/bin/env python3
"""
rigorous_nested_validator.py — PC Python Rigorous Multi-Level Custom Interface Validator
========================================================================================
Tests receiving multi-level nested custom message `z_ComplexRobotState` and invoking
custom service `z_FullSystemControl` against MCU live.
"""

import sys, os, time, threading
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.custom_msgs import z_ComplexRobotState, z_SensorTelemetry
from zenoh_ros.custom_srvs import z_FullSystemControl

GRN  = "\033[32m"; RED  = "\033[31m"; YLW = "\033[33m"; CYN = "\033[36m"; BOLD = "\033[1m"; RST = "\033[0m"

received_msg = False
service_success = False

def cb_state(msg: z_ComplexRobotState):
    global received_msg
    if not received_msg:
        received_msg = True
        print(f"\n{GRN}{BOLD}✓ PASS  [Python Multi-Level Custom Msg Received]{RST}")
        print(f"  Robot Mode : {msg.robot_mode} (Cycle: {msg.cycle_count})")
        print(f"  Header     : frame_id='{msg.header.frame_id}' sec={msg.header.stamp.sec}")
        print(f"  Pose       : Pos=[{msg.pose.position.x}, {msg.pose.position.y}, {msg.pose.position.z}] Ori.w={msg.pose.orientation.w}")
        print(f"  Telemetry  : Sensor ID={msg.telemetry.sensor_id} Temp={msg.telemetry.temp:.1f}C StatusOK={msg.telemetry.status_ok}")
        print(f"               Telemetry Header frame_id='{msg.telemetry.header.frame_id}'")
        print(f"               Accel=[{msg.telemetry.accel.x}, {msg.telemetry.accel.y}, {msg.telemetry.accel.z}]")

class RigorousPythonValidator(ZenohNode):
    def __init__(self):
        super().__init__("python_rigorous_validator")
        print(f"\n{CYN}{BOLD}╔══════════════════════════════════════════════════════╗")
        print(f"║  PC Python Rigorous Multi-Level Custom Validator     ║")
        print(f"╚══════════════════════════════════════════════════════╝{RST}\n")

        self.z_create_subscription(z_ComplexRobotState, "system/state", cb_state)
        self.cli_control = self.z_create_client(z_FullSystemControl, "system/control")
        print(f"{CYN}Subscribed to 'system/state' & Client ready on 'system/control'{RST}\n")

def main():
    global service_success
    config = ZenohConfig(host="10.42.0.50", port=7447)
    ZenohNode.init(config)
    node = RigorousPythonValidator()

    t = threading.Thread(target=node.z_spin, daemon=True)
    t.start()

    start = time.time()
    srv_called = False

    while time.time() - start < 12:
        time.sleep(0.1)

        if received_msg and not srv_called:
            srv_called = True
            print(f"{YLW}Invoking service 'system/control'...{RST}")
            req = z_FullSystemControl.Request(
                target_state=z_ComplexRobotState(
                    robot_mode="TEST_MODE_EXEC",
                    cycle_count=999
                ),
                command_code=7
            )
            res = node.cli_control.call(req, timeout_sec=3.0)
            if res and res.ack:
                service_success = True
                print(f"{GRN}{BOLD}✓ PASS  [Python Multi-Level Custom Service Call Response]{RST}")
                print(f"  Ack           : {res.ack}")
                print(f"  Status Details: '{res.status_details}'")
                print(f"  Curr Telemetry: Sensor ID={res.current_telemetry.sensor_id} Temp={res.current_telemetry.temp:.1f}C")
            else:
                print(f"{RED}✗ FAIL  Service call failed or timed out{RST}")

        if received_msg and service_success:
            break

    print(f"\n{BOLD}{'═'*54}")
    if received_msg and service_success:
        print(f"  {GRN}{BOLD}✓ PYTHON MULTI-LEVEL CUSTOM INTERFACES VERIFIED 100%!{RST}")
    else:
        print(f"  {RED}{BOLD}⚠  PYTHON TEST FAILED (Msg: {received_msg}, Srv: {service_success}){RST}")
    print(f"{'═'*54}{RST}\n")

    node.z_destroy()

if __name__ == "__main__":
    main()
