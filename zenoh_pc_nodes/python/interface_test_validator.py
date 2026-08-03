#!/usr/bin/env python3
"""
interface_test_validator.py
============================
Subscribes to all 27 test/* topics published by the MCU interface_test_publisher
firmware and validates each received value against expected sentinel values.

Expected sentinel values MUST match those in:
  mcu_firmware_examples/interface_test_publisher/src/main.cpp

Run:
    python3 interface_test_validator.py [host_ip]   (default: 10.42.0.50)
"""

import sys, os, time, threading
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.std_msgs import (
    z_Bool, z_Int8, z_UInt8, z_Int16, z_UInt16,
    z_Int32, z_UInt32, z_Int64, z_UInt64,
    z_Float32, z_Float64, z_String, z_Header,
    z_Int32MultiArray, z_Float64MultiArray
)
from zenoh_ros.sensor_msgs import (
    z_Imu, z_Temperature, z_Range, z_RelativeHumidity,
    z_BatteryState, z_NavSatFix, z_JoyFeedback, z_JointState
)
from zenoh_ros.geometry_msgs import z_Vector3, z_Quaternion, z_Pose, z_Twist

# ── Colours ──────────────────────────────────────────────────────────────────
GRN  = "\033[32m"; RED = "\033[31m"; YLW = "\033[33m"
CYN  = "\033[36m"; BOLD = "\033[1m"; RST = "\033[0m"

# ── Sentinel values (must EXACTLY match MCU firmware) ─────────────────────────
T_BOOL      = True
T_INT8      = -42
T_UINT8     = 200
T_INT16     = -1234
T_UINT16    = 54321
T_INT32     = -987654
T_UINT32    = 3141592
T_INT64     = -9876543210
T_UINT64    = 18446744073709551000
T_FLOAT32   = 3.14159
T_FLOAT64   = 2.718281828
T_STRING    = "zenoh_ros_test_OK"
T_HDR_SEC   = 1234567
T_HDR_NSEC  = 987654321
T_HDR_FRAME = "test_frame"

TOTAL_TOPICS = 27
results = {}
done_event = threading.Event()
lock = threading.Lock()

def _pass(topic, detail=""):
    with lock:
        if topic not in results:
            results[topic] = ("PASS", detail)
            n = len(results)
            print(f"  {GRN}✓ PASS{RST}  [{n:2d}/{TOTAL_TOPICS}] {BOLD}{topic}{RST}  {detail}")
            if n >= TOTAL_TOPICS:
                done_event.set()

def _fail(topic, detail=""):
    with lock:
        if topic not in results:
            results[topic] = ("FAIL", detail)
            n = len(results)
            print(f"  {RED}✗ FAIL{RST}  [{n:2d}/{TOTAL_TOPICS}] {BOLD}{topic}{RST}  {detail}")
            if n >= TOTAL_TOPICS:
                done_event.set()

def _approx(a, b, tol=1e-3):
    if b == 0: return abs(a) < tol
    return abs(a - b) / abs(b) < tol

def _check_header(hdr, topic):
    ok = True
    if hdr.stamp.sec != T_HDR_SEC:
        print(f"    {YLW}⚠ {topic} header.stamp.sec: got {hdr.stamp.sec} expected {T_HDR_SEC}{RST}"); ok = False
    if hdr.stamp.nanosec != T_HDR_NSEC:
        print(f"    {YLW}⚠ {topic} header.stamp.nanosec: got {hdr.stamp.nanosec} expected {T_HDR_NSEC}{RST}"); ok = False
    if hdr.frame_id != T_HDR_FRAME:
        print(f"    {YLW}⚠ {topic} header.frame_id: got '{hdr.frame_id}' expected '{T_HDR_FRAME}'{RST}"); ok = False
    return ok

# ── Subscriber callbacks ──────────────────────────────────────────────────────
def cb_bool(msg):
    (_pass if msg.data == T_BOOL else _fail)("test/bool", f"data={msg.data}")

def cb_int8(msg):
    (_pass if msg.data == T_INT8 else _fail)("test/int8", f"data={msg.data} (expected {T_INT8})")

def cb_uint8(msg):
    (_pass if msg.data == T_UINT8 else _fail)("test/uint8", f"data={msg.data}")

def cb_int16(msg):
    (_pass if msg.data == T_INT16 else _fail)("test/int16", f"data={msg.data}")

