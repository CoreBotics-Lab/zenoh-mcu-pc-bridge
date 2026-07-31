#!/usr/bin/env python3
import os
import sys
import argparse
import shutil

# Basic ROS 2 data type mappings
TYPE_MAP_CPP = {
    'int8': 'int8_t',
    'uint8': 'uint8_t',
    'int16': 'int16_t',
    'uint16': 'uint16_t',
    'int32': 'int32_t',
    'uint32': 'uint32_t',
    'int64': 'int64_t',
    'uint64': 'uint64_t',
    'float32': 'float',
    'float64': 'double',
    'string': 'std::string',
    'String': 'std::string',
    'bool': 'bool'
}

TYPE_MAP_PYTHON = {
    'int8': 'int',
    'uint8': 'int',
    'int16': 'int',
    'uint16': 'int',
    'int32': 'int',
    'uint32': 'int',
    'int64': 'int',
    'uint64': 'int',
    'float32': 'float',
    'float64': 'float',
    'string': 'str',
    'String': 'str',
    'bool': 'bool'
}

# Standard nested types mapping
NESTED_TYPES = {
    'geometry_msgs/Vector3': 'z_geometry_msgs::z_Vector3',
    'geometry_msgs/msg/Vector3': 'z_geometry_msgs::z_Vector3',
    'geometry_msgs/Point': 'z_geometry_msgs::z_Point',
    'geometry_msgs/msg/Point': 'z_geometry_msgs::z_Point',
    'geometry_msgs/Quaternion': 'z_geometry_msgs::z_Quaternion',
    'geometry_msgs/msg/Quaternion': 'z_geometry_msgs::z_Quaternion',
    'geometry_msgs/Twist': 'z_geometry_msgs::z_Twist',
    'geometry_msgs/msg/Twist': 'z_geometry_msgs::z_Twist'
}

NESTED_PYTHON_TYPES = {
    'geometry_msgs/Vector3': 'z_geometry_msgs.z_Vector3',
    'geometry_msgs/msg/Vector3': 'z_geometry_msgs.z_Vector3',
    'geometry_msgs/Point': 'z_geometry_msgs.z_Point',
    'geometry_msgs/msg/Point': 'z_geometry_msgs.z_Point',
    'geometry_msgs/Quaternion': 'z_geometry_msgs.z_Quaternion',
    'geometry_msgs/msg/Quaternion': 'z_geometry_msgs.z_Quaternion',
    'geometry_msgs/Twist': 'z_geometry_msgs.z_Twist',
    'geometry_msgs/msg/Twist': 'z_geometry_msgs.z_Twist'
}

def parse_fields(lines):
    fields = []
    for line in lines:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        parts = line.split()
        if len(parts) >= 2:
            field_type = parts[0]
            field_name = parts[1]
            fields.append((field_type, field_name))
    return fields

def get_cpp_type(raw_type):
    if raw_type in TYPE_MAP_CPP:
        return TYPE_MAP_CPP[raw_type]
    if raw_type in NESTED_TYPES:
        return NESTED_TYPES[raw_type]
    if '/' in raw_type:
        parts = raw_type.split('/')
        return f"{parts[0]}::z_{parts[-1]}"
    return f"custom_msgs::z_{raw_type}"

def get_python_type(raw_type):
    if raw_type in TYPE_MAP_PYTHON:
        return TYPE_MAP_PYTHON[raw_type]
    if raw_type in NESTED_PYTHON_TYPES:
        return NESTED_PYTHON_TYPES[raw_type]
    return 'Any'

def get_python_default(raw_type):
    if raw_type == 'bool':
        return 'False'
    if raw_type in ['float32', 'float64']:
        return '0.0'
    if raw_type in ['string', 'String']:
        return '""'
    if raw_type in NESTED_PYTHON_TYPES:
        return f"{NESTED_PYTHON_TYPES[raw_type]}()"
    # Int fallback
    if raw_type in TYPE_MAP_PYTHON:
        return '0'
    return 'None'

