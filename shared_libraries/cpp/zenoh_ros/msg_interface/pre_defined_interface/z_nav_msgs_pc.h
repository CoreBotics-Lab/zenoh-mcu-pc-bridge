#ifndef Z_NAV_MSGS_PC_H
#define Z_NAV_MSGS_PC_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include "z_std_msgs.h"
#include "z_geometry_msgs_pc.h"

// Forward declarations
template <typename T>
std::vector<uint8_t> serialize_msg_pc(const T& msg);

template <typename T>
void deserialize_msg_pc(const std::vector<uint8_t>& buffer, T& msg);

namespace z_nav_msgs {

    struct z_Odometry {
        z_std_msgs::z_Header header;
        std::string child_frame_id;
        z_geometry_msgs::z_Pose pose;
        double pose_covariance[36] = {0};
        z_geometry_msgs::z_Twist twist;
        double twist_covariance[36] = {0};
    };

    struct z_Path {
        z_std_msgs::z_Header header;
        std::vector<z_geometry_msgs::z_Pose> poses;
    };

} // namespace z_nav_msgs

// --- Serialization Specializations ---

template <>
inline std::vector<uint8_t> serialize_msg_pc<z_nav_msgs::z_Odometry>(const z_nav_msgs::z_Odometry& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["child_frame_id"] = msg.child_frame_id;
    j["pose"]["pose"]["position"] = {{"x", msg.pose.position.x}, {"y", msg.pose.position.y}, {"z", msg.pose.position.z}};
    j["pose"]["pose"]["orientation"] = {{"x", msg.pose.orientation.x}, {"y", msg.pose.orientation.y}, {"z", msg.pose.orientation.z}, {"w", msg.pose.orientation.w}};
    j["twist"]["twist"]["linear"] = {{"x", msg.twist.linear.x}, {"y", msg.twist.linear.y}, {"z", msg.twist.linear.z}};
    j["twist"]["twist"]["angular"] = {{"x", msg.twist.angular.x}, {"y", msg.twist.angular.y}, {"z", msg.twist.angular.z}};
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg_pc<z_nav_msgs::z_Odometry>(const std::vector<uint8_t>& buffer, z_nav_msgs::z_Odometry& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.header.stamp.sec = j["header"]["stamp"]["sec"].get<int32_t>();
    msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
    msg.header.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.child_frame_id = j["child_frame_id"].get<std::string>();

    msg.pose.position.x = j["pose"]["pose"]["position"]["x"].get<double>();
    msg.pose.position.y = j["pose"]["pose"]["position"]["y"].get<double>();
    msg.pose.position.z = j["pose"]["pose"]["position"]["z"].get<double>();
    msg.pose.orientation.x = j["pose"]["pose"]["orientation"]["x"].get<double>();
    msg.pose.orientation.y = j["pose"]["pose"]["orientation"]["y"].get<double>();
    msg.pose.orientation.z = j["pose"]["pose"]["orientation"]["z"].get<double>();
    msg.pose.orientation.w = j["pose"]["pose"]["orientation"]["w"].get<double>();

    msg.twist.linear.x = j["twist"]["twist"]["linear"]["x"].get<double>();
    msg.twist.linear.y = j["twist"]["twist"]["linear"]["y"].get<double>();
    msg.twist.linear.z = j["twist"]["twist"]["linear"]["z"].get<double>();
    msg.twist.angular.x = j["twist"]["twist"]["angular"]["x"].get<double>();
    msg.twist.angular.y = j["twist"]["twist"]["angular"]["y"].get<double>();
    msg.twist.angular.z = j["twist"]["twist"]["angular"]["z"].get<double>();
}

#endif // Z_NAV_MSGS_PC_H