def cb_uint16(msg):
    (_pass if msg.data == T_UINT16 else _fail)("test/uint16", f"data={msg.data}")

def cb_int32(msg):
    (_pass if msg.data == T_INT32 else _fail)("test/int32", f"data={msg.data}")

def cb_uint32(msg):
    (_pass if msg.data == T_UINT32 else _fail)("test/uint32", f"data={msg.data}")

def cb_int64(msg):
    (_pass if msg.data == T_INT64 else _fail)("test/int64", f"data={msg.data}")

def cb_uint64(msg):
    # ArduinoJson uint64 can have ±1000 diff at extremes
    ok = abs(int(msg.data) - T_UINT64) < 1000
    (_pass if ok else _fail)("test/uint64", f"data={msg.data}")

def cb_float32(msg):
    ok = _approx(msg.data, T_FLOAT32, tol=2e-4)
    (_pass if ok else _fail)("test/float32", f"data={msg.data:.5f} (expected≈{T_FLOAT32})")

def cb_float64(msg):
    ok = _approx(msg.data, T_FLOAT64, tol=1e-6)
    (_pass if ok else _fail)("test/float64", f"data={msg.data:.9f}")

def cb_string(msg):
    (_pass if msg.data == T_STRING else _fail)("test/string", f"data='{msg.data}'")

def cb_header(msg):
    ok = _check_header(msg, "test/header")
    (_pass if ok else _fail)("test/header", f"sec={msg.stamp.sec} frame='{msg.frame_id}'")

def cb_int32_array(msg):
    expected = [-100, 0, 100, 200, -32768]
    ok = list(msg.data) == expected
    (_pass if ok else _fail)("test/int32_array", f"data={list(msg.data)}")

def cb_float64_array(msg):
    expected = [1.1, 2.2, 3.3, -4.4, 5.5]
    ok = len(msg.data) == 5 and all(_approx(a, b, 1e-4) for a, b in zip(msg.data, expected))
    (_pass if ok else _fail)("test/float64_array", f"data={[round(v,2) for v in msg.data]}")

def cb_imu(msg):
    ok = _check_header(msg.header, "test/imu")
    ok &= _approx(msg.orientation.x, 0.1) and _approx(msg.orientation.w, 0.9165, tol=2e-3)
    ok &= _approx(msg.linear_acceleration.z, 9.81, tol=1e-3)
    ok &= _approx(msg.angular_velocity.x, 0.01, tol=1e-4)
    (_pass if ok else _fail)("test/imu", f"ori_x={msg.orientation.x:.3f} accel_z={msg.linear_acceleration.z:.2f}")

def cb_temperature(msg):
    ok = _check_header(msg.header, "test/temperature")
    ok &= _approx(msg.temperature, 36.6) and _approx(msg.variance, 0.1)
    (_pass if ok else _fail)("test/temperature", f"temp={msg.temperature:.1f} var={msg.variance:.2f}")

def cb_range(msg):
    ok = _check_header(msg.header, "test/range")
    ok &= _approx(msg.range, 1.234, tol=1e-3) and _approx(msg.max_range, 4.0)
    ok &= msg.radiation_type == 0
    (_pass if ok else _fail)("test/range", f"range={msg.range:.3f} max={msg.max_range}")

def cb_humidity(msg):
    ok = _check_header(msg.header, "test/humidity")
    ok &= _approx(msg.relative_humidity, 0.65) and _approx(msg.variance, 0.01)
    (_pass if ok else _fail)("test/humidity", f"rh={msg.relative_humidity:.2f}")

def cb_battery(msg):
    ok = _check_header(msg.header, "test/battery")
    ok &= _approx(msg.voltage, 12.4, tol=1e-3) and _approx(msg.percentage, 0.85, tol=1e-3)
    ok &= msg.power_supply_status == 2 and msg.present == True
    (_pass if ok else _fail)("test/battery", f"voltage={msg.voltage:.1f}V {msg.percentage*100:.0f}% status={msg.power_supply_status}")

def cb_navsatfix(msg):
    ok = _check_header(msg.header, "test/navsatfix")
    ok &= _approx(msg.latitude, 24.8607, 1e-4) and _approx(msg.longitude, 67.0011, 1e-4)
    ok &= _approx(msg.altitude, 8.0) and msg.status.status == 0 and msg.status.service == 1
    (_pass if ok else _fail)("test/navsatfix", f"lat={msg.latitude:.4f} lon={msg.longitude:.4f} alt={msg.altitude}")

