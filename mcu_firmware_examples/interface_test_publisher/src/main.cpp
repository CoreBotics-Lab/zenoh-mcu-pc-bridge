/**
 * interface_test_publisher — MCU Test Firmware
 * =============================================
 * Testing ALL 27 message types in zenoh_ros:
 *   #include <zenoh_ros/std_msgs/z_Int8.h>
 *   #include <zenoh_ros/geometry_msgs/z_Pose.h>
 *   etc.
 */

#include <Arduino.h>
#include <zenoh_ros/ZenohRos.h>

// Direct per-type MCU headers
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

#include "credential.h"

ZenohConfig cfg = {
    .ssid      = WIFI_SSID,
    .password  = WIFI_PASSWORD,
    .port      = 7447,
    .local_ip  = "10.42.0.50",
    .wifi_mode = WIFI_STA
};

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
static const char*    T_STRING    = "zenoh_ros_test_OK";
static const uint32_t T_HDR_SEC   = 1234567UL;
static const uint32_t T_HDR_NSEC  = 987654321UL;
static const char*    T_HDR_FRAME = "test_frame";

class InterfaceTestNode : public ZenohNode {
public:
    InterfaceTestNode() : ZenohNode("interface_test_publisher") {
        Serial.printf("[Node] %s started\n", z_get_name());
        create_publishers();
        timer_ = z_create_timer(1000, [this]() { publish_next(); });
    }

private:
    int step_ = 0;

    ZenohPublisher<z_Bool>*              pb_bool   = nullptr;
    ZenohPublisher<z_Int8>*              pb_i8     = nullptr;
    ZenohPublisher<z_UInt8>*             pb_u8     = nullptr;
    ZenohPublisher<z_Int16>*             pb_i16    = nullptr;
    ZenohPublisher<z_UInt16>*            pb_u16    = nullptr;
    ZenohPublisher<z_Int32>*             pb_i32    = nullptr;
    ZenohPublisher<z_UInt32>*            pb_u32    = nullptr;
    ZenohPublisher<z_Int64>*             pb_i64    = nullptr;
    ZenohPublisher<z_UInt64>*            pb_u64    = nullptr;
    ZenohPublisher<z_Float32>*           pb_f32    = nullptr;
    ZenohPublisher<z_Float64>*           pb_f64    = nullptr;
    ZenohPublisher<z_String>*            pb_str    = nullptr;
    ZenohPublisher<z_Header>*            pb_hdr    = nullptr;
    ZenohPublisher<z_Int32MultiArray>*   pb_i32arr = nullptr;
    ZenohPublisher<z_Float64MultiArray>* pb_f64arr = nullptr;

    ZenohPublisher<z_Imu>*               pb_imu   = nullptr;
    ZenohPublisher<z_Temperature>*        pb_temp  = nullptr;
    ZenohPublisher<z_Range>*              pb_range = nullptr;
    ZenohPublisher<z_RelativeHumidity>*   pb_hum   = nullptr;
    ZenohPublisher<z_BatteryState>*       pb_bat   = nullptr;
    ZenohPublisher<z_NavSatFix>*          pb_gps   = nullptr;
    ZenohPublisher<z_JoyFeedback>*        pb_joy   = nullptr;
    ZenohPublisher<z_JointState>*         pb_joint = nullptr;

    ZenohPublisher<z_Vector3>*    pb_vec3  = nullptr;
    ZenohPublisher<z_Quaternion>* pb_quat  = nullptr;
    ZenohPublisher<z_Pose>*       pb_pose  = nullptr;
    ZenohPublisher<z_Twist>*      pb_twist = nullptr;

    ZenohTimer* timer_ = nullptr;

