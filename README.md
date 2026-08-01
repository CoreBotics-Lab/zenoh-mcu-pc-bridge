# `zenoh_ros` — Lightweight ROS 2 Communication Library

`zenoh_ros` is a lightweight, real-time, low-latency ROS 2-style communication wrapper for **Microcontrollers (ESP32/Arduino)** and **Host PC (C++ and Python)** operating over **Eclipse Zenoh**.

The entire library lives inside a single portable folder: **`shared_libraries/`**. Copy this folder into any new project workspace to use `zenoh_ros`.

---

## 📁 Shared Library Directory Structure

```text
shared_libraries/
├── mcu/zenoh_ros/         ← Microcontroller C++ Library (ESP32 / Arduino)
├── cpp/zenoh_ros/         ← PC C++ Library
├── python/zenoh_ros/      ← PC Python Package
├── tools/                 ← Standalone C++ Echo Debugger (tools/build/echo)
└── msg_interface/         ← Message Generator (.msg, .srv & generate.py)
```

---

## 🛠️ Interface Generator (`generate.py`) & How to Write `.msg` / `.srv` Files

The generator system (`shared_libraries/msg_interface/generate_interface/generate.py`) converts standard ROS 2-style `.msg` and `.srv` definition files into cross-platform **MsgPack-serialized** C++ structs and Python classes across MCU C++, PC C++, and Python simultaneously.

---

### 1. How to Write `.msg` Files

Message files define structured data payloads. Place `.msg` files in `shared_libraries/msg_interface/msg/custom_msgs/`.

#### **1.1 Primitive Field Message (`SetLED.msg`)**
Use standard primitive data types (`uint8`, `int32`, `float32`, `string`, `bool`, etc.):
```text
uint8 led_num
uint8 r
uint8 g
uint8 b
uint8 brightness
```

#### **1.2 Primitive Type Mapping**
| `.msg` Type | C++ Type | Python Type |
|---|---|---|
| `int8` / `uint8` | `int8_t` / `uint8_t` | `int` |
| `int16` / `uint16` | `int16_t` / `uint16_t` | `int` |
| `int32` / `uint32` | `int32_t` / `uint32_t` | `int` |
| `int64` / `uint64` | `int64_t` / `uint64_t` | `int` |
| `float32` / `float64` | `float` / `double` | `float` |
| `string` | `std::string` | `str` |
| `bool` | `bool` | `bool` |

---

### 2. How to Write `.srv` Files

Service files define Request/Response pairs separated by three dashes (`---`). Place `.srv` files in `shared_libraries/msg_interface/srv/custom_srvs/`.

#### **2.1 Simple Service Definition (`SetColor.srv`)**
```text
# Request fields
uint8 r
uint8 g
uint8 b
---
# Response fields
bool success
string message
```

#### **2.2 Nested Service Definition (`SetLEDColor.srv`)**
Services can embed custom `.msg` types (e.g. `SetLED`) directly inside the Request or Response:
```text
# Request using nested custom SetLED message
custom_msgs/SetLED led_data
---
# Response
bool success
string message
```

---

### 3. How to Write Nested `.msg` Interfaces

You can nest pre-defined ROS 2 interfaces or other custom `.msg` types into your message definitions.

#### **3.1 Pre-defined Nested ROS 2 Message (`RobotOdom.msg`)**
Embed standard ROS 2 types like `std_msgs/Header`, `geometry_msgs/Twist`, `nav_msgs/Odometry`, or `sensor_msgs/Imu`:
```text
std_msgs/Header header
geometry_msgs/Twist velocity
float32 battery_voltage
```

#### **3.2 Custom Nested Message (`RobotState.msg`)**
Reference another custom message defined in `msg/custom_msgs/`:
```text
SetLED led_state         # References SetLED.msg
string robot_status
```

---

### 4. Running `generate.py`

Navigate to `shared_libraries/msg_interface/generate_interface` and run:

```bash
cd shared_libraries/msg_interface/generate_interface

# 1. Generate single custom message or service:
python3 generate.py -generate msg/custom_msgs/SetLED.msg
python3 generate.py -generate srv/custom_srvs/SetLEDColor.srv

# 2. Batch generate ALL custom messages and services at once:
python3 generate.py -generate custom

# 3. Remove generated interface files:
python3 generate.py -rm msg/custom_msgs/SetLED.msg
python3 generate.py -rm srv/custom_srvs/SetLEDColor.srv
```

#### **Generated Files Location & Include Headers:**

- **MCU C++ Header**: `#include <zenoh_ros/custom_msgs/z_SetLED.h>`
- **PC C++ Header**: `#include <zenoh_ros/custom_msgs/z_SetLED.h>`
- **PC Python Module**: `from zenoh_ros.custom_msgs import z_SetLED`

---

## 📡 API Reference & Core Methods

