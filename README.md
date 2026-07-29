# Zenoh-MCU Integration Workspace (`zenoh_ws`)

This workspace contains the integration tools, client libraries, and firmware examples for bridging **Microcontrollers (MCUs)** (running Arduino/C++) with a **Host PC** using **Eclipse Zenoh** through the custom **`zenoh_ros`** wrapper.

It serves as a lightweight, real-time, low-latency, and high-performance alternative to micro-ROS for ROS 2 robotics applications.

> [!NOTE]
> **Disclaimer & Upstream Acknowledgements**
> This library (`zenoh_ros`) is a wrapper interface and is built directly on top of the following open-source projects. We gratefully acknowledge their authors and projects:
> - **[Eclipse Zenoh](https://zenoh.io/) & [Zenoh C-SDK](https://github.com/eclipse-zenoh/zenoh-c)** (Eclipse Public License 2.0 / Apache License 2.0)
> - **[Eclipse Zenoh-Pico](https://github.com/eclipse-zenoh/zenoh-pico)** (Eclipse Public License 2.0 / Apache License 2.0)
> - **[ArduinoJson](https://arduinojson.org/) by Benoit Blanchon** (MIT License)
> - **[nlohmann/json](https://github.com/nlohmann/json)** (MIT License)
> - **[msgpack-python](https://github.com/msgpack/msgpack-python)** (Apache License 2.0)
>
> All copyrights, trademarks, and licenses belong to their respective owners.

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
   *(PlatformIO LDF will automatically link to the shared `zenoh_ros` library in `shared_libraries/mcu`.)*

#### 📦 Reusing the MCU Client Library (`zenoh_ros`)
Because we consolidated our libraries, you can reuse `zenoh_ros` in your own microcontroller projects without copying any source code files:
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
   #include <ZenohRos.h>
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

#### **Run the Subscriber**
```bash
python3 zenoh_test_sub.py
```

---

## 📦 Reusing the PC Client Libraries

### 🐍 Python Client Wrapper (`zenoh_ros`)
The Python package is located under `shared_libraries/python`.
1. Add the path `/path/to/zenoh_ws/shared_libraries/python` to your Python `sys.path`.
2. Import the node classes directly:
   ```python
   from zenoh_ros import ZenohNode, ZenohConfig, z_std_msgs
   ```

### 💻 C++ Client Wrapper (`ZenohRosPC.h`)
The C++ PC library is located under `shared_libraries/cpp`.
1. Include `shared_libraries/cpp/` in your project's include path.
2. Include the header:
   ```cpp
   #include <zenoh_ros/ZenohRosPC.h>
   ```