def get_cpp_includes(fields, is_pc=False):
    includes = []
    has_geometry = False
    for f_type, _ in fields:
        if 'geometry_msgs' in f_type:
            has_geometry = True
        elif f_type not in TYPE_MAP_CPP:
            # Custom nested message type (e.g. SetLED -> custom_msgs/z_SetLED.h)
            pkg = 'custom_msgs'
            msg_name = f_type
            if '/' in f_type:
                parts = f_type.split('/')
                pkg = parts[0]
                msg_name = parts[-1]
            header = f"zenoh_ros/{pkg}/z_{msg_name}.h"
            includes.append(f'#include <{header}>')

    if has_geometry:
        if is_pc:
            includes.append('#include "msg_interface/pre_defined_interface/z_geometry_msgs_pc.h"')
        else:
            includes.append('#include "msg_interface/pre_defined_interface/z_geometry_msgs.h"')
    return includes

# Generate Inline Nesting Serialization/Deserialization C++ helper code
def generate_cpp_field_serialization(f_type, f_name, doc_var, msg_var):
    if f_type in TYPE_MAP_CPP:
        return f'{doc_var}["{f_name}"] = {msg_var}.{f_name};'
    
    if 'Vector3' in f_type or 'Point' in f_type:
        return (f'  JsonArray {f_name}_arr = {doc_var}.createNestedArray("{f_name}");\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.x);\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.y);\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.z);')
    
    if 'Quaternion' in f_type:
        return (f'  JsonArray {f_name}_arr = {doc_var}.createNestedArray("{f_name}");\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.x);\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.y);\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.z);\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.w);')
    
    if 'Twist' in f_type:
        return (f'  JsonArray {f_name}_arr = {doc_var}.createNestedArray("{f_name}");\n'
                f'  JsonArray {f_name}_lin = {f_name}_arr.createNestedArray();\n'
                f'  {f_name}_lin.add({msg_var}.{f_name}.linear.x);\n'
                f'  {f_name}_lin.add({msg_var}.{f_name}.linear.y);\n'
                f'  {f_name}_lin.add({msg_var}.{f_name}.linear.z);\n'
                f'  JsonArray {f_name}_ang = {f_name}_arr.createNestedArray();\n'
                f'  {f_name}_ang.add({msg_var}.{f_name}.angular.x);\n'
                f'  {f_name}_ang.add({msg_var}.{f_name}.angular.y);\n'
                f'  {f_name}_ang.add({msg_var}.{f_name}.angular.z);')
                
    # Custom nested struct serialization
    return (f'  uint8_t {f_name}_buf[256];\n'
            f'  size_t {f_name}_len = serialize_msg({msg_var}.{f_name}, {f_name}_buf, sizeof({f_name}_buf));\n'
            f'  JsonDocument {f_name}_doc;\n'
            f'  deserializeMsgPack({f_name}_doc, {f_name}_buf, {f_name}_len);\n'
            f'  {doc_var}["{f_name}"] = {f_name}_doc;')

def generate_cpp_field_deserialization(f_type, f_name, doc_var, msg_var):
    if f_type in TYPE_MAP_CPP:
        return f'{msg_var}.{f_name} = {doc_var}["{f_name}"].as<{get_cpp_type(f_type)}>();'
        
    if 'Vector3' in f_type or 'Point' in f_type:
        return (f'  {msg_var}.{f_name}.x = {doc_var}["{f_name}"][0].as<double>();\n'
                f'  {msg_var}.{f_name}.y = {doc_var}["{f_name}"][1].as<double>();\n'
                f'  {msg_var}.{f_name}.z = {doc_var}["{f_name}"][2].as<double>();')
                
    if 'Quaternion' in f_type:
        return (f'  {msg_var}.{f_name}.x = {doc_var}["{f_name}"][0].as<double>();\n'
                f'  {msg_var}.{f_name}.y = {doc_var}["{f_name}"][1].as<double>();\n'
                f'  {msg_var}.{f_name}.z = {doc_var}["{f_name}"][2].as<double>();\n'
                f'  {msg_var}.{f_name}.w = {doc_var}["{f_name}"][3].as<double>();')
                
    if 'Twist' in f_type:
        return (f'  {msg_var}.{f_name}.linear.x = {doc_var}["{f_name}"][0][0].as<double>();\n'
                f'  {msg_var}.{f_name}.linear.y = {doc_var}["{f_name}"][0][1].as<double>();\n'
                f'  {msg_var}.{f_name}.linear.z = {doc_var}["{f_name}"][0][2].as<double>();\n'
                f'  {msg_var}.{f_name}.angular.x = {doc_var}["{f_name}"][1][0].as<double>();\n'
                f'  {msg_var}.{f_name}.angular.y = {doc_var}["{f_name}"][1][1].as<double>();\n'
                f'  {msg_var}.{f_name}.angular.z = {doc_var}["{f_name}"][1][2].as<double>();')
                
    # Custom nested struct deserialization
    return (f'  uint8_t {f_name}_buf[256];\n'
            f'  size_t {f_name}_len = serializeMsgPack({doc_var}["{f_name}"], {f_name}_buf, sizeof({f_name}_buf));\n'
            f'  deserialize_msg({f_name}_buf, {f_name}_len, {msg_var}.{f_name});')