All communication methods belong to `ZenohNode` and follow standard ROS 2 method signatures across C++ and Python.

---

### 1. Initializing Zenoh Session (`ZenohNode::init`)

- **MCU C++**:
  ```cpp
  ZenohConfig cfg = { .ssid = "ESP32_AP", .password = "12345678", .port = 7447 };
  ZenohNode::init(cfg);
  ```
- **PC C++**:
  ```cpp
  ZenohConfig config;
  config.host = "192.168.4.1"; // ESP32 IP
  config.port = 7447;          // Zenoh default port
  ZenohNode::init(config);
  ```
- **PC Python**:
  ```python
  ZenohNode.init(ZenohConfig(host="192.168.4.1", port=7447))
  ```

---

### 2. Creating a Publisher (`z_create_publisher`)

- **C++ (MCU & PC)**:
  ```cpp
  ZenohPublisher<z_Int32>* pub = node->z_create_publisher<z_Int32>("topic_name", 10);
  
  z_Int32 msg;
  msg.data = 100;
  pub->publish(msg);
  ```
- **Python**:
  ```python
  pub = node.z_create_publisher(z_Int32, "topic_name", 10)
  
  msg = z_Int32()
  msg.data = 100
  pub.publish(msg)
  ```

---

### 3. Creating a Subscription (`z_create_subscription`)

- **C++ (MCU & PC)**:
  ```cpp
  auto sub = node->z_create_subscription<z_Int32>(
      "topic_name",
      [](const z_Int32& msg) {
          // Process message
      },
      10
  );
  ```
- **Python**:
  ```python
  def callback(msg: z_Int32):
      print(msg.data)

  sub = node.z_create_subscription(z_Int32, "topic_name", callback, 10)
  ```

---

### 4. Creating a Timer (`z_create_timer`)

- **MCU C++**:
  ```cpp
  ZenohTimer* timer = node->z_create_timer(1000, []() {
      // Triggered every 1000ms (1 second)
  });
  ```
- **Python**:
  ```python
  timer = node.z_create_timer(1.0, timer_callback) # Triggered every 1.0 second
  ```

---

### 5. Creating a Service Server (`z_create_service`)

- **C++ (MCU & PC)**:
  ```cpp
  auto srv = node->z_create_service<z_SetLEDColor>(
      "set_led_color",
      [](const z_SetLEDColor::Request& req, z_SetLEDColor::Response& res) {
          res.success = true;
          res.message = "OK";
      }
  );
  ```
- **Python**:
  ```python
  def handle_service(req, res):
      res.success = True
      res.message = "OK"

  srv = node.z_create_service(z_SetLEDColor, "set_led_color", handle_service)
  ```

---

### 6. Creating a Service Client (`z_create_client`)

- **Python Client (Async)**:
  ```python
  client = node.z_create_client(z_SetLEDColor, "set_led_color")

  if client.wait_for_service(timeout_sec=5.0):
      req = z_SetLEDColor.Request()
      req.led_data.led_num = 0
      
      future = client.call_async(req)
      res = future.result(timeout_sec=3.0) # Blocking wait with timeout
      print(res.success, res.message)
  ```
- **MCU C++ Client**:
  ```cpp
  auto client = node->z_create_client<z_SetLEDColor>("set_led_color");

  if (client->wait_for_service(5000)) {
      z_SetLEDColor::Request req;
      z_SetLEDColor::Response res;
      if (client->call(req, res, 5000)) {
          // Success
      }
  }
  ```

---

### 7. Logging (`ZLOG` & `ZenohLogger`)

- **MCU C++**:
  ```cpp
  ZLogger logger("mcu_node");
  logger.z_attach(this); // Stream live logs to 'mcu_node/log'

  ZLOG_INFO(logger, "Ready");
  ZLOG_WARN_ONCE(logger, "Warning once");
  ZLOG_DEBUG_THROTTLE(logger, 2000, "Throttled every 2s");
  ```
- **Python**:
  ```python
  from zenoh_ros import get_logger
  logger = get_logger("py_node")

  logger.info("Ready")
  logger.warn("Warning once", once=True)
  logger.error("Throttled", throttle_duration_sec=2.0)
  ```

---

## 🔍 CLI Echo Debugger (`echo`)

Inspect any topic or live log stream in real time:

```bash
# Build tool once:
cd shared_libraries/tools && mkdir -p build && cd build && cmake .. && make echo

# Echo topic:
shared_libraries/tools/build/echo ws2812b_service_server/log
```

---

## 💡 Code Examples

- MCU Server Example: [`mcu_firmware_examples/RGB_ZenohRosSrv/src/main.cpp`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/mcu_firmware_examples/RGB_ZenohRosSrv/src/main.cpp)
- PC Python Client Example: [`zenoh_pc_nodes/python/led_service_client.py`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/zenoh_pc_nodes/python/led_service_client.py)
