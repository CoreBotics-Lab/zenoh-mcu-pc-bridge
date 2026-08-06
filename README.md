# `zenoh_ros` — Lightweight ROS 2 Communication Library

`zenoh_ros` is a lightweight, real-time, low-latency ROS 2-style communication wrapper for **Microcontrollers (ESP32/Arduino)** and **Host PC (C++ and Python)** operating over **Eclipse Zenoh**.

The entire library lives inside a single portable folder: **`shared_libraries/`**. Copy this folder into any new project workspace to use `zenoh_ros`.

---

> [!NOTE]
> **Disclaimer & Upstream Acknowledgements**
> This library (`zenoh_ros`) is a wrapper interface built directly on top of the following open-source projects:
> - **[Eclipse Zenoh](https://zenoh.io/) & [Zenoh C-SDK](https://github.com/eclipse-zenoh/zenoh-c)** (Eclipse Public License 2.0 / Apache License 2.0)
> - **[Eclipse Zenoh-Pico](https://github.com/eclipse-zenoh/zenoh-pico)** (Eclipse Public License 2.0 / Apache License 2.0)
> - **[ArduinoJson](https://arduinojson.org/) by Benoit Blanchon** (MIT License)
> - **[nlohmann/json](https://github.com/nlohmann/json)** (MIT License)
> - **[msgpack-python](https://github.com/msgpack/msgpack-python)** (Apache License 2.0)

---

## 📁 Shared Library Directory Structure

```text
shared_libraries/
├── mcu/zenoh_ros/         ← Microcontroller C++ Library (ESP32 / Arduino / ESP-IDF)
├── cpp/zenoh_ros/         ← Host PC C++ Library (C++17)
├── python/zenoh_ros/      ← Host PC Python Package (Python 3.x)
├── tools/                 ← Standalone C++ CLI Echo Debugger (tools/build/echo)
└── msg_interface/         ← Interface Generator (.msg, .srv & generate.py)
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
custom_msgs/SetLED led_state   # References SetLED.msg
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

## 📡 API Reference & Core Features

All communication methods belong to `ZenohNode` and follow standard ROS 2 method signatures across C++ and Python.

---

### 1. Initializing Zenoh Session (`ZenohNode::init`)

- **MCU C++ (Wi-Fi SoftAP / STA)**:
  ```cpp
  ZenohConfig cfg = {
      .communication_mode = ZenohCommunicationMode::ZENOH_COMM_WIFI,
      .ssid               = "ESP32_AP",
      .password           = "12345678",
      .port               = 7447,
      .wifi_mode          = WIFI_AP
  };
  ZenohNode::init(cfg);
  ```
- **MCU C++ (USB CDC / High-Speed Serial)**:
  ```cpp
  ZenohConfig cfg = {
      .communication_mode = ZenohCommunicationMode::ZENOH_COMM_UART_USB_CDC,
      .baudrate           = (uint32_t)ZenohBaudRate::USB_HIGH_SPEED
  };
  ZenohNode::init(cfg);
  ```
- **PC C++**:
  ```cpp
  ZenohConfig config;
  config.communication_mode = ZenohCommunicationMode::ZENOH_COMM_WIFI;
  config.host = "192.168.4.1"; // ESP32 SoftAP IP
  config.port = 7447;
  ZenohNode::init(config);
  ```
- **PC Python**:
  ```python
  config = ZenohConfig(
      communication_mode=ZenohCommunicationMode.ZENOH_COMM_WIFI,
      host="192.168.4.1",
      port=7447
  )
  ZenohNode.init(config)
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
      [this](const z_Int32& msg) {
          ZLOG_INFO(this->get_logger(), "Received: %d", msg.data);
      },
      10
  );
  ```
- **Python**:
  ```python
  def callback(msg: z_Int32):
      node.get_logger().info(f"Received: {msg.data}")

  sub = node.z_create_subscription(z_Int32, "topic_name", callback, 10)
  ```

---

### 4. Creating a Timer (`z_create_timer`)

- **C++ (MCU & PC)**:
  ```cpp
  ZenohTimer* timer = node->z_create_timer(1000, [this]() {
      // Triggered every 1000ms (1 second)
  });
  ```
- **Python**:
  ```python
  timer = node.z_create_timer(1000, timer_callback) # Triggered every 1000ms (1 second)
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

- **Python Client (Async / Future Pattern)**:
  ```python
  client = node.z_create_client(z_SetLEDColor, "set_led_color")

  if client.wait_for_service(timeout_sec=5.0):
      req = z_SetLEDColor.Request()
      req.led_data.led_num = 0
      
      future = client.call_async(req)
      res = future.result(timeout_sec=3.0) # Non-blocking Future with timeout
      print(res.success, res.message)
  ```
- **MCU C++ & PC C++ Client**:
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

### 7. ROS 2 Logging Subsystem & Live Zenoh Log Streaming

`zenoh_ros` features a full ROS 2-style logging framework matching `rclcpp` / `rclpy` logger APIs.

- **Standardized Log Format**: Outputs `[INFO] [node_name]: message` with ANSI severity colors.
- **Automatic Node Log Streaming**: `ZenohNode` automatically attaches its built-in logger (`this->get_logger()`) to the Zenoh session. Calling `ZLOG_INFO(this->get_logger(), ...)` or `self.get_logger().info(...)` prints to terminal AND publishes live logs to `{node_name}/log` over Zenoh!

#### **Creating & Attaching Custom Component Loggers (`z_attach`)**:
You can create separate, dedicated loggers for specific hardware components or subsystems (e.g. `hardware`, `bms`, `navigation`) and attach them to your `ZenohNode` using `.z_attach(node)`:

```cpp
class MyRobotNode : public ZenohNode {
public:
    MyRobotNode() : ZenohNode("my_robot_node"), bms_logger_("battery_system") {
        // Attach the separate logger to this node's Zenoh session
        bms_logger_.z_attach(this);

        // Logs to terminal AND streams live over Zenoh to topic 'battery_system/log'
        ZLOG_INFO(bms_logger_, "BMS Subsystem Initialized. Voltage: 24.2V");
    }

private:
    ZLogger bms_logger_;
};
```

> [!NOTE]
> Unattached loggers (such as `z_get_logger("system")` called before `ZenohNode` instantiation) print cleanly to hardware `Serial`, but do not stream over Zenoh until `.z_attach(node)` is called.

#### **Available C++ Logging Macros (`z_logger.h`)**:
```cpp
ZLOG_DEBUG(node.get_logger(), "Sensor val: %d", val);
ZLOG_INFO(node.get_logger(),  "Node initialized");
ZLOG_WARN(node.get_logger(),  "Voltage low: %.2fV", v);
ZLOG_ERROR(node.get_logger(), "Sensor failed: %s", name);
ZLOG_FATAL(node.get_logger(), "System halted");

// Print once per call site
ZLOG_INFO_ONCE(node.get_logger(), "Initialized");

// Throttle log printing (e.g. at most once every 1000ms)
ZLOG_INFO_THROTTLE(node.get_logger(), 1000, "Loop tick: %d", cnt);
```

#### **Python Logging (`self.get_logger()`)**:
```python
self.get_logger().info("Node started")
self.get_logger().warn(f"Battery at {level}%")
self.get_logger().error(f"Failed to connect: {e}")
```

#### **Echoing Live Node & Component Logs over Zenoh**:
Listen to any node or custom component's live log output over Zenoh without writing subscriber code:
```bash
# Node logs:
shared_libraries/tools/build/echo counter_publisher/log

# Separate attached component logs:
shared_libraries/tools/build/echo battery_system/log
```

---

### 8. ROS 2 Clock & Timestamps (`now()`, `get_clock()`)

Get uniform timestamps (`sec`, `nanosec`) matching ROS 2 across MCU, PC C++, and PC Python:

- **C++ (MCU & PC)**:
  ```cpp
  ZenohTime t1 = node->now();
  ZenohTime t2 = node->get_clock()->now();
  std::cout << "Sec: " << t1.sec << " Nanosec: " << t1.nanosec << "\n";
  ```
- **Python**:
  ```python
  t = node.now()  # or node.get_clock().now()
  print(f"Sec: {t.sec}, Nanosec: {t.nanosec}")
  ```

---

### 9. Parameter Server (`z_declare_parameter`, `z_get_parameter`)

Set and query configuration parameters dynamically without re-compiling firmware:

- **C++ (MCU & PC)**:
  ```cpp
  node->z_declare_parameter("sample_rate", 50);
  node->z_declare_parameter("gain", 1.5f);

  int rate = node->z_get_parameter("sample_rate", 50);
  float gain = node->z_get_parameter("gain", 1.0f);
  ```
- **Python**:
  ```python
  node.z_declare_parameter("sample_rate", 50)
  rate = node.z_get_parameter("sample_rate", 50)
  ```

---

## 🔌 Serial UART & USB CDC Transport Support

In addition to Wi-Fi (SoftAP & STA modes), `zenoh_ros` natively supports direct Serial UART and Native USB CDC communications.

### Transport Modes (`ZenohCommunicationMode`) & Preset Speeds (`ZenohBaudRate`)

| Communication Mode Enum | Description | Preset Baud Rate Enum | Throughput / Bandwidth |
| :--- | :--- | :--- | :--- |
| `ZENOH_COMM_UART_DEFAULT` **(Default)** | Standard UART0 over built-in USB/UART flashing port (`Serial`) | `ZenohBaudRate::UART_STANDARD` (`115200`) | `~11.5 KB/s` (Low-frequency single-topic nodes) |
| `ZENOH_COMM_UART_USB_CDC` | Native USB CDC Serial (`USBSerial` / USB OTG PHY) | `ZenohBaudRate::USB_HIGH_SPEED` (`12000000`) | `~300 KB/s - 1.2 MB/s` (High-rate binary streaming) |
| `ZENOH_COMM_UART_HW` | Hardware UART on custom RX/TX pins (`Serial1`) | `ZenohBaudRate::UART_HIGH_SPEED` (`921600`) | `~92 KB/s` (Multi-topic telemetry) |
| `ZENOH_COMM_WIFI` | Wireless TCP/UDP connection | N/A | `~2.5 MB/s - 5.0 MB/s` |

> [!WARNING]
> **Serial Bandwidth Note**: Streaming 30+ dynamic topics simultaneously at high rates over standard `115,200 baud` UART can saturate the serial bus buffer. For multi-topic nodes, use `UART_HIGH_SPEED` (`921,600 baud`) or `USB_HIGH_SPEED` (`12,000,000 baud`).

---

## ⚡ Performance, Low-Latency & Thread Safety Architecture

1. **Ultra-Low Latency Wi-Fi Mode (`WIFI_PS_NONE`)**:
   - `esp_wifi_set_ps(WIFI_PS_NONE)` automatically locks ESP32 Wi-Fi radio out of modem sleep mode, dropping packet response latency to **~2ms**.

2. **Cross-Platform Thread Safety Mutexes**:
   - **MCU C++**: `ZenohSessionMutex` FreeRTOS mutex protects session buffers from multi-core task crashes when timers (Core 1) and network callbacks (Core 0) run concurrently.
   - **PC C++ & Python**: `ZenohSessionMutexPC` (`std::mutex`) and `ZenohNode._lock` protect session buffers across multi-threaded publish and service calls.

3. **Automatic Node Liveliness Token**:
   - Declares `@ros2/{node_name}/liveliness` tokens automatically upon session initialization for network node discovery.

---

## 🔍 CLI Echo Debugger (`echo`)

Inspect any topic or live log stream in real time:

```bash
# 1. Setup C++ dependencies & build echo tool:
./shared_libraries/cpp/zenoh_install.sh

# 2. Echo topic over Wi-Fi:
shared_libraries/tools/build/echo robot/hello_string 192.168.4.1

# 3. Echo live node logs:
shared_libraries/tools/build/echo counter_publisher/log

# 4. Echo topic over Serial UART:
shared_libraries/tools/build/echo serial/counter /dev/ttyACM0
```

---

## 💡 Reference Example Projects

Check out the complete reference node examples in `mcu_firmware_examples/` and `pc_nodes_examples/`:

- **MCU Server Example**: [`mcu_firmware_examples/RGB_ZenohRosSrv/src/main.cpp`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/mcu_firmware_examples/RGB_ZenohRosSrv/src/main.cpp)
- **MCU Multi-Topic Publisher**: [`mcu_firmware_examples/publishMultipleTopics/src/main.cpp`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/mcu_firmware_examples/publishMultipleTopics/src/main.cpp)
- **MCU IMU Publisher**: [`mcu_firmware_examples/mpu6050_ZenohPub/src/main.cpp`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/mcu_firmware_examples/mpu6050_ZenohPub/src/main.cpp)
- **PC Python Client Example**: [`pc_nodes_examples/python/led_service_client.py`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/pc_nodes_examples/python/led_service_client.py)
- **PC C++ Client Example**: [`pc_nodes_examples/cpp/src/led_service_client.cpp`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/pc_nodes_examples/cpp/src/led_service_client.cpp)
- **PC 3D Flight Simulator (Python)**: [`pc_nodes_examples/python/flight_sim.py`](file:///home/syed-abdul-hayi/Corebotics%20Lab/zenoh_ws/pc_nodes_examples/python/flight_sim.py)
