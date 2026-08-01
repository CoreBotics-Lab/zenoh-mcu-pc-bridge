#ifndef Z_NAV_MSGS_H
#define Z_NAV_MSGS_H

#include <ArduinoJson.h>
#include <vector>
#include "z_std_msgs.h"
#include "z_geometry_msgs.h"

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
inline size_t serialize_msg<z_nav_msgs::z_Odometry>(const z_nav_msgs::z_Odometry& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec;
    doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec;
    doc["header"]["frame_id"] = msg.header.frame_id;
    doc["child_frame_id"] = msg.child_frame_id;

    JsonObject pos = doc["pose"]["pose"]["position"].to<JsonObject>();
    pos["x"] = msg.pose.position.x; pos["y"] = msg.pose.position.y; pos["z"] = msg.pose.position.z;
    JsonObject ori = doc["pose"]["pose"]["orientation"].to<JsonObject>();
    ori["x"] = msg.pose.orientation.x; ori["y"] = msg.pose.orientation.y; ori["z"] = msg.pose.orientation.z; ori["w"] = msg.pose.orientation.w;

    JsonObject lin = doc["twist"]["twist"]["linear"].to<JsonObject>();
    lin["x"] = msg.twist.linear.x; lin["y"] = msg.twist.linear.y; lin["z"] = msg.twist.linear.z;
    JsonObject ang = doc["twist"]["twist"]["angular"].to<JsonObject>();
    ang["x"] = msg.twist.angular.x; ang["y"] = msg.twist.angular.y; ang["z"] = msg.twist.angular.z;

    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_nav_msgs::z_Odometry>(const uint8_t* buffer, size_t len, z_nav_msgs::z_Odometry& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>();
    msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>();
    msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.child_frame_id = doc["child_frame_id"].as<std::string>();

    msg.pose.position.x = doc["pose"]["pose"]["position"]["x"].as<double>();
    msg.pose.position.y = doc["pose"]["pose"]["position"]["y"].as<double>();
    msg.pose.position.z = doc["pose"]["pose"]["position"]["z"].as<double>();
    msg.pose.orientation.x = doc["pose"]["pose"]["orientation"]["x"].as<double>();
    msg.pose.orientation.y = doc["pose"]["pose"]["orientation"]["y"].as<double>();
    msg.pose.orientation.z = doc["pose"]["pose"]["orientation"]["z"].as<double>();
    msg.pose.orientation.w = doc["pose"]["pose"]["orientation"]["w"].as<double>();

    msg.twist.linear.x = doc["twist"]["twist"]["linear"]["x"].as<double>();
    msg.twist.linear.y = doc["twist"]["twist"]["linear"]["y"].as<double>();
    msg.twist.linear.z = doc["twist"]["twist"]["linear"]["z"].as<double>();
    msg.twist.angular.x = doc["twist"]["twist"]["angular"]["x"].as<double>();
    msg.twist.angular.y = doc["twist"]["twist"]["angular"]["y"].as<double>();
    msg.twist.angular.z = doc["twist"]["twist"]["angular"]["z"].as<double>();
}

#endif // Z_NAV_MSGS_H
