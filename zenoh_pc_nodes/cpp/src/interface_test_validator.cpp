/**
 * interface_test_validator.cpp — PC C++ Test Validator
 * ======================================================
 * Subscribes to all 27 test/* topics from the MCU using standard per-message headers:
 *   #include <zenoh_ros/std_msgs/z_Int8.h>
 *   #include <zenoh_ros/sensor_msgs/z_JointState.h>
 *   etc.
 */

#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <chrono>
#include <thread>
#include <atomic>

#include <zenoh_ros/ZenohRosPC.h>

// Direct per-type PC C++ headers
#include <zenoh_ros/std_msgs/z_Bool.h>
#include <zenoh_ros/std_msgs/z_Int8.h>
#include <zenoh_ros/std_msgs/z_UInt8.h>
#include <zenoh_ros/std_msgs/z_Int16.h>
#include <zenoh_ros/std_msgs/z_UInt16.h>
#include <zenoh_ros/std_msgs/z_Int32.h>
#include <zenoh_ros/std_msgs/z_UInt32.h>
#include <zenoh_ros/std_msgs/z_Int64.h>
#include <zenoh_ros/std_msgs/z_UInt64.h>
#include <zenoh_ros/std_msgs/z_Float32.h>
#include <zenoh_ros/std_msgs/z_Float64.h>
#include <zenoh_ros/std_msgs/z_String.h>
#include <zenoh_ros/std_msgs/z_Header.h>
#include <zenoh_ros/std_msgs/z_Int32MultiArray.h>
#include <zenoh_ros/std_msgs/z_Float64MultiArray.h>

// sensor_msgs direct headers
#include <zenoh_ros/sensor_msgs/z_Imu.h>
#include <zenoh_ros/sensor_msgs/z_Temperature.h>
#include <zenoh_ros/sensor_msgs/z_Range.h>
#include <zenoh_ros/sensor_msgs/z_RelativeHumidity.h>
#include <zenoh_ros/sensor_msgs/z_BatteryState.h>
#include <zenoh_ros/sensor_msgs/z_NavSatFix.h>
#include <zenoh_ros/sensor_msgs/z_JoyFeedback.h>
#include <zenoh_ros/sensor_msgs/z_JointState.h>

// geometry_msgs direct headers
#include <zenoh_ros/geometry_msgs/z_Vector3.h>
#include <zenoh_ros/geometry_msgs/z_Quaternion.h>
#include <zenoh_ros/geometry_msgs/z_Pose.h>
#include <zenoh_ros/geometry_msgs/z_Twist.h>

static const bool     T_BOOL      = true;
static const int8_t   T_INT8      = -42;
static const uint8_t  T_UINT8     = 200;
static const int16_t  T_INT16     = -1234;
static const uint16_t T_UINT16    = 54321;
static const int32_t  T_INT32     = -987654;
static const uint32_t T_UINT32    = 3141592UL;
static const int64_t  T_INT64     = -9876543210LL;
static const float    T_FLOAT32   = 3.14159f;
static const double   T_FLOAT64   = 2.718281828;
static const std::string T_STRING = "zenoh_ros_test_OK";

static const uint32_t T_HDR_SEC   = 1234567UL;
static const uint32_t T_HDR_NSEC  = 987654321UL;
static const std::string T_HDR_FRAME = "test_frame";

std::map<std::string, bool> pass_results;
std::atomic<int> pass_count{0};

void record_pass(const std::string& topic, bool ok, const std::string& info) {
    if (pass_results.find(topic) == pass_results.end()) {
        pass_results[topic] = ok;
        pass_count++;
        if (ok) {
            std::cout << "  \033[32m✓ PASS\033[0m [" << pass_count << "/27] " << topic << "  (" << info << ")\n";
        } else {
            std::cout << "  \033[31m✗ FAIL\033[0m [" << pass_count << "/27] " << topic << "  (" << info << ")\n";
        }
    }
}

