# `zenoh_ros` — Lightweight ROS 2 Communication Wrapper for MCUs & Host PC

`zenoh_ros` is a lightweight, real-time, low-latency, and memory-efficient alternative to **micro-ROS** for robotics applications. It bridges Microcontrollers (MCUs) running Arduino/C++ with a Host PC (C++ and Python) over **Eclipse Zenoh**.

The entire framework is self-contained within a single main directory: **`shared_libraries/`**. You can copy this folder into any new project workspace to gain full MCU-to-PC communication, typed message interfaces, ROS 2-style logging, service client/servers, and CLI debugging tools.

---

> [!NOTE]
> **Upstream Acknowledgements**
> `zenoh_ros` is built directly on top of the following open-source projects:
> - **[Eclipse Zenoh](https://zenoh.io/) & [Zenoh C-SDK](https://github.com/eclipse-zenoh/zenoh-c)** (Eclipse Public License 2.0 / Apache License 2.0)
> - **[Eclipse Zenoh-Pico](https://github.com/eclipse-zenoh/zenoh-pico)** (Eclipse Public License 2.0 / Apache License 2.0)
> - **[ArduinoJson](https://arduinojson.org/) by Benoit Blanchon** (MIT License)
> - **[nlohmann/json](https://github.com/nlohmann/json)** (MIT License)
> - **[msgpack-python](https://github.com/msgpack/msgpack-python)** (Apache License 2.0)

---

## 🗂️ Library Directory Architecture

The `shared_libraries/` folder is the **Single Source of Truth** for the whole library:

```text
shared_libraries/
├── mcu/zenoh_ros/                     ← Microcontroller C++ Library (ESP32 / Arduino)
│   ├── zenoh_ros/
│   │   ├── ZenohRos.h                ← Main MCU ZenohNode, Publisher, Subscription, Service, Timer
│   │   ├── z_logger.h                 ← Zero-heap MCU Logger (ZLOG_INFO, ZLOG_WARN_ONCE, etc.)
│   │   └── msg_interface/            ← Pre-defined & Custom Message/Service headers
│   └── library.json                  ← PlatformIO Library Manifest
│
├── cpp/zenoh_ros/                     ← PC C++ Library
│   ├── zenoh_ros/
│   │   ├── ZenohRosPC.h              ← Main PC C++ ZenohNode, Publisher, Subscription, Service
│   │   ├── z_logger.h                 ← C++ Logger (ZLOG_INFO, ZLOG_WARN_THROTTLE, etc.)
│   │   └── msg_interface/            ← Pre-defined & Custom Message/Service headers
│   └── 3rdparty/                     ← Embedded zenoh-c and nlohmann/json headers & prebuilt libs
│
├── python/zenoh_ros/                  ← PC Python Package
│   ├── node.py                        ← ZenohNode, Publisher, Subscription, Service, Client, Future
│   ├── logging/                       ← ROS 2-style Python Logger (ZenohLogger, get_logger)
│   ├── msg_interface/                 ← Pre-defined & Custom Message/Service Python modules
│   └── __init__.py                    ← Exports copy_fields, z_delay, ZenohNode, ZenohLogger
│
├── tools/                             ← High-Performance Standalone CLI Tools
│   ├── CMakeLists.txt                 ← CMake build file for tools
│   └── build/echo                     ← Standalone C++ Echo CLI binary for real-time topic inspection
│
└── msg_interface/                     ← Automatic Generator System
    ├── msg/ & srv/                    ← .msg and .srv source definition files
    └── generate_interface/generate.py ← Code generator for Python, MCU C++, and PC C++
```

---

## ✨ Features Overview

| Feature | Description | Supported Platforms |
|---|---|---|
| **ROS 2 Identical Include Paths** | Clean `#include <zenoh_ros/std_msgs/z_String.h>` & Python `from zenoh_ros.std_msgs import z_String` | MCU, PC C++, Python |
| **Typed Topic Pub/Sub** | High-performance ROS 2-style publishers and subscriptions with custom QoS profiles | MCU, PC C++, Python |
| **Request/Response Services** | Non-blocking Request/Response client and server infrastructure with `Future` promises | MCU, PC C++, Python |
| **ROS 2 Severity Logging** | Severity-based loggers (`DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`) with `ONCE`, `THROTTLE`, and `EXPRESSION` rate-limiting | MCU, PC C++, Python |
| **Per-Node Live Log Streaming** | Automatic streaming of node log events over Zenoh topic `{node_name}/log` | MCU, PC C++, Python |
| **CLI Echo Debugger Tool** | Portable, high-speed C++ topic inspector (`echo`) with ANSI color decoding and IP auto-discovery | Host PC (C++) |
| **Automatic Code Generator** | Converts ROS 2 `.msg` and `.srv` files into typed structs and serialization functions across all 3 platforms | Generator (`generate.py`) |
| **ROS 2 DDS Reflection Bridge** | `copy_fields` utility for seamless bridging between Zenoh MsgPack structs and DDS `rclcpp`/`rclpy` nodes | PC Python |

---

## 🚀 How to Add `shared_libraries` to a New Project

To use `zenoh_ros` in a brand new workspace, copy the `shared_libraries/` folder directly into your workspace root.

---

### 1. Microcontroller Firmware Setup (ESP32-S3 / Arduino)

#### **PlatformIO Configuration (`platformio.ini`)**
Point PlatformIO to your local `shared_libraries/mcu` directory using `lib_extra_dirs`:

```ini
[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200

# Link directly to shared_libraries/mcu
lib_extra_dirs = ../shared_libraries/mcu

lib_deps = 
    zenoh_ros
    https://github.com/eclipse-zenoh/zenoh-pico.git
    bblanchon/ArduinoJson@^7.4.3
```

#### **MCU Code Example (`src/main.cpp`)**

```cpp
#include <Arduino.h>
#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/z_logger.h>
#include <zenoh_ros/std_msgs/z_Int32.h>
#include <zenoh_ros/custom_srvs/z_SetLEDColor.h>

// Wi-Fi Access Point Configuration
ZenohConfig cfg = {
    .ssid = "ESP32S3_Zenoh_AP",
    .password = "zenoh1234",
    .port = 7447
};

class MyNode : public ZenohNode {
public:
    MyNode() : ZenohNode("my_mcu_node"), logger_("my_mcu_node") {
        // Attach logger to node to enable remote live log streaming over '{node_name}/log'
        logger_.z_attach(this);

        ZLOG_INFO(logger_, "MCU Node started successfully.");
        ZLOG_INFO_ONCE(logger_, "Hardware peripheral initialized.");

        // 1. Create Publisher
        pub_ = this->z_create_publisher<std_msgs::z_Int32>("mcu/counter", 10);

        // 2. Create Service Server
        srv_ = this->z_create_service<custom_srvs::z_SetLEDColor>(
            "set_led_color",
            [this](const custom_srvs::z_SetLEDColor::Request& req, custom_srvs::z_SetLEDColor::Response& res) {
                ZLOG_INFO(logger_, "Service request received: LED=%d", req.led_data.led_num);
                res.success = true;
                res.message = "OK";
            }
        );

        // 3. Create Wall Timer (triggers every 1000ms)
        timer_ = this->z_create_timer(1000, [this]() {
            msg_.data++;
            pub_->publish(msg_);
            ZLOG_DEBUG_THROTTLE(logger_, 5000, "Counter published: %d", msg_.data);
        });
    }

private:
    ZLogger logger_;
    ZenohPublisher<std_msgs::z_Int32>* pub_ = nullptr;
    ZenohService<custom_srvs::z_SetLEDColor>* srv_ = nullptr;
    ZenohTimer* timer_ = nullptr;
    std_msgs::z_Int32 msg_;
};

MyNode* node = nullptr;

void setup() {
    Serial.begin(115200);
    z_delay(2000);

    if (ZenohNode::init(cfg)) {
        node = new MyNode();
    }
}

void loop() {
    if (node) {
        node->z_spin();
    }
}
```

---

### 2. Host PC Setup: Python (`shared_libraries/python/`)

#### **Imports & Environment Setup**
Add `shared_libraries/python` to `sys.path` or create a `.pth` link inside your Python virtual environment:

```python
import sys
import os
sys.path.append(os.path.abspath("shared_libraries/python"))

from zenoh_ros import ZenohNode, ZenohConfig, get_logger
from zenoh_ros.std_msgs import z_Int32
from zenoh_ros.custom_srvs import z_SetLEDColor
```

#### **Python Code Example (`my_python_node.py`)**

```python
#!/usr/bin/env python3
import sys
import os
import time
sys.path.append(os.path.abspath("shared_libraries/python"))

from zenoh_ros import ZenohNode, ZenohConfig, get_logger
from zenoh_ros.std_msgs import z_Int32
from zenoh_ros.custom_srvs import z_SetLEDColor

logger = get_logger("my_python_node")

class MyPythonNode(ZenohNode):
    def __init__(self):
        super().__init__("my_python_node")
        logger.info("Python node initialized")

        # 1. Create Subscriber
        self.sub = self.z_create_subscription(
            z_Int32, "mcu/counter", self.counter_callback, 10
        )

        # 2. Create Service Client
        self.client = self.z_create_client(z_SetLEDColor, "set_led_color")

    def counter_callback(self, msg: z_Int32):
        logger.info("Received counter from MCU: %d", msg.data)

def main():
    # Connect to ESP32-S3 SoftAP IP address (defaults to 192.168.4.1:7447)
    config = ZenohConfig(host="192.168.4.1", port=7447)
    ZenohNode.init(config)

    node = MyPythonNode()

    # Call service on MCU
    if node.client.wait_for_service(timeout_sec=5.0):
        req = z_SetLEDColor.Request()
        req.led_data.led_num = 1
        req.led_data.r = 255
        req.led_data.brightness = 60

        future = node.client.call_async(req)
        try:
            res = future.result(timeout_sec=3.0)
            logger.info("Service response: success=%s, msg='%s'", res.success, res.message)
        except TimeoutError:
            logger.error("Service request timed out!")

    try:
        node.z_spin()
    except KeyboardInterrupt:
        logger.warn("Shutdown requested by user")
    finally:
        node.z_destroy()

if __name__ == '__main__':
    main()
```

---

### 3. Host PC Setup: C++ (`shared_libraries/cpp/`)

#### **CMake Configuration (`CMakeLists.txt`)**
Add `shared_libraries/cpp/zenoh_ros` to your target include directories and link against `zenohc` and `nlohmann_json`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(my_pc_cpp_project LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(SHARED_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shared_libraries")

include_directories(SYSTEM "${SHARED_LIB_DIR}/cpp/3rdparty")
include_directories(SYSTEM "${SHARED_LIB_DIR}/cpp/3rdparty/zenoh-c/include")
find_library(ZENOHC_LIB NAMES zenohc PATHS "${SHARED_LIB_DIR}/cpp/3rdparty/zenoh-c/lib" REQUIRED)

add_executable(my_cpp_node src/main.cpp)
target_include_directories(my_cpp_node PRIVATE "${SHARED_LIB_DIR}/cpp/zenoh_ros")
target_link_libraries(my_cpp_node PRIVATE ${ZENOHC_LIB})
```

#### **PC C++ Code Example (`src/main.cpp`)**

```cpp
#include <zenoh_ros/ZenohRosPC.h>
#include <zenoh_ros/z_logger.h>
#include <zenoh_ros/std_msgs/z_Int32.h>
#include <iostream>

class MyPCCppNode : public ZenohNode {
public:
    MyPCCppNode() : ZenohNode("my_pc_cpp_node") {
        logger = z_get_logger("my_pc_cpp_node");
        ZLOG_INFO(logger, "PC C++ Node started");

        sub_ = this->z_create_subscription<std_msgs::z_Int32>(
            "mcu/counter",
            [this](const std_msgs::z_Int32& msg) {
                ZLOG_INFO(logger, "Recv counter: %d", msg.data);
            },
            10
        );
    }

private:
    std::shared_ptr<ZLoggerPC> logger;
    ZenohSubscription<std_msgs::z_Int32>* sub_ = nullptr;
};

int main() {
    ZenohConfig config;
    config.host = "192.168.4.1";
    config.port = 7447;

    if (!ZenohNode::init(config)) {
        return 1;
    }

    MyPCCppNode node;
    node.z_spin();

    ZenohNode::shutdown();
    return 0;
}
```

---

## 🛠️ CLI Topic Debugger: `echo`

`shared_libraries/tools/build/echo` is a high-speed compiled C++ CLI tool for real-time topic inspection and live log monitoring.

### Building `echo` inside `shared_libraries/tools/`:
```bash
cd shared_libraries/tools
mkdir -p build && cd build
cmake .. && make echo
```

### Usage Examples:

```bash
# 1. Echo live MCU log topic (defaults to IP 192.168.4.1)
shared_libraries/tools/build/echo ws2812b_service_server/log

# 2. Echo custom sensor data topic
shared_libraries/tools/build/echo robot/mpu6050

# 3. Echo topic from custom target IP (e.g. Wi-Fi STA mode)
shared_libraries/tools/build/echo robot/mpu6050 192.168.1.100
```

---

## ⚡ Generating Custom Messages & Services (`generate.py`)

The automatic code generator converts standard ROS 2 `.msg` and `.srv` files into typed C++ structs and Python modules across MCU C++, PC C++, and Python.

### **1. Place `.msg` or `.srv` files in `shared_libraries/msg_interface/`**:
- Custom Messages $\rightarrow$ `shared_libraries/msg_interface/msg/custom_msgs/MyMsg.msg`
- Custom Services $\rightarrow$ `shared_libraries/msg_interface/srv/custom_srvs/MySrv.srv`

### **2. Execute `generate.py`**:
```bash
cd shared_libraries/msg_interface/generate_interface

# Generate typed bindings across MCU C++, PC C++, and Python:
python3 generate.py -generate msg/custom_msgs/MyMsg.msg
python3 generate.py -generate srv/custom_srvs/MySrv.srv

# Remove generated bindings:
python3 generate.py -rm msg/custom_msgs/MyMsg.msg
```

### **3. Clean Include Convention**:
After generation, include your new message types anywhere using clean ROS 2-style include rules:

- **C++ (MCU & PC)**:
  ```cpp
  #include <zenoh_ros/custom_msgs/z_MyMsg.h>
  #include <zenoh_ros/custom_srvs/z_MySrv.h>
  ```
- **Python**:
  ```python
  from zenoh_ros.custom_msgs import z_MyMsg
  from zenoh_ros.custom_srvs import z_MySrv
  ```

---

## 🌉 ROS 2 DDS Reflection Bridge (`copy_fields`)

Because `zenoh_ros` utilizes **MsgPack** binary format on MCUs to save memory and CPU cycles, ROS 2 DDS nodes (`rclcpp`/`rclpy`) expecting raw CDR format cannot deserialize MCU data directly.

Use the built-in **`copy_fields`** reflection helper in Python to create a 5-line gateway bridge node:

```python
import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32

from zenoh_ros import ZenohNode, ZenohConfig, copy_fields
from zenoh_ros.std_msgs import z_Int32

class ZenohToRosBridge(Node):
    def __init__(self):
        super().__init__("zenoh_ros_bridge")
        self.ros_pub = self.create_publisher(Int32, "mcu/counter", 10)

        ZenohNode.init(ZenohConfig(host="192.168.4.1", port=7447))
        self.z_node = ZenohNode("bridge_client")
        self.z_sub = self.z_node.z_create_subscription(
            z_Int32, "mcu/counter", self.zenoh_cb, 10
        )

    def zenoh_cb(self, zenoh_msg: z_Int32):
        ros_msg = Int32()
        copy_fields(ros_msg, zenoh_msg)  # Automatically copies matching fields
        self.ros_pub.publish(ros_msg)
```

Now `ros2 topic echo /mcu/counter` works natively in standard ROS 2!