def cb_joyfeedback(msg):
    ok = msg.type == 0 and msg.id == 3 and _approx(msg.intensity, 0.75, 1e-3)
    (_pass if ok else _fail)("test/joyfeedback", f"id={msg.id} intensity={msg.intensity:.2f}")

def cb_jointstate(msg):
    ok = _check_header(msg.header, "test/jointstate")
    ok &= list(msg.name) == ["joint1","joint2","joint3"]
    ok &= all(_approx(a,b) for a,b in zip(msg.position,[0.1,0.2,0.3]))
    ok &= all(_approx(a,b) for a,b in zip(msg.velocity,[1.0,2.0,3.0]))
    ok &= all(_approx(a,b) for a,b in zip(msg.effort,[10.0,20.0,30.0]))
    (_pass if ok else _fail)("test/jointstate", f"joints={list(msg.name)} pos={[round(v,2) for v in msg.position]}")

def cb_vector3(msg):
    ok = _approx(msg.x,1.0) and _approx(msg.y,2.0) and _approx(msg.z,3.0)
    (_pass if ok else _fail)("test/vector3", f"[{msg.x},{msg.y},{msg.z}]")

def cb_quaternion(msg):
    ok = _approx(msg.x,0.0) and _approx(msg.y,0.0) and _approx(msg.z,0.707,1e-3) and _approx(msg.w,0.707,1e-3)
    (_pass if ok else _fail)("test/quaternion", f"z={msg.z:.3f} w={msg.w:.3f}")

def cb_pose(msg):
    ok = _approx(msg.position.x,1.0) and _approx(msg.position.y,2.0) and _approx(msg.position.z,3.0)
    ok &= _approx(msg.orientation.w,1.0) and _approx(msg.orientation.z,0.0)
    (_pass if ok else _fail)("test/pose", f"pos=[{msg.position.x},{msg.position.y},{msg.position.z}]")

def cb_twist(msg):
    ok = _approx(msg.linear.x,1.5) and _approx(msg.linear.y,0.0) and _approx(msg.linear.z,0.0)
    ok &= _approx(msg.angular.z,0.5) and _approx(msg.angular.x,0.0)
    (_pass if ok else _fail)("test/twist", f"linear.x={msg.linear.x} angular.z={msg.angular.z}")