# PC C++ Serialization/Deserialization using nlohmann/json
def generate_pc_cpp_field_serialization(f_type, f_name, json_var, msg_var):
    if f_type in TYPE_MAP_CPP:
        return f'{json_var}["{f_name}"] = {msg_var}.{f_name};'
        
    if 'Vector3' in f_type or 'Point' in f_type:
        return f'{json_var}["{f_name}"] = {{{msg_var}.{f_name}.x, {msg_var}.{f_name}.y, {msg_var}.{f_name}.z}};'
        
    if 'Quaternion' in f_type:
        return f'{json_var}["{f_name}"] = {{{msg_var}.{f_name}.x, {msg_var}.{f_name}.y, {msg_var}.{f_name}.z, {msg_var}.{f_name}.w}};'
        
    if 'Twist' in f_type:
        return (f'{json_var}["{f_name}"] = {{\n'
                f'    {{{msg_var}.{f_name}.linear.x, {msg_var}.{f_name}.linear.y, {msg_var}.{f_name}.linear.z}},\n'
                f'    {{{msg_var}.{f_name}.angular.x, {msg_var}.{f_name}.angular.y, {msg_var}.{f_name}.angular.z}}\n'
                f'  }};')
                
    return (f'  std::vector<uint8_t> {f_name}_buf = serialize_msg_pc({msg_var}.{f_name});\n'
            f'  {json_var}["{f_name}"] = nlohmann::json::from_msgpack({f_name}_buf);')

def generate_pc_cpp_field_deserialization(f_type, f_name, json_var, msg_var):
    if f_type in TYPE_MAP_CPP:
        return f'{msg_var}.{f_name} = {json_var}["{f_name}"].get<{get_cpp_type(f_type)}>();'
        
    if 'Vector3' in f_type or 'Point' in f_type:
        return (f'  {msg_var}.{f_name}.x = {json_var}["{f_name}"][0].get<double>();\n'
                f'  {msg_var}.{f_name}.y = {json_var}["{f_name}"][1].get<double>();\n'
                f'  {msg_var}.{f_name}.z = {json_var}["{f_name}"][2].get<double>();')
                
    if 'Quaternion' in f_type:
        return (f'  {msg_var}.{f_name}.x = {json_var}["{f_name}"][0].get<double>();\n'
                f'  {msg_var}.{f_name}.y = {json_var}["{f_name}"][1].get<double>();\n'
                f'  {msg_var}.{f_name}.z = {json_var}["{f_name}"][2].get<double>();\n'
                f'  {msg_var}.{f_name}.w = {json_var}["{f_name}"][3].get<double>();')
                
    if 'Twist' in f_type:
        return (f'  {msg_var}.{f_name}.linear.x = {json_var}["{f_name}"][0][0].get<double>();\n'
                f'  {msg_var}.{f_name}.linear.y = {json_var}["{f_name}"][0][1].get<double>();\n'
                f'  {msg_var}.{f_name}.linear.z = {json_var}["{f_name}"][0][2].get<double>();\n'
                f'  {msg_var}.{f_name}.angular.x = {json_var}["{f_name}"][1][0].get<double>();\n'
                f'  {msg_var}.{f_name}.angular.y = {json_var}["{f_name}"][1][1].get<double>();\n'
                f'  {msg_var}.{f_name}.angular.z = {json_var}["{f_name}"][1][2].get<double>();')
                
    return (f'  std::vector<uint8_t> {f_name}_buf = nlohmann::json::to_msgpack({json_var}["{f_name}"]);\n'
            f'  deserialize_msg_pc({f_name}_buf, {msg_var}.{f_name});')

