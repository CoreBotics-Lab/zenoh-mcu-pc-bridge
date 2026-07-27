#ifndef Z_GEOMETRY_MSGS_H
#define Z_GEOMETRY_MSGS_H

#include <ArduinoJson.h>

namespace z_geometry_msgs {

    struct Vector3 {
        double x;
        double y;
        double z;
    };

    struct Point {
        double x;
        double y;
        double z;
    };

    struct Quaternion {
        double x;
        double y;
        double z;
        double w;
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
        double x;
        double y;
        double theta;
    };

} // namespace z_geometry_msgs

// --- Serialization Specializations ---

template <>
inline size_t serialize_msg<z_geometry_msgs::Vector3>(const z_geometry_msgs::Vector3& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(msg.x);
    array.add(msg.y);
    array.add(msg.z);
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::Point>(const z_geometry_msgs::Point& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(msg.x);
    array.add(msg.y);
    array.add(msg.z);
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::Quaternion>(const z_geometry_msgs::Quaternion& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(msg.x);
    array.add(msg.y);
    array.add(msg.z);
    array.add(msg.w);
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::Pose>(const z_geometry_msgs::Pose& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    JsonArray pos = array.add<JsonArray>();
    pos.add(msg.position.x);
    pos.add(msg.position.y);
    pos.add(msg.position.z);
    
    JsonArray ori = array.add<JsonArray>();
    ori.add(msg.orientation.x);
    ori.add(msg.orientation.y);
    ori.add(msg.orientation.z);
    ori.add(msg.orientation.w);
    
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::Twist>(const z_geometry_msgs::Twist& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    JsonArray lin = array.add<JsonArray>();
    lin.add(msg.linear.x);
    lin.add(msg.linear.y);
    lin.add(msg.linear.z);
    
    JsonArray ang = array.add<JsonArray>();
    ang.add(msg.angular.x);
    ang.add(msg.angular.y);
    ang.add(msg.angular.z);
    
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::Wrench>(const z_geometry_msgs::Wrench& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    JsonArray force = array.add<JsonArray>();
    force.add(msg.force.x);
    force.add(msg.force.y);
    force.add(msg.force.z);
    
    JsonArray torque = array.add<JsonArray>();
    torque.add(msg.torque.x);
    torque.add(msg.torque.y);
    torque.add(msg.torque.z);
    
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::Transform>(const z_geometry_msgs::Transform& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    JsonArray trans = array.add<JsonArray>();
    trans.add(msg.translation.x);
    trans.add(msg.translation.y);
    trans.add(msg.translation.z);
    
    JsonArray rot = array.add<JsonArray>();
    rot.add(msg.rotation.x);
    rot.add(msg.rotation.y);
    rot.add(msg.rotation.z);
    rot.add(msg.rotation.w);
    
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::Pose2D>(const z_geometry_msgs::Pose2D& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(msg.x);
    array.add(msg.y);
    array.add(msg.theta);
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Deserialization Specializations ---

template <>
inline void deserialize_msg<z_geometry_msgs::Vector3>(const uint8_t* buffer, size_t len, z_geometry_msgs::Vector3& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.x = doc[0].as<double>();
    msg.y = doc[1].as<double>();
    msg.z = doc[2].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::Point>(const uint8_t* buffer, size_t len, z_geometry_msgs::Point& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.x = doc[0].as<double>();
    msg.y = doc[1].as<double>();
    msg.z = doc[2].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::Quaternion>(const uint8_t* buffer, size_t len, z_geometry_msgs::Quaternion& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.x = doc[0].as<double>();
    msg.y = doc[1].as<double>();
    msg.z = doc[2].as<double>();
    msg.w = doc[3].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::Pose>(const uint8_t* buffer, size_t len, z_geometry_msgs::Pose& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    
    msg.position.x = doc[0][0].as<double>();
    msg.position.y = doc[0][1].as<double>();
    msg.position.z = doc[0][2].as<double>();
    
    msg.orientation.x = doc[1][0].as<double>();
    msg.orientation.y = doc[1][1].as<double>();
    msg.orientation.z = doc[1][2].as<double>();
    msg.orientation.w = doc[1][3].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::Twist>(const uint8_t* buffer, size_t len, z_geometry_msgs::Twist& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    
    msg.linear.x = doc[0][0].as<double>();
    msg.linear.y = doc[0][1].as<double>();
    msg.linear.z = doc[0][2].as<double>();
    
    msg.angular.x = doc[1][0].as<double>();
    msg.angular.y = doc[1][1].as<double>();
    msg.angular.z = doc[1][2].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::Wrench>(const uint8_t* buffer, size_t len, z_geometry_msgs::Wrench& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    
    msg.force.x = doc[0][0].as<double>();
    msg.force.y = doc[0][1].as<double>();
    msg.force.z = doc[0][2].as<double>();
    
    msg.torque.x = doc[1][0].as<double>();
    msg.torque.y = doc[1][1].as<double>();
    msg.torque.z = doc[1][2].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::Transform>(const uint8_t* buffer, size_t len, z_geometry_msgs::Transform& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    
    msg.translation.x = doc[0][0].as<double>();
    msg.translation.y = doc[0][1].as<double>();
    msg.translation.z = doc[0][2].as<double>();
    
    msg.rotation.x = doc[1][0].as<double>();
    msg.rotation.y = doc[1][1].as<double>();
    msg.rotation.z = doc[1][2].as<double>();
    msg.rotation.w = doc[1][3].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::Pose2D>(const uint8_t* buffer, size_t len, z_geometry_msgs::Pose2D& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.x = doc[0].as<double>();
    msg.y = doc[1].as<double>();
    msg.theta = doc[2].as<double>();
}

#endif // Z_GEOMETRY_MSGS_H
