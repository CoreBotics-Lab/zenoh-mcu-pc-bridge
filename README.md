# Zenoh-MCU Integration Workspace (`zenoh_ws`)

This workspace contains the integration tools, client libraries, and firmware examples for bridging **Microcontrollers (MCUs)** (running Arduino/C++) with a **Host PC** using **Eclipse Zenoh**. 

It serves as a lightweight, real-time, low-latency, and high-performance alternative to micro-ROS for ROS 2 robotics applications.

---

## 📁 Workspace Directory Structure

```text
zenoh_ws/
├── mcu_firmware_examples/     # Microcontroller firmware and libraries
│   └── simpleCounterWithZenoh/ # PlatformIO project for ESP32-S3 DevKit
│       ├── include/
│       │   └── zenoh_workbench/ # Namespace-isolated MCU C++ helper library
│       │       ├── ZenohWorkbench.h
│       │       ├── z_std_msgs.h
│       │       └── z_geometry_msgs.h
│       ├── src/
│       │   └── main.cpp         # Main ESP32-S3 application
│       └── platformio.ini       # PlatformIO configuration
│
└── zenoh_pc_nodes/            # PC-side client nodes and wrappers
    ├── cpp/                   # C++ PC Node implementation
    │   ├── 3rdparty/          # Locally vendored dependencies (Modular Setup)
    │   │   ├── nlohmann/      # nlohmann/json C++ serialization
    │   │   └── zenoh-c/       # Zenoh C-SDK precompiled headers and libraries
    │   ├── include/           # C++ PC Helper Headers
    │   │   └── ZenohWorkbenchPC.h
    │   ├── src/
    │   │   └── zenoh_test_sub.cpp # Standalone subscriber node
    │   ├── CMakeLists.txt     # CMake configuration
    │   └── zenoh_install.sh   # Direct setup and compilation script
    │
    └── python/                # Python PC Node implementation
        ├── ros2_zenoh/        # Python module wrappers for Zenoh (analogous to rclpy)
        │   ├── node.py
        │   ├── z_std_msgs.py
        │   └── z_geometry_msgs.py
        ├── requirements.txt   # Pip dependencies
        └── zenoh_test_sub.py  # Standalone Python subscriber
```

---

## 🚀 Getting Started

### 1. Microcontroller Firmware Setup (ESP32-S3)

The MCU uses the **`zenoh-pico`** client library (a pure-C, ultra-lightweight Zenoh implementation).

#### **Prerequisites**
* Install [PlatformIO](https://platformio.org/) (either via VS Code extension or command line).

#### **Build & Upload**
1. Navigate to the project directory:
   ```bash
   cd "mcu_firmware_examples/simpleCounterWithZenoh"
   ```
2. Build and upload the firmware to your ESP32-S3:
   ```bash
   platformio run --target upload
   ```
   *(Note: The first run will automatically download the `zenoh-pico` library and compile it locally.)*

3. **How it works:** 
   The ESP32-S3 spins up a SoftAP Wi-Fi access point (configured in `main.cpp` via `ZenohConfig`). It starts a publisher on the topic `robot/sim_counter` sending integer values every 1000ms.

#### 📦 Reusing the MCU Client Library (`zenoh_workbench`)
If you want to use this ROS 2-style C++ library in your own custom microcontroller projects, it is completely modular and self-contained:
1. **Copy the library folder**: Copy the `zenoh_workbench/` directory (located under `mcu_firmware_examples/simpleCounterWithZenoh/include/`) into your new project's `include/` folder.
2. **Add Dependencies**: Add the following library dependencies to your new project's `platformio.ini` file:
   ```ini
   lib_deps = 
       https://github.com/eclipse-zenoh/zenoh-pico.git
       bblanchon/ArduinoJson@^7.1.0
   ```
3. **Include in code**:
   ```cpp
   #include "zenoh_workbench/ZenohWorkbench.h"
   ```

---


### 2. PC-Side Client: C++ Node Setup

The C++ node is built to be **fully modular and self-contained**. It does not require any system-wide packages or dependencies to be pre-installed.

#### **Quick Run (One Command)**
Run the setup script which will automatically download the required architecture-specific headers, libraries (`zenoh-c` and `nlohmann_json`), configure CMake, and build the target:
```bash
./zenoh_pc_nodes/cpp/zenoh_install.sh
```

#### **How to Build Manually**
If you already ran `zenoh_install.sh` once and only want to rebuild:
```bash
cd zenoh_pc_nodes/cpp
mkdir -p build && cd build
cmake ..
make
```

#### **Run the Subscriber**
```bash
./build/zenoh_test_sub
```
This subscriber connects to the Zenoh network, subscribes to the `robot/sim_counter` topic, and prints incoming data sent from the ESP32-S3.

---

### 3. PC-Side Client: Python Node Setup

If you prefer Python, you can run the standalone Python subscriber.

#### **Installation**
1. Navigate to the Python directory:
   ```bash
   cd zenoh_pc_nodes/python
   ```
2. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```

#### **Run the Subscriber**
```bash
python3 zenoh_test_sub.py
```

---

## 🔗 Integrating Zenoh with ROS 2

Because Zenoh handles the low-overhead, wireless transmission between the MCU and PC, you can easily bridge the incoming Zenoh data on the PC side directly into native ROS 2 topics.

### The Gateway Concept
You can create a ROS 2 node on the PC (written in Python using `rclpy` or C++ using `rclcpp`) that:
1. **Subscribes to Zenoh topics** (e.g., subscribing to `robot/sim_counter` via the `ros2_zenoh` client wrapper).
2. **Receives and deserializes the incoming data** (JSON/MessagePack payload).
3. **Publishes the data natively to the local ROS 2 graph** (e.g. publishing to `/sim_counter` topic as a `std_msgs/msg/Int32` message).

This approach isolates the heavy ROS 2 DDS layers on the PC and prevents Wi-Fi dropouts or out-of-memory errors on the microcontroller.