# Python field serialization helpers
def generate_python_serialize_field(f_type, f_name):
    if f_type in TYPE_MAP_PYTHON:
        return f'"{f_name}": self.{f_name}'
    if 'Vector3' in f_type or 'Point' in f_type:
        return f'"{f_name}": [self.{f_name}.x, self.{f_name}.y, self.{f_name}.z]'
    if 'Quaternion' in f_type:
        return f'"{f_name}": [self.{f_name}.x, self.{f_name}.y, self.{f_name}.z, self.{f_name}.w]'
    if 'Twist' in f_type:
        return (f'"{f_name}": [\n'
                f'                [self.{f_name}.linear.x, self.{f_name}.linear.y, self.{f_name}.linear.z],\n'
                f'                [self.{f_name}.angular.x, self.{f_name}.angular.y, self.{f_name}.angular.z]\n'
                f'            ]')
    return f'"{f_name}": None'

def generate_python_deserialize_field(f_type, f_name):
    if f_type in TYPE_MAP_PYTHON:
        py_type = get_python_type(f_type)
        default_val = get_python_default(f_type)
        return f'{py_type}(data.get(b"{f_name}", data.get("{f_name}", {default_val})))'
        
    if 'Vector3' in f_type or 'Point' in f_type:
        pkg = 'z_geometry_msgs'
        cls_name = 'Vector3' if 'Vector3' in f_type else 'Point'
        return (f'{pkg}.{cls_name}(\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0]))[0]),\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0]))[1]),\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0]))[2])\n'
                f'            )')
                
    if 'Quaternion' in f_type:
        pkg = 'z_geometry_msgs'
        return (f'{pkg}.Quaternion(\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0,1]))[0]),\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0,1]))[1]),\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0,1]))[2]),\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0,1]))[3])\n'
                f'            )')
                
    if 'Twist' in f_type:
        pkg = 'z_geometry_msgs'
        return (f'{pkg}.Twist(\n'
                f'                linear={pkg}.Vector3(\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[0][0]),\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[0][1]),\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[0][2])\n'
                f'                ),\n'
                f'                angular={pkg}.Vector3(\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[1][0]),\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[1][1]),\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[1][2])\n'
                f'                )\n'
                f'            )')
                
    return 'None'