class CppValidatorNode : public ZenohNode {
public:
    CppValidatorNode() : ZenohNode("cpp_interface_validator") {
        z_create_subscription<z_Bool>("test/bool", [](const z_Bool& msg) {
            record_pass("test/bool", msg.data == T_BOOL, msg.data ? "true" : "false");
        });

        z_create_subscription<z_Int8>("test/int8", [](const z_Int8& msg) {
            record_pass("test/int8", msg.data == T_INT8, std::to_string(msg.data));
        });

        z_create_subscription<z_UInt8>("test/uint8", [](const z_UInt8& msg) {
            record_pass("test/uint8", msg.data == T_UINT8, std::to_string(msg.data));
        });

        z_create_subscription<z_Int16>("test/int16", [](const z_Int16& msg) {
            record_pass("test/int16", msg.data == T_INT16, std::to_string(msg.data));
        });

        z_create_subscription<z_UInt16>("test/uint16", [](const z_UInt16& msg) {
            record_pass("test/uint16", msg.data == T_UINT16, std::to_string(msg.data));
        });

        z_create_subscription<z_Int32>("test/int32", [](const z_Int32& msg) {
            record_pass("test/int32", msg.data == T_INT32, std::to_string(msg.data));
        });

        z_create_subscription<z_UInt32>("test/uint32", [](const z_UInt32& msg) {
            record_pass("test/uint32", msg.data == T_UINT32, std::to_string(msg.data));
        });

        z_create_subscription<z_Int64>("test/int64", [](const z_Int64& msg) {
            record_pass("test/int64", msg.data == T_INT64, std::to_string(msg.data));
        });

        z_create_subscription<z_UInt64>("test/uint64", [](const z_UInt64& msg) {
            record_pass("test/uint64", true, std::to_string(msg.data));
        });

        z_create_subscription<z_Float32>("test/float32", [](const z_Float32& msg) {
            bool ok = std::abs(msg.data - T_FLOAT32) < 1e-4;
            record_pass("test/float32", ok, std::to_string(msg.data));
        });

        z_create_subscription<z_Float64>("test/float64", [](const z_Float64& msg) {
            bool ok = std::abs(msg.data - T_FLOAT64) < 1e-6;
            record_pass("test/float64", ok, std::to_string(msg.data));
        });

        z_create_subscription<z_String>("test/string", [](const z_String& msg) {
            record_pass("test/string", msg.data == T_STRING, msg.data);
        });

        z_create_subscription<z_Header>("test/header", [](const z_Header& msg) {
            bool ok = (msg.stamp.sec == T_HDR_SEC && msg.stamp.nanosec == T_HDR_NSEC && msg.frame_id == T_HDR_FRAME);
            record_pass("test/header", ok, "sec=" + std::to_string(msg.stamp.sec) + " frame=" + msg.frame_id);
        });

        z_create_subscription<z_Int32MultiArray>("test/int32_array", [](const z_Int32MultiArray& msg) {
            record_pass("test/int32_array", msg.data.size() == 5, "size=" + std::to_string(msg.data.size()));
        });

        z_create_subscription<z_Float64MultiArray>("test/float64_array", [](const z_Float64MultiArray& msg) {
            record_pass("test/float64_array", msg.data.size() == 5, "size=" + std::to_string(msg.data.size()));
        });

        z_create_subscription<z_Imu>("test/imu", [](const z_Imu& msg) {
            bool ok = std::abs(msg.linear_acceleration.z - 9.81) < 1e-2;
            record_pass("test/imu", ok, "accel_z=" + std::to_string(msg.linear_acceleration.z));
        });

        z_create_subscription<z_Temperature>("test/temperature", [](const z_Temperature& msg) {
            bool ok = std::abs(msg.temperature - 36.6) < 1e-2;
            record_pass("test/temperature", ok, "temp=" + std::to_string(msg.temperature));
        });

        z_create_subscription<z_Range>("test/range", [](const z_Range& msg) {
            bool ok = std::abs(msg.range - 1.234f) < 1e-2;
            record_pass("test/range", ok, "range=" + std::to_string(msg.range));
        });

        z_create_subscription<z_RelativeHumidity>("test/humidity", [](const z_RelativeHumidity& msg) {
            bool ok = std::abs(msg.relative_humidity - 0.65) < 1e-2;
            record_pass("test/humidity", ok, "rh=" + std::to_string(msg.relative_humidity));
        });

        z_create_subscription<z_BatteryState>("test/battery", [](const z_BatteryState& msg) {
            bool ok = std::abs(msg.voltage - 12.4f) < 1e-2;
            record_pass("test/battery", ok, "v=" + std::to_string(msg.voltage));
        });

        z_create_subscription<z_NavSatFix>("test/navsatfix", [](const z_NavSatFix& msg) {
            bool ok = std::abs(msg.latitude - 24.8607) < 1e-3;
            record_pass("test/navsatfix", ok, "lat=" + std::to_string(msg.latitude));
        });

        z_create_subscription<z_JoyFeedback>("test/joyfeedback", [](const z_JoyFeedback& msg) {
            bool ok = msg.id == 3 && std::abs(msg.intensity - 0.75f) < 1e-2;
            record_pass("test/joyfeedback", ok, "id=" + std::to_string(msg.id));
        });

        z_create_subscription<z_JointState>("test/jointstate", [](const z_JointState& msg) {
            bool ok = msg.name.size() == 3 && msg.position.size() == 3;
            record_pass("test/jointstate", ok, "joints=" + std::to_string(msg.name.size()));
        });

        z_create_subscription<z_Vector3>("test/vector3", [](const z_Vector3& msg) {
            bool ok = msg.x == 1.0 && msg.y == 2.0 && msg.z == 3.0;
            record_pass("test/vector3", ok, "x=" + std::to_string(msg.x));
        });

        z_create_subscription<z_Quaternion>("test/quaternion", [](const z_Quaternion& msg) {
            bool ok = std::abs(msg.z - 0.707) < 1e-2;
            record_pass("test/quaternion", ok, "z=" + std::to_string(msg.z));
        });

        z_create_subscription<z_Pose>("test/pose", [](const z_Pose& msg) {
            bool ok = msg.position.x == 1.0 && msg.orientation.w == 1.0;
            record_pass("test/pose", ok, "pos_x=" + std::to_string(msg.position.x));
        });

        z_create_subscription<z_Twist>("test/twist", [](const z_Twist& msg) {
            bool ok = msg.linear.x == 1.5 && msg.angular.z == 0.5;
            record_pass("test/twist", ok, "lin_x=" + std::to_string(msg.linear.x));
        });
    }
};

int main(int argc, char** argv) {
    std::cout << "\n==========================================\n";
    std::cout << "  zenoh_ros PC C++ Interface Test Validator\n";
    std::cout << "==========================================\n\n";

    ZenohConfig cfg;
    cfg.host = (argc > 1) ? argv[1] : "10.42.0.50";
    cfg.port = 7447;

    if (!ZenohNode::init(cfg)) {
        std::cerr << "Zenoh init failed!\n";
        return 1;
    }

    CppValidatorNode node;

    std::thread spin_thread([&]() {
        node.z_spin();
    });

    auto start = std::chrono::steady_clock::now();
    while (pass_count < 27) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 60) {
            std::cout << "\nTimed out after 60s!\n";
            break;
        }
    }

    std::cout << "\n==========================================\n";
    std::cout << "  PC C++ RESULTS: " << pass_count.load() << "/27 passed\n";
    std::cout << "==========================================\n\n";

    spin_thread.detach();
    return 0;
}
