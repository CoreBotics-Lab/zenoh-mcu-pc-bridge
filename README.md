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

## 📁 Shared Library Directory Structure

The `shared_libraries/` folder is the **Single Source of Truth** for the whole library:

```text
shared_libraries/
├── mcu/zenoh_ros/                     ← MCU C++ Library (ESP32 / Arduino)
├── cpp/zenoh_ros/                     ← PC C++ Library (Header-only + 3rdparty prebuilts)
├── python/zenoh_ros/                  ← PC Python Package
├── tools/                             ← Standalone C++ Echo Debugger Tool (tools/build/echo)
└── msg_interface/                     ← Interface Generator System (.msg, .srv & generate.py)
```

---

## 🛠️ 1. Interface Generator System (`generate.py`)

The generator system converts standard ROS 2 `.msg` and `.srv` definition files into typed C++ structs and Python modules across MCU C++, PC C++, and Python.

### **1.1 Defining Messages & Services**

Place your definition files under `shared_libraries/msg_interface/`:
- **Messages**: `shared_libraries/msg_interface/msg/custom_msgs/`
- **Services**: `shared_libraries/msg_interface/srv/custom_srvs/`

#### **Creating a Custom Message (`SetLED.msg`)**
```text
uint8 led_num
uint8 r
uint8 g
uint8 b
uint8 brightness
```

#### **Creating a Custom Service (`SetLEDColor.srv`)**
Services use `---` to separate Request from Response:
```text
# Request (supports nested interfaces!)
custom_msgs/SetLED led_data
---
# Response
bool success
string message
```

#### **Creating a Custom Nested Message (`RobotState.msg`)**
You can nest pre-defined interface types (`std_msgs`, `geometry_msgs`, `sensor_msgs`, etc.) or other custom messages:
```text
std_msgs/Header header
custom_msgs/MPU6050Data imu_data
string status_message
```

---

### **1.2 Running `generate.py`**

Navigate to `shared_libraries/msg_interface/generate_interface` and run:

```bash
cd shared_libraries/msg_interface/generate_interface

# 1. Generate single message/service binding:
python3 generate.py -generate msg/custom_msgs/SetLED.msg
python3 generate.py -generate srv/custom_srvs/SetLEDColor.srv

# 2. Generate ALL custom messages and services in batch:
python3 generate.py -generate custom

# 3. Remove generated interface bindings:
python3 generate.py -rm msg/custom_msgs/SetLED.msg
python3 generate.py -rm srv/custom_srvs/SetLEDColor.srv
```

---

### **1.3 Clean Include Rules Across Platforms**

Once generated, all message and service types follow standard ROS 2 include conventions across MCU, PC C++, and Python:

| Platform | C++ Header / Python Import Path |
|---|---|
| **MCU C++** | `#include <zenoh_ros/custom_msgs/z_SetLED.h>`<br>`#include <zenoh_ros/custom_srvs/z_SetLEDColor.h>` |
| **PC C++** | `#include <zenoh_ros/custom_msgs/z_SetLED.h>`<br>`#include <zenoh_ros/custom_srvs/z_SetLEDColor.h>` |
| **PC Python** | `from zenoh_ros.custom_msgs import z_SetLED`<br>`from zenoh_ros.custom_srvs import z_SetLEDColor` |
| **Pre-defined** | `#include <zenoh_ros/std_msgs/z_Int32.h>`<br>`#include <zenoh_ros/sensor_msgs/z_Imu.h>` |

---

## 📡 2. Topics (Publish / Subscribe)

### **2.1 Publisher API**

- **MCU C++**:
  ```cpp
  #include <zenoh_ros/ZenohRos.h>
  #include <zenoh_ros/std_msgs/z_Int32.h>

  ZenohPublisher<z_Int32>* pub = node->z_create_publisher<z_Int32>("my_topic", 10);
  z_Int32 msg;
  msg.data = 42;
  pub->publish(msg);
  ```

- **PC C++**:
  ```cpp
  #include <zenoh_ros/ZenohRosPC.h>
  #include <zenoh_ros/std_msgs/z_Int32.h>

  ZenohPublisher<z_Int32>* pub = node->z_create_publisher<z_Int32>("my_topic", 10);
  z_Int32 msg;
  msg.data = 42;
  pub->publish(msg);
  ```

- **Python**:
  ```python
  from zenoh_ros import ZenohNode
  from zenoh_ros.std_msgs import z_Int32

  pub = node.z_create_publisher(z_Int32, "my_topic", 10)
  msg = z_Int32()
  msg.data = 42
  pub.publish(msg)
  ```

---

### **2.2 Subscription API**

- **MCU C++**:
  ```cpp
  auto sub = node->z_create_subscription<z_Int32>(
      "my_topic",
      [](const z_Int32& msg) {
          Serial.printf("Recv: %d\n", msg.data);
      },
      10
  );
  ```

- **PC C++**:
  ```cpp
  auto sub = node->z_create_subscription<z_Int32>(
      "my_topic",
      [](const z_Int32& msg) {
          std::cout << "Recv: " << msg.data << std::endl;
      },
      10
  );
  ```

