#ifndef Z_GEOMETRY_MSGS_PC_H
#define Z_GEOMETRY_MSGS_PC_H

#include <nlohmann/json.hpp>
#include <vector>

// Forward declarations of serialization templates
template <typename T>
std::vector<uint8_t> serialize_msg_pc(const T& msg);

template <typename T>
void deserialize_msg_pc(const std::vector<uint8_t>& buffer, T& msg);

namespace z_geometry_msgs {

    struct Vector3 {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };

    struct Point {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };

    struct Quaternion {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double w = 1.0;
    };

    struct Pose {
        Point position;
        Quaternion orientation;
    };

    struct Twist {
        Vector3 linear;
        Vector3 angular;
    };

    struct Wrench {
        Vector3 force;
        Vector3 torque;
    };

    struct Transform {
        Vector3 translation;
        Quaternion rotation;
    };

    struct Pose2D {
        double x = 0.0;
        double y = 0.0;
        double theta = 0.0;
    };

} // namespace z_geometry_msgs

// --- Serialization Specializations ---

template <>
inline std::vector<uint8_t> serialize_msg_pc<z_geometry_msgs::Vector3>(const z_geometry_msgs::Vector3& msg) {
    nlohmann::json j = {msg.x, msg.y, msg.z};
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg_pc<z_geometry_msgs::Point>(const z_geometry_msgs::Point& msg) {
    nlohmann::json j = {msg.x, msg.y, msg.z};
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg_pc<z_geometry_msgs::Quaternion>(const z_geometry_msgs::Quaternion& msg) {
    nlohmann::json j = {msg.x, msg.y, msg.z, msg.w};
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg_pc<z_geometry_msgs::Pose>(const z_geometry_msgs::Pose& msg) {
    nlohmann::json j = {
        {msg.position.x, msg.position.y, msg.position.z},
        {msg.orientation.x, msg.orientation.y, msg.orientation.z, msg.orientation.w}
    };
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg_pc<z_geometry_msgs::Twist>(const z_geometry_msgs::Twist& msg) {
    nlohmann::json j = {
        {msg.linear.x, msg.linear.y, msg.linear.z},
        {msg.angular.x, msg.angular.y, msg.angular.z}
    };
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg_pc<z_geometry_msgs::Wrench>(const z_geometry_msgs::Wrench& msg) {
    nlohmann::json j = {
        {msg.force.x, msg.force.y, msg.force.z},
        {msg.torque.x, msg.torque.y, msg.torque.z}
    };
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg_pc<z_geometry_msgs::Transform>(const z_geometry_msgs::Transform& msg) {
    nlohmann::json j = {
        {msg.translation.x, msg.translation.y, msg.translation.z},
        {msg.rotation.x, msg.rotation.y, msg.rotation.z, msg.rotation.w}
    };
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg_pc<z_geometry_msgs::Pose2D>(const z_geometry_msgs::Pose2D& msg) {
    nlohmann::json j = {msg.x, msg.y, msg.theta};
    return nlohmann::json::to_msgpack(j);
}

// --- Deserialization Specializations ---

template <>
inline void deserialize_msg_pc<z_geometry_msgs::Vector3>(const std::vector<uint8_t>& buffer, z_geometry_msgs::Vector3& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.x = j[0].get<double>();
    msg.y = j[1].get<double>();
    msg.z = j[2].get<double>();
}

template <>
inline void deserialize_msg_pc<z_geometry_msgs::Point>(const std::vector<uint8_t>& buffer, z_geometry_msgs::Point& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.x = j[0].get<double>();
    msg.y = j[1].get<double>();
    msg.z = j[2].get<double>();
}

template <>
inline void deserialize_msg_pc<z_geometry_msgs::Quaternion>(const std::vector<uint8_t>& buffer, z_geometry_msgs::Quaternion& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.x = j[0].get<double>();
    msg.y = j[1].get<double>();
    msg.z = j[2].get<double>();
    msg.w = j[3].get<double>();
}

template <>
inline void deserialize_msg_pc<z_geometry_msgs::Pose>(const std::vector<uint8_t>& buffer, z_geometry_msgs::Pose& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.position.x = j[0][0].get<double>();
    msg.position.y = j[0][1].get<double>();
    msg.position.z = j[0][2].get<double>();
    
    msg.orientation.x = j[1][0].get<double>();
    msg.orientation.y = j[1][1].get<double>();
    msg.orientation.z = j[1][2].get<double>();
    msg.orientation.w = j[1][3].get<double>();
}

template <>
inline void deserialize_msg_pc<z_geometry_msgs::Twist>(const std::vector<uint8_t>& buffer, z_geometry_msgs::Twist& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.linear.x = j[0][0].get<double>();
    msg.linear.y = j[0][1].get<double>();
    msg.linear.z = j[0][2].get<double>();
    
    msg.angular.x = j[1][0].get<double>();
    msg.angular.y = j[1][1].get<double>();
    msg.angular.z = j[1][2].get<double>();
}

template <>
inline void deserialize_msg_pc<z_geometry_msgs::Wrench>(const std::vector<uint8_t>& buffer, z_geometry_msgs::Wrench& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.force.x = j[0][0].get<double>();
    msg.force.y = j[0][1].get<double>();
    msg.force.z = j[0][2].get<double>();
    
    msg.torque.x = j[1][0].get<double>();
    msg.torque.y = j[1][1].get<double>();
    msg.torque.z = j[1][2].get<double>();
}

template <>
inline void deserialize_msg_pc<z_geometry_msgs::Transform>(const std::vector<uint8_t>& buffer, z_geometry_msgs::Transform& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.translation.x = j[0][0].get<double>();
    msg.translation.y = j[0][1].get<double>();
    msg.translation.z = j[0][2].get<double>();
    
    msg.rotation.x = j[1][0].get<double>();
    msg.rotation.y = j[1][1].get<double>();
    msg.rotation.z = j[1][2].get<double>();
    msg.rotation.w = j[1][3].get<double>();
}

template <>
inline void deserialize_msg_pc<z_geometry_msgs::Pose2D>(const std::vector<uint8_t>& buffer, z_geometry_msgs::Pose2D& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.x = j[0].get<double>();
    msg.y = j[1].get<double>();
    msg.theta = j[2].get<double>();
}

#endif // Z_GEOMETRY_MSGS_PC_H
