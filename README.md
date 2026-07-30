# Zenoh-MCU Integration Workspace (`zenoh_ws`)

This workspace contains integration tools, client libraries, and firmware examples for bridging **Microcontrollers (MCUs)** (running Arduino/C++) with a **Host PC** using **Eclipse Zenoh** through the custom **`zenoh_ros`** wrapper.

It serves as a lightweight, real-time, low-latency, and high-performance alternative to micro-ROS for ROS 2 robotics applications.

> [!NOTE]
> **Disclaimer & Upstream Acknowledgements**
> This library (`zenoh_ros`) is a wrapper interface built directly on top of the following open-source projects:
> - **[Eclipse Zenoh](https://zenoh.io/) & [Zenoh C-SDK](https://github.com/eclipse-zenoh/zenoh-c)** (Eclipse Public License 2.0 / Apache License 2.0)
> - **[Eclipse Zenoh-Pico](https://github.com/eclipse-zenoh/zenoh-pico)** (Eclipse Public License 2.0 / Apache License 2.0)
> - **[ArduinoJson](https://arduinojson.org/) by Benoit Blanchon** (MIT License)
> - **[nlohmann/json](https://github.com/nlohmann/json)** (MIT License)
> - **[msgpack-python](https://github.com/msgpack/msgpack-python)** (Apache License 2.0)

---

## ✨ Features

* **Same C++ API**: Write identical C++ code on both the PC and the MCU.
* **Lightweight Serialization**: Uses **MsgPack** binary format instead of CDR/DDS, saving precious CPU cycles and memory on microcontrollers.
* **Automatic Resource Cleanup**: Node memory management aligns with ROS 2; dynamically created publishers, subscriptions, and timers are automatically tracked and destroyed by the base `ZenohNode` destructor.
* **Independent Client Nodes**: PC C++ nodes compile stand-alone without requiring a local ROS 2 installation.
* **Generic ROS 2 Bridging**: Built-in reflection helpers in Python allow you to map Zenoh nodes to standard ROS 2 DDS nodes instantly.

---

## 📁 Workspace Directory Structure

The workspace is organized into three main directories:
* **`shared_libraries/`**: Centralized wrapper packages for C++ (MCU & PC) and Python targets (Single Source of Truth).
* **`mcu_firmware_examples/`**: PlatformIO firmware projects for microcontrollers (e.g. ESP32-S3).
* **`zenoh_pc_nodes/`**: PC-side subscriber and publisher implementations in C++ and Python.

---

## 🚀 Getting Started

### 1. Microcontroller Firmware Setup (ESP32-S3)

The MCU uses the **`zenoh-pico`** client library (a pure-C, ultra-lightweight Zenoh implementation).

#### **Prerequisites**
* Install [PlatformIO](https://platformio.org/) (either via the VS Code extension or command line).

#### **Build & Upload**
1. Navigate to the project directory:
   ```bash
   cd mcu_firmware_examples/simpleCounterWithZenoh
   ```
2. Build and upload the firmware to your ESP32-S3:
   ```bash
   platformio run --target upload
   ```
   *(PlatformIO's Library Dependency Finder will automatically link to the shared `zenoh_ros` library in `shared_libraries/mcu`.)*

#### 📦 Reusing the MCU Client Library (`zenoh_ros`)
To reuse `zenoh_ros` in your own microcontroller projects without copying any source code files:
1. Open your project's `platformio.ini` and point PlatformIO to the shared folder directory using `lib_extra_dirs`:
   ```ini
   lib_extra_dirs = /path/to/zenoh_ws/shared_libraries/mcu
   ```
2. Include the central library name in your `lib_deps` along with its upstream requirements:
   ```ini
   lib_deps = 
       zenoh_ros
       https://github.com/eclipse-zenoh/zenoh-pico.git
       bblanchon/ArduinoJson@^7.1.0
   ```
3. Include the header in your C++ code:
   ```cpp
   #include <zenoh_ros/ZenohRos.h>
   #include <zenoh_ros/std_msgs/z_Int32.h>
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

#### **Running Examples**
After building, run any of the generated C++ executables in the `build/` folder:
```bash
# Start the standalone subscriber
./build/zenoh_test_sub

# Start the animation publisher for LED arrays
./build/ws2812b_publisher
```

---

### 3. PC-Side Client: Python Node Setup

#### **Installation**
1. Navigate to the Python directory:
   ```bash
   cd zenoh_pc_nodes/python
   ```
2. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```

#### **Running Examples**
Run the python scripts directly:
```bash
# Run the simple subscriber
python3 zenoh_test_sub.py

# Run the animation publisher
python3 ws2812b_publisher.py
```

---

## 🛠️ Generating Custom Messages & Services

To add custom ROS 2 message formats or services, we provide a code generation tool that produces fully-typed C++ headers (for PC and MCU) and Python modules.

### 1. Define Message & Service Files
Place your standard `.msg` or `.srv` definition files in the message interface directory:
* **Messages**: Create files in `shared_libraries/msg_interface/msg/custom_msgs/` (e.g. `MyMessage.msg`)
* **Services**: Create files in `shared_libraries/msg_interface/srv/custom_srvs/` (e.g. `MyService.srv`)

Example file format (`SetLED.msg`):
```text
uint8 led_num
uint8 r
uint8 g
uint8 b
uint8 brightness
```

### 2. Run the Generator
Navigate to the generator directory and run `generate.py`, specifying the relative path of the file starting from `msg/` or `srv/`:
```bash
cd shared_libraries/msg_interface/generate_interface

# To generate bindings
python3 generate.py -generate msg/custom_msgs/SetLED.msg

# To delete generated bindings
python3 generate.py -rm msg/custom_msgs/SetLED.msg
```

### 3. Locations of Generated Bindings
The generator automatically creates the files in the correct directories for all platforms:
* **MCU C++ Header**: `shared_libraries/mcu/zenoh_ros/zenoh_ros/custom_msgs/z_SetLED.h`
* **PC C++ Header**: `shared_libraries/cpp/zenoh_ros/zenoh_ros/custom_msgs/z_SetLED.h`
* **PC Python Module**: `shared_libraries/python/zenoh_ros/msg_interface/custom_interface/msgs/custom_msgs/SetLED.py`

---

## 🔌 Interfacing with Standard ROS 2 (DDS)

Because our MCU client utilizes lightweight **MsgPack** binary format to remain fast, standard tools like `zenoh-bridge-ros2` or `zenoh-bridge-dds` cannot deserialize the data directly out-of-the-box (since standard ROS 2 expects **CDR** bytes).

To bridge your MCU topics natively into standard ROS 2 on your PC:
1. Run a lightweight **Gateway Node** on your PC.
2. Use the built-in **`copy_fields`** utility in our Python library to automatically map fields between the Zenoh MsgPack struct and the standard ROS 2 message without manual parsing.

### Example Python Gateway Node
```python
import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32

from zenoh_ros import ZenohNode, ZenohConfig, copy_fields
from zenoh_ros.std_msgs import z_Int32

class Int32Gateway(Node):
    def __init__(self):
        super().__init__('int32_gateway')
        self.ros_pub = self.create_publisher(Int32, 'robot/sim_counter', 10)
        
        # Connect to Zenoh MCU Client
        ZenohNode.init(ZenohConfig(host="10.42.0.50", port=7447))
        self.zenoh_node = ZenohNode("gateway_client")
        self.zenoh_sub = self.zenoh_node.z_create_subscription(
            z_Int32, "robot/sim_counter", self.zenoh_callback, 10
        )

    def zenoh_callback(self, msg):
        ros_msg = Int32()
        copy_fields(ros_msg, msg)  # Automatically copies msg.data to ros_msg.data
        self.ros_pub.publish(ros_msg)

# ... standard rclpy setup & spin code ...
```
This enables you to run `ros2 topic echo /robot/sim_counter` directly on your PC!