class InterfaceTestValidator(ZenohNode):
    def __init__(self):
        super().__init__("interface_test_validator")
        print(f"\n{CYN}{BOLD}╔══════════════════════════════════════════════════╗")
        print(f"║   zenoh_ros Interface Test Validator             ║")
        print(f"║   Waiting for MCU — 27 message types, ~2s each  ║")
        print(f"╚══════════════════════════════════════════════════╝{RST}\n")
        # std_msgs
        self.z_create_subscription(z_Bool,              "test/bool",         cb_bool,         10)
        self.z_create_subscription(z_Int8,              "test/int8",         cb_int8,         10)
        self.z_create_subscription(z_UInt8,             "test/uint8",        cb_uint8,        10)
        self.z_create_subscription(z_Int16,             "test/int16",        cb_int16,        10)
        self.z_create_subscription(z_UInt16,            "test/uint16",       cb_uint16,       10)
        self.z_create_subscription(z_Int32,             "test/int32",        cb_int32,        10)
        self.z_create_subscription(z_UInt32,            "test/uint32",       cb_uint32,       10)
        self.z_create_subscription(z_Int64,             "test/int64",        cb_int64,        10)
        self.z_create_subscription(z_UInt64,            "test/uint64",       cb_uint64,       10)
        self.z_create_subscription(z_Float32,           "test/float32",      cb_float32,      10)
        self.z_create_subscription(z_Float64,           "test/float64",      cb_float64,      10)
        self.z_create_subscription(z_String,            "test/string",       cb_string,       10)
        self.z_create_subscription(z_Header,            "test/header",       cb_header,       10)
        self.z_create_subscription(z_Int32MultiArray,   "test/int32_array",  cb_int32_array,  10)
        self.z_create_subscription(z_Float64MultiArray, "test/float64_array",cb_float64_array,10)
        # sensor_msgs
        self.z_create_subscription(z_Imu,              "test/imu",          cb_imu,          10)
        self.z_create_subscription(z_Temperature,       "test/temperature",  cb_temperature,  10)
        self.z_create_subscription(z_Range,             "test/range",        cb_range,        10)
        self.z_create_subscription(z_RelativeHumidity,  "test/humidity",     cb_humidity,     10)
        self.z_create_subscription(z_BatteryState,      "test/battery",      cb_battery,      10)
        self.z_create_subscription(z_NavSatFix,         "test/navsatfix",    cb_navsatfix,    10)
        self.z_create_subscription(z_JoyFeedback,       "test/joyfeedback",  cb_joyfeedback,  10)
        self.z_create_subscription(z_JointState,        "test/jointstate",   cb_jointstate,   10)
        # geometry_msgs
        self.z_create_subscription(z_Vector3,           "test/vector3",      cb_vector3,      10)
        self.z_create_subscription(z_Quaternion,        "test/quaternion",   cb_quaternion,   10)
        self.z_create_subscription(z_Pose,              "test/pose",         cb_pose,         10)
        self.z_create_subscription(z_Twist,             "test/twist",        cb_twist,        10)
        print(f"{CYN}All 27 subscriptions declared. Listening for MCU...{RST}\n")

    def print_summary(self):
        total   = len(results)
        passed  = sum(1 for v in results.values() if v[0]=="PASS")
        failed  = total - passed
        missing = TOTAL_TOPICS - total
        all_topics = {
            "test/bool","test/int8","test/uint8","test/int16","test/uint16",
            "test/int32","test/uint32","test/int64","test/uint64",
            "test/float32","test/float64","test/string","test/header",
            "test/int32_array","test/float64_array",
            "test/imu","test/temperature","test/range","test/humidity",
            "test/battery","test/navsatfix","test/joyfeedback","test/jointstate",
            "test/vector3","test/quaternion","test/pose","test/twist"
        }
        print(f"\n{BOLD}{'═'*54}")
        print(f"  INTERFACE TEST RESULTS  —  {total}/{TOTAL_TOPICS} topics tested")
        print(f"{'═'*54}{RST}")
        print(f"  {GRN}{BOLD}PASSED : {passed}{RST}")
        if failed:  print(f"  {RED}{BOLD}FAILED : {failed}{RST}")
        if missing:
            print(f"  {YLW}{BOLD}NO MESSAGE RECEIVED : {missing}{RST}")
            for t in sorted(all_topics - results.keys()):
                print(f"    {YLW}· {t}{RST}")
        if failed:
            print(f"\n  {RED}{BOLD}Failed topics:{RST}")
            for t, (s, d) in sorted(results.items()):
                if s == "FAIL":
                    print(f"    {RED}· {t}: {d}{RST}")
        print(f"{BOLD}{'═'*54}{RST}")
        if failed == 0 and missing == 0:
            print(f"\n  {GRN}{BOLD}✓ ALL {TOTAL_TOPICS} INTERFACES VERIFIED SUCCESSFULLY!{RST}\n")
        else:
            print(f"\n  {RED}{BOLD}⚠  ISSUES DETECTED — review above{RST}\n")


def main():
    host = sys.argv[1] if len(sys.argv) > 1 else "10.42.0.50"
    config = ZenohConfig(host=host, port=7447)
    ZenohNode.init(config)
    node = None
    try:
        node = InterfaceTestValidator()

        timeout = 90  # 27 topics × 2s each + WiFi connection buffer
        print(f"{YLW}Timeout: {timeout}s.  The MCU publishes one type every 2s.{RST}\n")

        # Run z_spin in a daemon thread; main thread waits for done_event
        t = threading.Thread(target=node.z_spin, daemon=True)
        t.start()

        done_event.wait(timeout=timeout)

        if len(results) < TOTAL_TOPICS:
            print(f"\n{YLW}Timed out after {timeout}s — {len(results)}/{TOTAL_TOPICS} received{RST}")

        node.print_summary()

    except KeyboardInterrupt:
        print(f"\n{YLW}[CTRL+C] Interrupted — printing partial results...{RST}")
        if node:
            node.print_summary()
    finally:
        if node:
            try: node.z_destroy()
            except: pass

if __name__ == "__main__":
    main()