def generate_mcu_header(package, name, fields, is_srv=False, req_fields=None, res_fields=None):
    z_name = f"z_{name}"
    guard = f"CUSTOM_INTERFACE_{package.upper()}_{z_name.upper()}_H"
    
    # Process includes
    all_fields = fields if not is_srv else (req_fields + res_fields)
    includes = get_cpp_includes(all_fields, is_pc=False)
    include_str = '\n'.join(includes)
    if include_str:
        include_str += '\n'
        
    content = f"""#ifndef {guard}
#define {guard}

#include <ArduinoJson.h>
{include_str}
namespace {package} {{
"""
    if not is_srv:
        # Generate topic message struct
        struct_fields = '\n'.join([f"    {get_cpp_type(ftype)} {fname};" for ftype, fname in fields])
        content += f"""struct {z_name} {{
{struct_fields}
}};
}} // namespace {package}

// --- Topic Serialization Override ---
template <>
inline size_t serialize_msg<{package}::{z_name}>(
    const {package}::{z_name}& msg, uint8_t* buffer, size_t max_len) {{
    JsonDocument doc;
"""
        for ftype, fname in fields:
            content += f"    {generate_cpp_field_serialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""    return serializeMsgPack(doc, buffer, max_len);
}}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<{package}::{z_name}>(
    const uint8_t* buffer, size_t len, {package}::{z_name}& msg) {{
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
"""
        for ftype, fname in fields:
            content += f"    {generate_cpp_field_deserialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""}}
"""
    else:
        # Generate service message struct (nested classes)
        req_struct = '\n'.join([f"        {get_cpp_type(ftype)} {fname};" for ftype, fname in req_fields])
        res_struct = '\n'.join([f"        {get_cpp_type(ftype)} {fname};" for ftype, fname in res_fields])
        content += f"""struct {z_name} {{
    struct Request {{
{req_struct}
    }};

    struct Response {{
{res_struct}
    }};
}};
}} // namespace {package}

// --- Service Request Serializer ---
template <>
inline size_t serialize_msg<{package}::{z_name}::Request>(
    const {package}::{z_name}::Request& msg, uint8_t* buffer, size_t max_len) {{
    JsonDocument doc;
"""
        for ftype, fname in req_fields:
            content += f"    {generate_cpp_field_serialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""    return serializeMsgPack(doc, buffer, max_len);
}}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg<{package}::{z_name}::Request>(
    const uint8_t* buffer, size_t len, {package}::{z_name}::Request& msg) {{
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
"""
        for ftype, fname in req_fields:
            content += f"    {generate_cpp_field_deserialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""}}

// --- Service Response Serializer ---
template <>
inline size_t serialize_msg<{package}::{z_name}::Response>(
    const {package}::{z_name}::Response& msg, uint8_t* buffer, size_t max_len) {{
    JsonDocument doc;
"""
        for ftype, fname in res_fields:
            content += f"    {generate_cpp_field_serialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""    return serializeMsgPack(doc, buffer, max_len);
}}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg<{package}::{z_name}::Response>(
    const uint8_t* buffer, size_t len, {package}::{z_name}::Response& msg) {{
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
"""
        for ftype, fname in res_fields:
            content += f"    {generate_cpp_field_deserialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""}}
"""
    content += f"\nusing {z_name} = {package}::{z_name};\n"
    content += f"\n#endif // {guard}\n"
    return content

def generate_pc_header(package, name, fields, is_srv=False, req_fields=None, res_fields=None):
    z_name = f"z_{name}"
    guard = f"CUSTOM_INTERFACE_PC_{package.upper()}_{z_name.upper()}_H"
    
    # Process includes
    all_fields = fields if not is_srv else (req_fields + res_fields)
    includes = get_cpp_includes(all_fields, is_pc=True)
    include_str = '\n'.join(includes)
    if include_str:
        include_str += '\n'
        
    content = f"""#ifndef {guard}
#define {guard}

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
{include_str}
namespace {package} {{
"""
    if not is_srv:
        struct_fields = '\n'.join([f"    {get_cpp_type(ftype)} {fname};" for ftype, fname in fields])
        content += f"""struct {z_name} {{
{struct_fields}
}};
}} // namespace {package}

// --- Topic Serialization Override ---
template <>
inline std::vector<uint8_t> serialize_msg_pc<{package}::{z_name}>(
    const {package}::{z_name}& msg) {{
    nlohmann::json j;
"""
        for ftype, fname in fields:
            content += f"    {generate_pc_cpp_field_serialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""    return nlohmann::json::to_msgpack(j);
}}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg_pc<{package}::{z_name}>(
    const std::vector<uint8_t>& buffer, {package}::{z_name}& msg) {{
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
"""
        for ftype, fname in fields:
            content += f"    {generate_pc_cpp_field_deserialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""}}
"""
    else:
        req_struct = '\n'.join([f"        {get_cpp_type(ftype)} {fname};" for ftype, fname in req_fields])
        res_struct = '\n'.join([f"        {get_cpp_type(ftype)} {fname};" for ftype, fname in res_fields])
        content += f"""struct {z_name} {{
    struct Request {{
{req_struct}
    }};

    struct Response {{
{res_struct}
    }};
}};
}} // namespace {package}

// --- Service Request Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg_pc<{package}::{z_name}::Request>(
    const {package}::{z_name}::Request& msg) {{
    nlohmann::json j;
"""
        for ftype, fname in req_fields:
            content += f"    {generate_pc_cpp_field_serialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""    return nlohmann::json::to_msgpack(j);
}}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg_pc<{package}::{z_name}::Request>(
    const std::vector<uint8_t>& buffer, {package}::{z_name}::Request& msg) {{
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
"""
        for ftype, fname in req_fields:
            content += f"    {generate_pc_cpp_field_deserialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""}}

// --- Service Response Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg_pc<{package}::{z_name}::Response>(
    const {package}::{z_name}::Response& msg) {{
    nlohmann::json j;
"""
        for ftype, fname in res_fields:
            content += f"    {generate_pc_cpp_field_serialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""    return nlohmann::json::to_msgpack(j);
}}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg_pc<{package}::{z_name}::Response>(
    const std::vector<uint8_t>& buffer, {package}::{z_name}::Response& msg) {{
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
"""
        for ftype, fname in res_fields:
            content += f"    {generate_pc_cpp_field_deserialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""}}
"""
    content += f"\nusing {z_name} = {package}::{z_name};\n"
    content += f"\n#endif // {guard}\n"
    return content

def generate_python_module(package, name, fields, is_srv=False, req_fields=None, res_fields=None):
    z_name = f"z_{name}"
    all_fields = fields if not is_srv else (req_fields + res_fields)
    has_geometry = False
    for f_type, _ in all_fields:
        if 'geometry_msgs' in f_type:
            has_geometry = True

    geometry_import = ""
    if has_geometry:
        geometry_import = "from zenoh_ros.msg_interface.pre_defined_interface import z_geometry_msgs\n"

    content = f"""import msgpack
from typing import Any, List, Optional, cast
{geometry_import}
"""
    if not is_srv:
        params = []
        inits = []
        reprs = []
        for ftype, fname in fields:
            py_type = get_python_type(ftype)
            default = get_python_default(ftype)
            params.append(f"{fname}: {py_type} = {default}")
            inits.append(f"        self.{fname} = {fname}")
            reprs.append(f"{fname}={{self.{fname}}}")
            
        param_str = ', '.join(params)
        init_str = '\n'.join(inits)
        repr_str = ', '.join(reprs)
        
        serialize_fields = ', '.join([generate_python_serialize_field(ftype, fname) for ftype, fname in fields])
        deserialize_fields = ', '.join([generate_python_deserialize_field(ftype, fname) for ftype, fname in fields])
        
        content += f"""class {z_name}:
    def __init__(self, {param_str}) -> None:
{init_str}

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({{{serialize_fields}}}))

    @classmethod
    def deserialize(cls, payload: bytes) -> '{z_name}':
        data = msgpack.unpackb(payload)
        # Handle empty/invalid payload
        if not isinstance(data, dict):
            data = {{}}
        return cls({deserialize_fields})

    def __repr__(self) -> str:
        return f"{package}.{z_name}({repr_str})"
"""
    else:
        req_params, req_inits, req_reprs = [], [], []
        for ftype, fname in req_fields:
            py_type = get_python_type(ftype)
            default = get_python_default(ftype)
            req_params.append(f"{fname}: {py_type} = {default}")
            req_inits.append(f"            self.{fname} = {fname}")
            req_reprs.append(f"{fname}={{self.{fname}}}")
            
        res_params, res_inits, res_reprs = [], [], []
        for ftype, fname in res_fields:
            py_type = get_python_type(ftype)
            default = get_python_default(ftype)
            res_params.append(f"{fname}: {py_type} = {default}")
            res_inits.append(f"            self.{fname} = {fname}")
            res_reprs.append(f"{fname}={{self.{fname}}}")
            
        req_serialize = ', '.join([generate_python_serialize_field(ftype, fname) for ftype, fname in req_fields])
        req_deserialize = ', '.join([generate_python_deserialize_field(ftype, fname) for ftype, fname in req_fields])
        
        res_serialize = ', '.join([generate_python_serialize_field(ftype, fname) for ftype, fname in res_fields])
        res_deserialize = ', '.join([generate_python_deserialize_field(ftype, fname) for ftype, fname in res_fields])

        content += f"""class {z_name}:
    class Request:
        def __init__(self, {', '.join(req_params)}) -> None:
{'\n'.join(req_inits)}

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({{{req_serialize}}}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Request':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {{}}
            return cls({req_deserialize})

        def __repr__(self) -> str:
            return f"{package}.{z_name}.Request({', '.join(req_reprs)})"

    class Response:
        def __init__(self, {', '.join(res_params)}) -> None:
{'\n'.join(res_inits)}

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({{{res_serialize}}}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Response':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {{}}
            return cls({res_deserialize})

        def __repr__(self) -> str:
            return f"{package}.{z_name}.Response({', '.join(res_reprs)})"
"""
    return content

def main():
    parser = argparse.ArgumentParser(description="zenoh_ros Interface Generator Script")
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('-generate', type=str, help="Generate target message/service files")
    group.add_argument('-rm', type=str, help="Remove generated message/service files")
    
    args = parser.parse_args()
    
    relative_path = args.generate if args.generate else args.rm
    action = 'generate' if args.generate else 'rm'
    
    # Locate paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    ws_root = os.path.abspath(os.path.join(script_dir, "../../../"))
    
    # Resolve the input file path relative to msg_interface folder
    input_file = os.path.abspath(os.path.join(script_dir, "../", relative_path))
    
    # Determine type, package, and name
    # e.g., srv/custom_srvs/SetColor.srv -> type=srv, package=custom_srvs, name=SetColor
    parts = relative_path.replace('\\', '/').split('/')
    if len(parts) < 2:
        print(f"Error: Invalid relative path layout {relative_path}. Expects '<msg|srv>/[package/]<filename>'")
        sys.exit(1)
        
    if parts[0] not in ['msg', 'srv']:
        print(f"Error: Path must start with 'msg/' or 'srv/'")
        sys.exit(1)
        
    interface_type = parts[0]
    filename = parts[-1]
    name, ext = os.path.splitext(filename)
    
    # Default package namespace if subdirectory is missing
    if len(parts) == 2:
        package = "custom_msgs" if interface_type == 'msg' else "custom_srvs"
    else:
        package = parts[1]
        
    # Generate relative output destinations
    out_dir_mcu = os.path.join(ws_root, f"shared_libraries/mcu/zenoh_ros/zenoh_ros/{package}")
    out_dir_cpp = os.path.join(ws_root, f"shared_libraries/cpp/zenoh_ros/zenoh_ros/{package}")
    out_dir_py = os.path.join(ws_root, f"shared_libraries/python/zenoh_ros/msg_interface/custom_interface/{interface_type}s/{package}")
    
    mcu_file = os.path.join(out_dir_mcu, f"z_{name}.h")
    cpp_file = os.path.join(out_dir_cpp, f"z_{name}.h")
    py_file = os.path.join(out_dir_py, f"{name}.py")
    
    if action == 'rm':
        print(f"Removing generated files for {package}/{name}...")
        for f in [mcu_file, cpp_file, py_file]:
            if os.path.exists(f):
                os.remove(f)
                print(f"  Deleted: {f}")
            else:
                print(f"  Not found: {f}")
        # Clean up directories if they become empty
        for d in [out_dir_mcu, out_dir_cpp, out_dir_py]:
            if os.path.exists(d) and not os.listdir(d):
                os.rmdir(d)
                print(f"  Removed empty directory: {d}")
        sys.exit(0)
        
    # Generate action
    if not os.path.exists(input_file):
        print(f"Error: Input definition file not found: {input_file}")
        sys.exit(1)
        
    print(f"Reading definition from: {input_file}")
    with open(input_file, 'r') as f:
        content_lines = f.readlines()
        
    # Parse fields
    is_srv = (ext == '.srv')
    fields = []
    req_fields = []
    res_fields = []
    
    if not is_srv:
        fields = parse_fields(content_lines)
    else:
        # Split by divider '---'
        req_lines = []
        res_lines = []
        target = req_lines
        for line in content_lines:
            if line.strip() == '---':
                target = res_lines
                continue
            target.append(line)
        req_fields = parse_fields(req_lines)
        res_fields = parse_fields(res_lines)
        
    # Ensure folders exist
    for d in [out_dir_mcu, out_dir_cpp, out_dir_py]:
        os.makedirs(d, exist_ok=True)
        # Create empty __init__.py inside Python generated package directory
        if d == out_dir_py:
            init_file = os.path.join(d, "__init__.py")
            if not os.path.exists(init_file):
                with open(init_file, 'w') as init_f:
                    init_f.write("")
                    
    # Generate content
    mcu_code = generate_mcu_header(package, name, fields, is_srv, req_fields, res_fields)
    cpp_code = generate_pc_header(package, name, fields, is_srv, req_fields, res_fields)
    py_code = generate_python_module(package, name, fields, is_srv, req_fields, res_fields)
    
    # Save files (overwriting if exists)
    with open(mcu_file, 'w') as f:
        f.write(mcu_code)
    with open(cpp_file, 'w') as f:
        f.write(cpp_code)
    with open(py_file, 'w') as f:
        f.write(py_code)
        
    print(f"Successfully generated interface files for {package}/{name}:")
    print(f"  MCU: {mcu_file}")
    print(f"  CPP: {cpp_file}")
    print(f"  Python: {py_file}")

if __name__ == "__main__":
    main()
