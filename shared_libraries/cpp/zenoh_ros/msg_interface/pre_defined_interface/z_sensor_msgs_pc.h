#ifndef Z_SENSOR_MSGS_PC_H
#define Z_SENSOR_MSGS_PC_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include "z_std_msgs.h"
#include "z_geometry_msgs_pc.h"
#include "z_builtin_interfaces.h"

// Forward declarations
template <typename T>
std::vector<uint8_t> serialize_msg_pc(const T& msg);

template <typename T>
void deserialize_msg_pc(const std::vector<uint8_t>& buffer, T& msg);

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
inline std::vector<uint8_t> serialize_msg_pc<z_sensor_msgs::z_Imu>(const z_sensor_msgs::z_Imu& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["orientation"] = {{"x", msg.orientation.x}, {"y", msg.orientation.y}, {"z", msg.orientation.z}, {"w", msg.orientation.w}};
    j["angular_velocity"] = {{"x", msg.angular_velocity.x}, {"y", msg.angular_velocity.y}, {"z", msg.angular_velocity.z}};
    j["linear_acceleration"] = {{"x", msg.linear_acceleration.x}, {"y", msg.linear_acceleration.y}, {"z", msg.linear_acceleration.z}};
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg_pc<z_sensor_msgs::z_Imu>(const std::vector<uint8_t>& buffer, z_sensor_msgs::z_Imu& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.header.stamp.sec = j["header"]["stamp"]["sec"].get<int32_t>();
    msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
    msg.header.frame_id = j["header"]["frame_id"].get<std::string>();

    msg.orientation.x = j["orientation"]["x"].get<double>();
    msg.orientation.y = j["orientation"]["y"].get<double>();
    msg.orientation.z = j["orientation"]["z"].get<double>();
    msg.orientation.w = j["orientation"]["w"].get<double>();

    msg.angular_velocity.x = j["angular_velocity"]["x"].get<double>();
    msg.angular_velocity.y = j["angular_velocity"]["y"].get<double>();
    msg.angular_velocity.z = j["angular_velocity"]["z"].get<double>();

    msg.linear_acceleration.x = j["linear_acceleration"]["x"].get<double>();
    msg.linear_acceleration.y = j["linear_acceleration"]["y"].get<double>();
    msg.linear_acceleration.z = j["linear_acceleration"]["z"].get<double>();
}

template <>
inline std::vector<uint8_t> serialize_msg_pc<z_sensor_msgs::z_Temperature>(const z_sensor_msgs::z_Temperature& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["temperature"] = msg.temperature;
    j["variance"] = msg.variance;
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg_pc<z_sensor_msgs::z_Temperature>(const std::vector<uint8_t>& buffer, z_sensor_msgs::z_Temperature& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.header.stamp.sec = j["header"]["stamp"]["sec"].get<int32_t>();
    msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
    msg.header.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.temperature = j["temperature"].get<double>();
    msg.variance = j["variance"].get<double>();
}

template <>
inline std::vector<uint8_t> serialize_msg_pc<z_sensor_msgs::z_Range>(const z_sensor_msgs::z_Range& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["radiation_type"] = msg.radiation_type;
    j["field_of_view"] = msg.field_of_view;
    j["min_range"] = msg.min_range;
    j["max_range"] = msg.max_range;
    j["range"] = msg.range;
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg_pc<z_sensor_msgs::z_Range>(const std::vector<uint8_t>& buffer, z_sensor_msgs::z_Range& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.header.stamp.sec = j["header"]["stamp"]["sec"].get<int32_t>();
    msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
    msg.header.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.radiation_type = j["radiation_type"].get<uint8_t>();
    msg.field_of_view = j["field_of_view"].get<float>();
    msg.min_range = j["min_range"].get<float>();
    msg.max_range = j["max_range"].get<float>();
    msg.range = j["range"].get<float>();
}

#endif // Z_SENSOR_MSGS_PC_H