- **Python**:
  ```python
  def cb(msg: z_Int32):
      print(f"Recv: {msg.data}")

  sub = node.z_create_subscription(z_Int32, "my_topic", cb, 10)
  ```

---

## 🔄 3. Services (Client & Server)

Services use a non-blocking Request/Response architecture.

### **3.1 Service Server API**

- **MCU C++**:
  ```cpp
  #include <zenoh_ros/custom_srvs/z_SetLEDColor.h>

  auto srv = node->z_create_service<z_SetLEDColor>(
      "set_led_color",
      [](const z_SetLEDColor::Request& req, z_SetLEDColor::Response& res) {
          res.success = true;
          res.message = "LED updated";
      }
  );
  ```

- **Python Server**:
  ```python
  from zenoh_ros.custom_srvs import z_SetLEDColor

  def handle_srv(req, res):
      res.success = True
      res.message = "OK"

  srv = node.z_create_service(z_SetLEDColor, "set_led_color", handle_srv)
  ```

---

### **3.2 Service Client API**

- **Python Client (Async with Timeout)**:
  ```python
  client = node.z_create_client(z_SetLEDColor, "set_led_color")

  if client.wait_for_service(timeout_sec=5.0):
      req = z_SetLEDColor.Request()
      req.led_data.led_num = 0
      req.led_data.r = 255

      future = client.call_async(req)
      try:
          res = future.result(timeout_sec=3.0)
          print(f"Result: success={res.success}, msg='{res.message}'")
      except TimeoutError:
          print("Service request timed out!")
  ```

- **MCU C++ Client**:
  ```cpp
  ZenohClient<z_SetLEDColor>* client = node->z_create_client<z_SetLEDColor>("set_led_color");
  if (client->wait_for_service(5000)) {
      z_SetLEDColor::Request req;
      z_SetLEDColor::Response res;
      if (client->call(req, res, 5000)) {
          // Response received
      }
  }
  ```

---

## 🪵 4. Logging System

`zenoh_ros` provides ROS 2-style logging with severity levels (`DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`) and rate-limiting across all platforms. Every node also streams its logs over `{node_name}/log`.

### **4.1 MCU C++ Logging (`z_logger.h`)**

```cpp
#include <zenoh_ros/z_logger.h>

ZLogger logger("my_mcu_node");
logger.z_attach(this); // Stream logs live over 'my_mcu_node/log'

ZLOG_INFO(logger, "Node started");
ZLOG_WARN_ONCE(logger, "Init warning printed once");
ZLOG_DEBUG_THROTTLE(logger, 2000, "Throttled tick every 2000ms: %d", val);
ZLOG_ERROR_EXPRESSION(logger, temp > 80.0, "High temp: %.1f", temp);
```

---

### **4.2 PC C++ Logging (`z_logger.h`)**

```cpp
#include <zenoh_ros/z_logger.h>

auto logger = z_get_logger("my_pc_node");

ZLOG_INFO(logger, "PC node running");
ZLOG_WARN_ONCE(logger, "Warn once");
ZLOG_ERROR_THROTTLE(logger, 1000, "Error throttle 1s");
```

---

### **4.3 Python Logging (`zenoh_ros.logging`)**

```python
from zenoh_ros import get_logger, LogLevel

logger = get_logger("my_python_node")

logger.info("Python node running")
logger.warn("Low battery: %.1f", 3.1, throttle_duration_sec=2.0)
logger.error("Sensor error", once=True)
```

---

## 🔍 5. Real-Time CLI Debugger (`echo`)

`shared_libraries/tools/build/echo` is a high-speed compiled C++ CLI tool for real-time topic inspection and live log monitoring.

### **Building `echo`**:
```bash
cd shared_libraries/tools
mkdir -p build && cd build
cmake .. && make echo
```

### **Usage**:
```bash
# 1. Inspect live node log stream (auto-discovers IP 192.168.4.1)
shared_libraries/tools/build/echo ws2812b_service_server/log

# 2. Inspect custom sensor topic
shared_libraries/tools/build/echo robot/mpu6050

# 3. Inspect topic on custom target IP (e.g. Wi-Fi STA mode)
shared_libraries/tools/build/echo robot/mpu6050 192.168.1.100
```

---

## 💡 6. Code Examples Reference

For complete, runnable node implementations, refer directly to the reference projects in the workspace:

- **MCU Firmware Example**: [`mcu_firmware_examples/RGB_ZenohRosSrv/src/main.cpp`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/mcu_firmware_examples/RGB_ZenohRosSrv/src/main.cpp)
- **MCU MPU6050 Publisher**: [`mcu_firmware_examples/mpu6050_ZenohPub/src/main.cpp`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/mcu_firmware_examples/mpu6050_ZenohPub/src/main.cpp)
- **PC Python LED Service Client**: [`zenoh_pc_nodes/python/led_service_client.py`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/zenoh_pc_nodes/python/led_service_client.py)
- **PC Python MPU6050 Subscriber**: [`zenoh_pc_nodes/python/mpu6050_sub.py`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/zenoh_pc_nodes/python/mpu6050_sub.py)
- **PC C++ Subscriber**: [`zenoh_pc_nodes/cpp/src/mpu6050_sub.cpp`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/zenoh_pc_nodes/cpp/src/mpu6050_sub.cpp)
