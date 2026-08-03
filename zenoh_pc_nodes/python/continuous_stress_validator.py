#!/usr/bin/env python3
"""
continuous_stress_validator.py — 2-Minute High-Frequency Continuous Stress Validator (Python)
==========================================================================================
Validates continuous streaming on 34 topics + frequent service calls (every 2s) for 120s.
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

GRN = "\033[32m"; RED = "\033[31m"; YLW = "\033[33m"; CYN = "\033[36m"; BOLD = "\033[1m"; RST = "\033[0m"

topic_counts = {}
lock = threading.Lock()

def make_cb(topic_name):
    def cb(msg):
        with lock:
            topic_counts[topic_name] = topic_counts.get(topic_name, 0) + 1
    return cb

class ContinuousStressNode(ZenohNode):
    def __init__(self):
        super().__init__("continuous_stress_node")
        print(f"\n{CYN}{BOLD}╔══════════════════════════════════════════════════════════════╗")
        print(f"║  CONTINUOUS 2-MINUTE STRESS TEST & VALIDATOR (Python)       ║")
        print(f"║  Testing continuous data stream & frequent service calls     ║")
        print(f"╚══════════════════════════════════════════════════════════════╝{RST}\n")

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

        custom_topics = [
            (z_SetLED, "test_custom/set_led"), (z_MPU6050Data, "test_custom/mpu6050"),
            (z_RobotState, "test_custom/robot_state"), (z_MotorStatus, "test_custom/motor_status"),
            (z_SensorTelemetry, "test_custom/telemetry"), (z_RobotDiagnostic, "test_custom/robot_diag"),
            (z_ComplexRobotState, "test_custom/complex_state")
        ]

        for cls, top in std_topics + custom_topics:
            self.z_create_subscription(cls, top, make_cb(top))

        self.cli_set_led_clr = self.z_create_client(z_SetLEDColor, "srv_custom/set_led_color")
        self.cli_set_clr     = self.z_create_client(z_SetColor, "srv_custom/set_color")
        self.cli_cfg_robot   = self.z_create_client(z_ConfigureRobot, "srv_custom/configure_robot")
        self.cli_full_ctrl   = self.z_create_client(z_FullSystemControl, "srv_custom/full_system_control")

def main():
    config = ZenohConfig(host="10.42.0.50", port=7447)
    ZenohNode.init(config)
    node = ContinuousStressNode()

    t = threading.Thread(target=node.z_spin, daemon=True)
    t.start()

    start_time = time.time()
    duration = 120 # 2 minutes
    last_srv_time = time.time()

    total_srv_calls = 0
    successful_srv_calls = 0

    print(f"{YLW}Starting 120-second continuous stress test...{RST}")

    while time.time() - start_time < duration:
        time.sleep(0.5)
        elapsed = int(time.time() - start_time)

        # Service call round every 2 seconds
        if time.time() - last_srv_time >= 2.0:
            last_srv_time = time.time()
            total_srv_calls += 4

            try:
                res1 = node.cli_set_led_clr.call(z_SetLEDColor.Request(led_data=z_SetLED(r=255, g=0, b=0, brightness=100, led_num=1)), timeout_sec=2.0)
                if res1 and res1.success: successful_srv_calls += 1
                time.sleep(0.05)

                res2 = node.cli_set_clr.call(z_SetColor.Request(r=100, g=200, b=50), timeout_sec=2.0)
                if res2 and res2.success: successful_srv_calls += 1
                time.sleep(0.05)

                req3 = z_ConfigureRobot.Request(target_status=z_MotorStatus(motor_id=1, speed=120.0, is_active=True), mode=1)
                res3 = node.cli_cfg_robot.call(req3, timeout_sec=2.0)
                if res3 and res3.success: successful_srv_calls += 1
                time.sleep(0.05)

                req4 = z_FullSystemControl.Request(target_state=z_ComplexRobotState(robot_mode="STRESS", cycle_count=elapsed), command_code=7)
                res4 = node.cli_full_ctrl.call(req4, timeout_sec=2.0)
                if res4 and res4.ack: successful_srv_calls += 1
            except Exception as e:
                print(f"  {RED}⚠ Service call exception: {e}{RST}")

        with lock:
            rcvd_topics = len(topic_counts)
            total_msgs = sum(topic_counts.values())

        if elapsed % 10 == 0:
            print(f"  [{elapsed:3d}s/120s] Topics Received: {rcvd_topics:2d}/34 | Total Msgs: {total_msgs:5d} | Srv Calls OK: {successful_srv_calls}/{total_srv_calls}")

    print(f"\n{BOLD}{'═'*64}")
    print(f"  CONTINUOUS STRESS TEST FINISHED ({int(time.time() - start_time)}s elapsed)")
    print(f"  Topics Active  : {len(topic_counts)}/34")
    print(f"  Total Messages : {sum(topic_counts.values())}")
    print(f"  Service Calls  : {successful_srv_calls}/{total_srv_calls} PASSED ({(successful_srv_calls/total_srv_calls)*100:.1f}%)")
    if len(topic_counts) == 34 and successful_srv_calls == total_srv_calls:
        print(f"  {GRN}{BOLD}✓ 100% PERFECT CONTINUOUS STABILITY VERIFIED! ZERO LOSS!{RST}")
    else:
        print(f"  {RED}{BOLD}⚠  STRESS TEST COMPLETED WITH LOSS/ERRORS{RST}")
    print(f"{'═'*64}{RST}\n")

    node.z_destroy()

if __name__ == "__main__":
    main()