    void create_publishers() {
        pb_bool   = z_create_publisher<z_Bool>            ("test/bool",          5);
        pb_i8     = z_create_publisher<z_Int8>             ("test/int8",          5);
        pb_u8     = z_create_publisher<z_UInt8>            ("test/uint8",         5);
        pb_i16    = z_create_publisher<z_Int16>            ("test/int16",         5);
        pb_u16    = z_create_publisher<z_UInt16>           ("test/uint16",        5);
        pb_i32    = z_create_publisher<z_Int32>            ("test/int32",         5);
        pb_u32    = z_create_publisher<z_UInt32>           ("test/uint32",        5);
        pb_i64    = z_create_publisher<z_Int64>            ("test/int64",         5);
        pb_u64    = z_create_publisher<z_UInt64>           ("test/uint64",        5);
        pb_f32    = z_create_publisher<z_Float32>          ("test/float32",       5);
        pb_f64    = z_create_publisher<z_Float64>          ("test/float64",       5);
        pb_str    = z_create_publisher<z_String>           ("test/string",        5);
        pb_hdr    = z_create_publisher<z_Header>           ("test/header",        5);
        pb_i32arr = z_create_publisher<z_Int32MultiArray>  ("test/int32_array",   5);
        pb_f64arr = z_create_publisher<z_Float64MultiArray>("test/float64_array", 5);

        pb_imu    = z_create_publisher<z_Imu>           ("test/imu",           5);
        pb_temp   = z_create_publisher<z_Temperature>   ("test/temperature",   5);
        pb_range  = z_create_publisher<z_Range>         ("test/range",         5);
        pb_hum    = z_create_publisher<z_RelativeHumidity>("test/humidity",    5);
        pb_bat    = z_create_publisher<z_BatteryState>  ("test/battery",       5);
        pb_gps    = z_create_publisher<z_NavSatFix>     ("test/navsatfix",     5);
        pb_joy    = z_create_publisher<z_JoyFeedback>   ("test/joyfeedback",   5);
        pb_joint  = z_create_publisher<z_JointState>    ("test/jointstate",    5);

        pb_vec3   = z_create_publisher<z_Vector3>     ("test/vector3",       5);
        pb_quat   = z_create_publisher<z_Quaternion>  ("test/quaternion",    5);
        pb_pose   = z_create_publisher<z_Pose>        ("test/pose",          5);
        pb_twist  = z_create_publisher<z_Twist>       ("test/twist",         5);
        Serial.println("[Publisher] All 27 direct-included publishers declared OK");
    }

    z_std_msgs::z_Header sentinel_header() {
        z_std_msgs::z_Header h;
        h.stamp.sec = T_HDR_SEC; h.stamp.nanosec = T_HDR_NSEC; h.frame_id = T_HDR_FRAME;
        return h;
    }

