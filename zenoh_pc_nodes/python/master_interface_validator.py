#!/usr/bin/env python3
"""
master_interface_validator.py — Monolithic All-In-One Interface Validator
========================================================================
Tests 27 Standard ROS 2 Msgs + 7 Custom Msgs + 4 Custom Services against MCU live.
"""

import sys, os, time, threading
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.std_msgs import (
    z_Bool, z_Int8, z_UInt8, z_Int16, z_UInt16, z_Int32, z_UInt32, z_Int64, z_UInt64,
    z_Float32, z_Float64, z_String, z_Header, z_Int32MultiArray, z_Float64MultiArray
)
from zenoh_ros.sensor_msgs import z_Imu, z_Temperature, z_Range, z_RelativeHumidity, z_BatteryState, z_NavSatFix, z_JoyFeedback, z_JointState
from zenoh_ros.geometry_msgs import z_Vector3, z_Quaternion, z_Pose, z_Twist

from zenoh_ros.custom_msgs import (
    z_SetLED, z_MPU6050Data, z_RobotState, z_MotorStatus, z_SensorTelemetry, z_RobotDiagnostic, z_ComplexRobotState
)
from zenoh_ros.custom_srvs import z_SetLEDColor, z_SetColor, z_ConfigureRobot, z_FullSystemControl

GRN  = "\033[32m"; RED  = "\033[31m"; YLW = "\033[33m"; CYN = "\033[36m"; BOLD = "\033[1m"; RST = "\033[0m"

passed_topics = set()
services_tested = set()

def make_cb(topic_name):
    def cb(msg):
        if topic_name not in passed_topics:
            passed_topics.add(topic_name)
            print(f"  {GRN}✓ PASS{RST} [{len(passed_topics):2d}/34] {topic_name}")
    return cb

class MasterPythonValidator(ZenohNode):
    def __init__(self):
        super().__init__("master_python_validator")
        print(f"\n{CYN}{BOLD}╔══════════════════════════════════════════════════════╗")
        print(f"║  ALL-IN-ONE MASTER INTERFACE VALIDATOR (Python)      ║")
        print(f"║  Testing 34 Topics & 4 Custom Services Live          ║")
        print(f"╚══════════════════════════════════════════════════════╝{RST}\n")

        # Standard subscriptions
        std_topics = [
            (z_Bool, "test/bool"), (z_Int8, "test/int8"), (z_UInt8, "test/uint8"),
            (z_Int16, "test/int16"), (z_UInt16, "test/uint16"), (z_Int32, "test/int32"),
            (z_UInt32, "test/uint32"), (z_Int64, "test/int64"), (z_UInt64, "test/uint64"),
            (z_Float32, "test/float32"), (z_Float64, "test/float64"), (z_String, "test/string"),
            (z_Header, "test/header"), (z_Int32MultiArray, "test/int32_array"),
            (z_Float64MultiArray, "test/float64_array"), (z_Imu, "test/imu"),
            (z_Temperature, "test/temperature"), (z_Range, "test/range"),
            (z_RelativeHumidity, "test/humidity"), (z_BatteryState, "test/battery"),
            (z_NavSatFix, "test/navsatfix"), (z_JoyFeedback, "test/joyfeedback"),
            (z_JointState, "test/jointstate"), (z_Vector3, "test/vector3"),
            (z_Quaternion, "test/quaternion"), (z_Pose, "test/pose"), (z_Twist, "test/twist")
        ]

        # Custom subscriptions
        custom_topics = [
            (z_SetLED, "test_custom/set_led"), (z_MPU6050Data, "test_custom/mpu6050"),
            (z_RobotState, "test_custom/robot_state"), (z_MotorStatus, "test_custom/motor_status"),
            (z_SensorTelemetry, "test_custom/telemetry"), (z_RobotDiagnostic, "test_custom/robot_diag"),
            (z_ComplexRobotState, "test_custom/complex_state")
        ]

        for cls, top in std_topics + custom_topics:
            self.z_create_subscription(cls, top, make_cb(top))

        # Service clients
        self.cli_set_led_clr = self.z_create_client(z_SetLEDColor, "srv_custom/set_led_color")
        self.cli_set_clr = self.z_create_client(z_SetColor, "srv_custom/set_color")
        self.cli_cfg_robot = self.z_create_client(z_ConfigureRobot, "srv_custom/configure_robot")
        self.cli_full_ctrl = self.z_create_client(z_FullSystemControl, "srv_custom/full_system_control")

def main():
    cfg = ZenohConfig(host="10.42.0.50", port=7447)
    if not ZenohNode.init(cfg):
        return
    node = MasterPythonValidator()

    t = threading.Thread(target=node.z_spin, daemon=True)
    t.start()

    start = time.time()
    srv_tested = False

    while time.time() - start < 20:
        time.sleep(0.2)

        if len(passed_topics) >= 30 and not srv_tested:
            srv_tested = True
            print(f"\n{YLW}Testing 4 Custom Services live...{RST}")

            # 1. SetLEDColor
            res1 = node.cli_set_led_clr.call(z_SetLEDColor.Request(led_data=z_SetLED(r=255, g=0, b=0, brightness=100, led_num=1)), timeout_sec=2.0)
            if res1 and res1.success:
                services_tested.add("srv_custom/set_led_color")
                print(f"  {GRN}✓ PASS{RST} Service 'srv_custom/set_led_color'")

            # 2. SetColor
            res2 = node.cli_set_clr.call(z_SetColor.Request(r=255, g=128, b=0), timeout_sec=2.0)
            if res2 and res2.success:
                services_tested.add("srv_custom/set_color")
                print(f"  {GRN}✓ PASS{RST} Service 'srv_custom/set_color'")

            # 3. ConfigureRobot
            req3 = z_ConfigureRobot.Request(target_status=z_MotorStatus(motor_id=1, speed=100.0, is_active=True), mode=1)
            res3 = node.cli_cfg_robot.call(req3, timeout_sec=2.0)
            if res3 and res3.success:
                services_tested.add("srv_custom/configure_robot")
                print(f"  {GRN}✓ PASS{RST} Service 'srv_custom/configure_robot'")

            # 4. FullSystemControl
            req4 = z_FullSystemControl.Request(target_state=z_ComplexRobotState(robot_mode="TEST", cycle_count=1), command_code=5)
            res4 = node.cli_full_ctrl.call(req4, timeout_sec=2.0)
            if res4 and res4.ack:
                services_tested.add("srv_custom/full_system_control")
                print(f"  {GRN}✓ PASS{RST} Service 'srv_custom/full_system_control'")

        if len(passed_topics) == 34 and len(services_tested) == 4:
            break

    print(f"\n{BOLD}{'═'*54}")
    print(f"  RESULTS: {len(passed_topics)}/34 Topics Passed | {len(services_tested)}/4 Services Passed")
    if len(passed_topics) == 34 and len(services_tested) == 4:
        print(f"  {GRN}{BOLD}✓ ALL PRE-DEFINED & CUSTOM INTERFACES VERIFIED 100%!{RST}")
    else:
        print(f"  {RED}{BOLD}⚠  PARTIAL FAILURE (Topics: {len(passed_topics)}/34, Services: {len(services_tested)}/4){RST}")
    ZenohNode.shutdown()

if __name__ == "__main__":
    main()
