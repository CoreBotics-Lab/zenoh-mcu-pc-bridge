#ifndef Z_SENSOR_MSGS_H
#define Z_SENSOR_MSGS_H

#include <ArduinoJson.h>
#include <vector>
#include <string>
#include "z_std_msgs.h"
#include "z_geometry_msgs.h"
#include "z_builtin_interfaces.h"

namespace z_sensor_msgs {

    struct z_Imu {
        z_std_msgs::z_Header header;
        z_geometry_msgs::z_Quaternion orientation;
        double orientation_covariance[9] = {0};
        z_geometry_msgs::z_Vector3 angular_velocity;
        double angular_velocity_covariance[9] = {0};
        z_geometry_msgs::z_Vector3 linear_acceleration;
        double linear_acceleration_covariance[9] = {0};
    };

    struct z_JointState {
        z_std_msgs::z_Header header;
        std::vector<std::string> name;
        std::vector<double> position;
        std::vector<double> velocity;
        std::vector<double> effort;
    };

    struct z_Temperature {
        z_std_msgs::z_Header header;
        double temperature = 0.0;
        double variance = 0.0;
    };

    struct z_FluidPressure {
        z_std_msgs::z_Header header;
        double fluid_pressure = 0.0;
        double variance = 0.0;
    };

    struct z_Illuminance {
        z_std_msgs::z_Header header;
        double illuminance = 0.0;
        double variance = 0.0;
    };

    struct z_MagneticField {
        z_std_msgs::z_Header header;
        z_geometry_msgs::z_Vector3 magnetic_field;
        double magnetic_field_covariance[9] = {0};
    };

    struct z_Range {
        z_std_msgs::z_Header header;
        uint8_t radiation_type = 0;
        float field_of_view = 0.0f;
        float min_range = 0.0f;
        float max_range = 0.0f;
        float range = 0.0f;
    };

} // namespace z_sensor_msgs

// --- Serialization Specializations ---

template <>
inline size_t serialize_msg<z_sensor_msgs::z_Imu>(const z_sensor_msgs::z_Imu& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonObject header_obj = doc["header"].to<JsonObject>();
    header_obj["stamp"]["sec"] = msg.header.stamp.sec;
    header_obj["stamp"]["nanosec"] = msg.header.stamp.nanosec;
    header_obj["frame_id"] = msg.header.frame_id;

    JsonObject ori = doc["orientation"].to<JsonObject>();
    ori["x"] = msg.orientation.x; ori["y"] = msg.orientation.y; ori["z"] = msg.orientation.z; ori["w"] = msg.orientation.w;

    JsonObject ang = doc["angular_velocity"].to<JsonObject>();
    ang["x"] = msg.angular_velocity.x; ang["y"] = msg.angular_velocity.y; ang["z"] = msg.angular_velocity.z;

    JsonObject lin = doc["linear_acceleration"].to<JsonObject>();
    lin["x"] = msg.linear_acceleration.x; lin["y"] = msg.linear_acceleration.y; lin["z"] = msg.linear_acceleration.z;

    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_sensor_msgs::z_Imu>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_Imu& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>();
    msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>();
    msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();

    msg.orientation.x = doc["orientation"]["x"].as<double>();
    msg.orientation.y = doc["orientation"]["y"].as<double>();
    msg.orientation.z = doc["orientation"]["z"].as<double>();
    msg.orientation.w = doc["orientation"]["w"].as<double>();

    msg.angular_velocity.x = doc["angular_velocity"]["x"].as<double>();
    msg.angular_velocity.y = doc["angular_velocity"]["y"].as<double>();
    msg.angular_velocity.z = doc["angular_velocity"]["z"].as<double>();

    msg.linear_acceleration.x = doc["linear_acceleration"]["x"].as<double>();
    msg.linear_acceleration.y = doc["linear_acceleration"]["y"].as<double>();
    msg.linear_acceleration.z = doc["linear_acceleration"]["z"].as<double>();
}

template <>
inline size_t serialize_msg<z_sensor_msgs::z_Temperature>(const z_sensor_msgs::z_Temperature& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec;
    doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec;
    doc["header"]["frame_id"] = msg.header.frame_id;
    doc["temperature"] = msg.temperature;
    doc["variance"] = msg.variance;
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_sensor_msgs::z_Temperature>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_Temperature& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>();
    msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>();
    msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.temperature = doc["temperature"].as<double>();
    msg.variance = doc["variance"].as<double>();
}

template <>
inline size_t serialize_msg<z_sensor_msgs::z_Range>(const z_sensor_msgs::z_Range& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec;
    doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec;
    doc["header"]["frame_id"] = msg.header.frame_id;
    doc["radiation_type"] = msg.radiation_type;
    doc["field_of_view"] = msg.field_of_view;
    doc["min_range"] = msg.min_range;
    doc["max_range"] = msg.max_range;
    doc["range"] = msg.range;
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_sensor_msgs::z_Range>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_Range& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>();
    msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>();
    msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.radiation_type = doc["radiation_type"].as<uint8_t>();
    msg.field_of_view = doc["field_of_view"].as<float>();
    msg.min_range = doc["min_range"].as<float>();
    msg.max_range = doc["max_range"].as<float>();
    msg.range = doc["range"].as<float>();
}

#endif // Z_SENSOR_MSGS_H
