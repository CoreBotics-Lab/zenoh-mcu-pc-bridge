#ifndef Z_STD_MSGS_PC_H
#define Z_STD_MSGS_PC_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>

// Forward declarations of serialization templates
template <typename T>
std::vector<uint8_t> serialize_msg_pc(const T& msg);

template <typename T>
void deserialize_msg_pc(const std::vector<uint8_t>& buffer, T& msg);

// --- Helper macro to define primitive message types and their serialization traits ---
#define DEFINE_Z_STD_MSG_PRIMITIVE_PC(TypeName, Type) \
    namespace z_std_msgs { \
        struct TypeName { \
            Type data; \
        }; \
    } \
    template <> \
    inline std::vector<uint8_t> serialize_msg_pc<z_std_msgs::TypeName>(const z_std_msgs::TypeName& msg) { \
        nlohmann::json j = nlohmann::json::array(); \
        j.push_back(msg.data); \
        return nlohmann::json::to_msgpack(j); \
    } \
    template <> \
    inline void deserialize_msg_pc<z_std_msgs::TypeName>(const std::vector<uint8_t>& buffer, z_std_msgs::TypeName& msg) { \
        nlohmann::json j = nlohmann::json::from_msgpack(buffer); \
        msg.data = j[0].get<Type>(); \
    }

// --- Helper macro to define array message types and their serialization traits ---
#define DEFINE_Z_STD_MSG_ARRAY_PC(TypeName, Type) \
    namespace z_std_msgs { \
        struct TypeName { \
            std::vector<Type> data; \
        }; \
    } \
    template <> \
    inline std::vector<uint8_t> serialize_msg_pc<z_std_msgs::TypeName>(const z_std_msgs::TypeName& msg) { \
        nlohmann::json j = msg.data; \
        return nlohmann::json::to_msgpack(j); \
    } \
    template <> \
    inline void deserialize_msg_pc<z_std_msgs::TypeName>(const std::vector<uint8_t>& buffer, z_std_msgs::TypeName& msg) { \
        nlohmann::json j = nlohmann::json::from_msgpack(buffer); \
        msg.data = j.get<std::vector<Type>>(); \
    }

// Define Primitives
DEFINE_Z_STD_MSG_PRIMITIVE_PC(Bool, bool)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(Byte, int8_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(Char, uint8_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(Int8, int8_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(UInt8, uint8_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(Int16, int16_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(UInt16, uint16_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(Int32, int32_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(UInt32, uint32_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(Int64, int64_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(UInt64, uint64_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(Float32, float)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(Float64, double)

// Define Arrays
DEFINE_Z_STD_MSG_ARRAY_PC(ByteMultiArray, int8_t)
DEFINE_Z_STD_MSG_ARRAY_PC(Int8MultiArray, int8_t)
DEFINE_Z_STD_MSG_ARRAY_PC(UInt8MultiArray, uint8_t)
DEFINE_Z_STD_MSG_ARRAY_PC(Int16MultiArray, int16_t)
DEFINE_Z_STD_MSG_ARRAY_PC(UInt16MultiArray, uint16_t)
DEFINE_Z_STD_MSG_ARRAY_PC(Int32MultiArray, int32_t)
DEFINE_Z_STD_MSG_ARRAY_PC(UInt32MultiArray, uint32_t)
DEFINE_Z_STD_MSG_ARRAY_PC(Int64MultiArray, int64_t)
DEFINE_Z_STD_MSG_ARRAY_PC(UInt64MultiArray, uint64_t)
DEFINE_Z_STD_MSG_ARRAY_PC(Float32MultiArray, float)
DEFINE_Z_STD_MSG_ARRAY_PC(Float64MultiArray, double)

// --- Custom Specialization for z_std_msgs::String ---
namespace z_std_msgs {
    struct String {
        std::string data;
    };
}
template <>
inline std::vector<uint8_t> serialize_msg_pc<z_std_msgs::String>(const z_std_msgs::String& msg) {
    nlohmann::json j = nlohmann::json::array();
    j.push_back(msg.data);
    return nlohmann::json::to_msgpack(j);
}
template <>
inline void deserialize_msg_pc<z_std_msgs::String>(const std::vector<uint8_t>& buffer, z_std_msgs::String& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.data = j[0].get<std::string>();
}

// --- Custom Specialization for z_std_msgs::Empty ---
namespace z_std_msgs {
    struct Empty {};
}
template <>
inline std::vector<uint8_t> serialize_msg_pc<z_std_msgs::Empty>(const z_std_msgs::Empty& msg) {
    nlohmann::json j = nlohmann::json::array();
    return nlohmann::json::to_msgpack(j);
}
template <>
inline void deserialize_msg_pc<z_std_msgs::Empty>(const std::vector<uint8_t>& buffer, z_std_msgs::Empty& msg) {
    (void)buffer; (void)msg;
}

// --- Passthrough Specialization for Raw messages ---
namespace z_std_msgs {
    struct Raw {
        std::vector<uint8_t> data;
    };
}
template <>
inline std::vector<uint8_t> serialize_msg_pc<z_std_msgs::Raw>(const z_std_msgs::Raw& msg) {
    return msg.data;
}
template <>
inline void deserialize_msg_pc<z_std_msgs::Raw>(const std::vector<uint8_t>& buffer, z_std_msgs::Raw& msg) {
    msg.data = buffer;
}

// --- Specializations for nlohmann::json ---
template <>
inline std::vector<uint8_t> serialize_msg_pc<nlohmann::json>(const nlohmann::json& msg) {
    return nlohmann::json::to_msgpack(msg);
}
template <>
inline void deserialize_msg_pc<nlohmann::json>(const std::vector<uint8_t>& buffer, nlohmann::json& msg) {
    msg = nlohmann::json::from_msgpack(buffer);
}

#endif // Z_STD_MSGS_PC_H
