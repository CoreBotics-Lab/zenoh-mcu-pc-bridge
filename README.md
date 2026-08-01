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

## 🛠️ API Reference & Methods

All communication methods belong to `ZenohNode` and follow standard ROS 2 method signatures across C++ and Python.

---

### 1. Initializing Zenoh Session (`ZenohNode::init`)

Initialize the network session before instantiating any node:

- **MCU C++**:
  ```cpp
  ZenohConfig cfg = { .ssid = "ESP32_AP", .password = "12345678", .port = 7447 };
  ZenohNode::init(cfg);
  ```
- **PC C++**:
  ```cpp
  ZenohConfig config;
  config.host = "192.168.4.1"; // ESP32 IP
  ZenohNode::init(config);
  ```
- **PC Python**:
  ```python
  ZenohNode.init(ZenohConfig(host="192.168.4.1"))
  ```

---

### 2. Creating a Publisher (`z_create_publisher`)

Creates a typed publisher on a specific topic.

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

Subscribes to a topic and triggers a callback when data arrives.

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

Schedules a recurring wall timer callback.

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

Registers a service server to handle incoming requests and return a response.

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

Sends requests to a service server and waits for the response.

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

Severity levels: `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`.

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

## 🛠️ Interface Generator (`generate.py`)

Place `.msg` or `.srv` files in `shared_libraries/msg_interface/` and run:

```bash
cd shared_libraries/msg_interface/generate_interface

# Generate custom interfaces across Python, MCU C++, and PC C++
python3 generate.py -generate custom
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
