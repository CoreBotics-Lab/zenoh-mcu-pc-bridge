#ifndef Z_STD_MSGS_H
#define Z_STD_MSGS_H

#include <ArduinoJson.h>
#include <vector>
#include <string>

// --- Helper macro to define primitive message types and their serialization traits ---
#define DEFINE_Z_STD_MSG_PRIMITIVE(TypeName, Type) \
    namespace z_std_msgs { \
        struct TypeName { \
            Type data; \
        }; \
    } \
    template <> \
    inline size_t serialize_msg<z_std_msgs::TypeName>(const z_std_msgs::TypeName& msg, uint8_t* buffer, size_t max_len) { \
        JsonDocument doc; \
        doc.add(msg.data); \
        return serializeMsgPack(doc, buffer, max_len); \
    } \
    template <> \
    inline void deserialize_msg<z_std_msgs::TypeName>(const uint8_t* buffer, size_t len, z_std_msgs::TypeName& msg) { \
        JsonDocument doc; \
        deserializeMsgPack(doc, buffer, len); \
        msg.data = doc[0].as<Type>(); \
    }

// --- Helper macro to define array message types and their serialization traits ---
#define DEFINE_Z_STD_MSG_ARRAY(TypeName, Type) \
    namespace z_std_msgs { \
        struct TypeName { \
            std::vector<Type> data; \
        }; \
    } \
    template <> \
    inline size_t serialize_msg<z_std_msgs::TypeName>(const z_std_msgs::TypeName& msg, uint8_t* buffer, size_t max_len) { \
        JsonDocument doc; \
        JsonArray array = doc.to<JsonArray>(); \
        for (Type val : msg.data) { \
            array.add(val); \
        } \
        return serializeMsgPack(doc, buffer, max_len); \
    } \
    template <> \
    inline void deserialize_msg<z_std_msgs::TypeName>(const uint8_t* buffer, size_t len, z_std_msgs::TypeName& msg) { \
        JsonDocument doc; \
        deserializeMsgPack(doc, buffer, len); \
        JsonArrayConst array = doc.as<JsonArrayConst>(); \
        msg.data.clear(); \
        for (JsonVariantConst val : array) { \
            msg.data.push_back(val.as<Type>()); \
        } \
    }

// Define Primitives
DEFINE_Z_STD_MSG_PRIMITIVE(Bool, bool)
DEFINE_Z_STD_MSG_PRIMITIVE(Byte, int8_t)
DEFINE_Z_STD_MSG_PRIMITIVE(Char, uint8_t)
DEFINE_Z_STD_MSG_PRIMITIVE(Int8, int8_t)
DEFINE_Z_STD_MSG_PRIMITIVE(UInt8, uint8_t)
DEFINE_Z_STD_MSG_PRIMITIVE(Int16, int16_t)
DEFINE_Z_STD_MSG_PRIMITIVE(UInt16, uint16_t)
DEFINE_Z_STD_MSG_PRIMITIVE(Int32, int32_t)
DEFINE_Z_STD_MSG_PRIMITIVE(UInt32, uint32_t)
DEFINE_Z_STD_MSG_PRIMITIVE(Int64, int64_t)
DEFINE_Z_STD_MSG_PRIMITIVE(UInt64, uint64_t)
DEFINE_Z_STD_MSG_PRIMITIVE(Float32, float)
DEFINE_Z_STD_MSG_PRIMITIVE(Float64, double)

// Define Arrays
DEFINE_Z_STD_MSG_ARRAY(ByteMultiArray, int8_t)
DEFINE_Z_STD_MSG_ARRAY(Int8MultiArray, int8_t)
DEFINE_Z_STD_MSG_ARRAY(UInt8MultiArray, uint8_t)
DEFINE_Z_STD_MSG_ARRAY(Int16MultiArray, int16_t)
DEFINE_Z_STD_MSG_ARRAY(UInt16MultiArray, uint16_t)
DEFINE_Z_STD_MSG_ARRAY(Int32MultiArray, int32_t)
DEFINE_Z_STD_MSG_ARRAY(UInt32MultiArray, uint32_t)
DEFINE_Z_STD_MSG_ARRAY(Int64MultiArray, int64_t)
DEFINE_Z_STD_MSG_ARRAY(UInt64MultiArray, uint64_t)
DEFINE_Z_STD_MSG_ARRAY(Float32MultiArray, float)
DEFINE_Z_STD_MSG_ARRAY(Float64MultiArray, double)

// --- Custom Specialization for z_std_msgs::String ---
namespace z_std_msgs {
    struct String {
        std::string data;
    };
}
template <>
inline size_t serialize_msg<z_std_msgs::String>(const z_std_msgs::String& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc.add(msg.data);
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_std_msgs::String>(const uint8_t* buffer, size_t len, z_std_msgs::String& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.data = doc[0].as<std::string>();
}

// --- Custom Specialization for z_std_msgs::Empty ---
namespace z_std_msgs {
    struct Empty {};
}
template <>
inline size_t serialize_msg<z_std_msgs::Empty>(const z_std_msgs::Empty& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_std_msgs::Empty>(const uint8_t* buffer, size_t len, z_std_msgs::Empty& msg) {
    (void)buffer; (void)len; (void)msg;
}

// --- Passthrough Specialization for Raw messages ---
namespace z_std_msgs {
    struct Raw {
        std::vector<uint8_t> data;
    };
}
template <>
inline size_t serialize_msg<z_std_msgs::Raw>(const z_std_msgs::Raw& msg, uint8_t* buffer, size_t max_len) {
    size_t copy_len = msg.data.size() < max_len ? msg.data.size() : max_len;
    memcpy(buffer, msg.data.data(), copy_len);
    return copy_len;
}
template <>
inline void deserialize_msg<z_std_msgs::Raw>(const uint8_t* buffer, size_t len, z_std_msgs::Raw& msg) {
    msg.data.assign(buffer, buffer + len);
}

// --- Specializations for ArduinoJson::JsonDocument ---
template <>
inline size_t serialize_msg<ArduinoJson::JsonDocument>(const ArduinoJson::JsonDocument& msg, uint8_t* buffer, size_t max_len) {
    return serializeMsgPack(msg, buffer, max_len);
}
template <>
inline void deserialize_msg<ArduinoJson::JsonDocument>(const uint8_t* buffer, size_t len, ArduinoJson::JsonDocument& msg) {
    deserializeMsgPack(msg, buffer, len);
}

#endif // Z_STD_MSGS_H