    void publish_next() {
        int s = step_ % 27;
        switch (s) {
        case 0:  { z_Bool m; m.data=T_BOOL; pb_bool->publish(m); Serial.printf("[%d/27] bool=%s\n",s+1,T_BOOL?"true":"false"); break; }
        case 1:  { z_Int8 m; m.data=T_INT8; pb_i8->publish(m); Serial.printf("[%d/27] int8=%d\n",s+1,(int)T_INT8); break; }
        case 2:  { z_UInt8 m; m.data=T_UINT8; pb_u8->publish(m); Serial.printf("[%d/27] uint8=%u\n",s+1,(unsigned)T_UINT8); break; }
        case 3:  { z_Int16 m; m.data=T_INT16; pb_i16->publish(m); Serial.printf("[%d/27] int16=%d\n",s+1,(int)T_INT16); break; }
        case 4:  { z_UInt16 m; m.data=T_UINT16; pb_u16->publish(m); Serial.printf("[%d/27] uint16=%u\n",s+1,(unsigned)T_UINT16); break; }
        case 5:  { z_Int32 m; m.data=T_INT32; pb_i32->publish(m); Serial.printf("[%d/27] int32=%d\n",s+1,T_INT32); break; }
        case 6:  { z_UInt32 m; m.data=T_UINT32; pb_u32->publish(m); Serial.printf("[%d/27] uint32=%lu\n",s+1,(unsigned long)T_UINT32); break; }
        case 7:  { z_Int64 m; m.data=T_INT64; pb_i64->publish(m); Serial.printf("[%d/27] int64=%lld\n",s+1,(long long)T_INT64); break; }
        case 8:  { z_UInt64 m; m.data=18446744073709551000ULL; pb_u64->publish(m); Serial.printf("[%d/27] uint64 sent\n",s+1); break; }
        case 9:  { z_Float32 m; m.data=T_FLOAT32; pb_f32->publish(m); Serial.printf("[%d/27] float32=%.5f\n",s+1,(double)T_FLOAT32); break; }
        case 10: { z_Float64 m; m.data=T_FLOAT64; pb_f64->publish(m); Serial.printf("[%d/27] float64=%.9f\n",s+1,T_FLOAT64); break; }
        case 11: { z_String m; m.data=T_STRING; pb_str->publish(m); Serial.printf("[%d/27] string=%s\n",s+1,T_STRING); break; }
        case 12: { z_Header m=sentinel_header(); pb_hdr->publish(m); Serial.printf("[%d/27] header\n",s+1); break; }
        case 13: {
            z_Int32MultiArray m; m.data={-100,0,100,200,-32768};
            pb_i32arr->publish(m); Serial.printf("[%d/27] int32_array[5]\n",s+1); break;
        }
        case 14: {
            z_Float64MultiArray m; m.data={1.1,2.2,3.3,-4.4,5.5};
            pb_f64arr->publish(m); Serial.printf("[%d/27] float64_array[5]\n",s+1); break;
        }
        case 15: {
            z_Imu m;
            m.header=sentinel_header();
            m.orientation={0.1,0.2,0.3,0.9165};
            m.angular_velocity={0.01,0.02,0.03};
            m.linear_acceleration={1.0,2.0,9.81};
            pb_imu->publish(m); Serial.printf("[%d/27] imu\n",s+1); break;
        }
        case 16: {
            z_Temperature m;
            m.header=sentinel_header(); m.temperature=36.6; m.variance=0.1;
            pb_temp->publish(m); Serial.printf("[%d/27] temperature=36.6\n",s+1); break;
        }
        case 17: {
            z_Range m;
            m.header=sentinel_header(); m.radiation_type=0;
            m.field_of_view=0.523f; m.min_range=0.02f; m.max_range=4.0f; m.range=1.234f;
            pb_range->publish(m); Serial.printf("[%d/27] range=1.234\n",s+1); break;
        }
        case 18: {
            z_RelativeHumidity m;
            m.header=sentinel_header(); m.relative_humidity=0.65; m.variance=0.01;
            pb_hum->publish(m); Serial.printf("[%d/27] humidity=0.65\n",s+1); break;
        }
        case 19: {
            z_BatteryState m;
            m.header=sentinel_header(); m.voltage=12.4f; m.percentage=0.85f;
            m.power_supply_status=2; m.present=true;
            pb_bat->publish(m); Serial.printf("[%d/27] battery 12.4V\n",s+1); break;
        }
        case 20: {
            z_NavSatFix m;
            m.header=sentinel_header(); m.status.status=0; m.status.service=1;
            m.latitude=24.8607; m.longitude=67.0011; m.altitude=8.0;
            m.position_covariance_type=0;
            pb_gps->publish(m); Serial.printf("[%d/27] navsatfix\n",s+1); break;
        }
        case 21: {
            z_JoyFeedback m; m.type=0; m.id=3; m.intensity=0.75f;
            pb_joy->publish(m); Serial.printf("[%d/27] joyfeedback\n",s+1); break;
        }
        case 22: {
            z_JointState m;
            m.header=sentinel_header();
            m.name={"joint1","joint2","joint3"};
            m.position={0.1,0.2,0.3}; m.velocity={1.0,2.0,3.0}; m.effort={10.0,20.0,30.0};
            pb_joint->publish(m); Serial.printf("[%d/27] jointstate\n",s+1); break;
        }
        case 23: {
            z_Vector3 m; m.x=1.0; m.y=2.0; m.z=3.0;
            pb_vec3->publish(m); Serial.printf("[%d/27] vector3\n",s+1); break;
        }
        case 24: {
            z_Quaternion m; m.x=0.0; m.y=0.0; m.z=0.707; m.w=0.707;
            pb_quat->publish(m); Serial.printf("[%d/27] quaternion\n",s+1); break;
        }
        case 25: {
            z_Pose m; m.position={1.0,2.0,3.0}; m.orientation={0.0,0.0,0.0,1.0};
            pb_pose->publish(m); Serial.printf("[%d/27] pose\n",s+1); break;
        }
        case 26: {
            z_Twist m;
            m.linear={1.5,0.0,0.0}; m.angular={0.0,0.0,0.5};
            pb_twist->publish(m); Serial.printf("[%d/27] twist\n",s+1); break;
        }
        }
        step_++;
        if (s == 26)
            Serial.println("\n[TEST] === Full cycle (27/27). Repeating... ===\n");
    }
};

InterfaceTestNode* node_instance = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000);
    Serial.println("\n==========================================");
    Serial.println("  zenoh_ros Interface Test Publisher");
    Serial.println("  Testing ALL 27 direct per-message includes!");
    Serial.println("==========================================");
    if (ZenohNode::init(cfg)) {
        node_instance = new InterfaceTestNode();
    } else {
        Serial.println("[CRITICAL] Zenoh init failed!");
        while(1) { z_delay(1000); }
    }
    Serial.println("==========================================\n");
}

void loop() {
    if (node_instance) node_instance->z_spin();
}
