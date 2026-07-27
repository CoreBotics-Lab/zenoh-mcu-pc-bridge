# Zenoh PC Nodes (Zenoh-MCU Workbench)

This directory contains the PC-side integration tools for the **MCU-ROS 2 Integration Workbench**, utilizing **Zenoh** for low-latency, real-time communication between an ESP32-S3 microcontroller and a PC (running ROS 2).

---

## 📁 Directory Structure

```text
zenoh_pc_nodes/
├── cpp/                   # C++ Zenoh implementation
│   ├── CMakeLists.txt     # CMake build configuration
│   ├── include/           # Header files (ZenohNode wrappers, QoS settings)
│   └── src/               # Source files (subscriber implementations)
├── python/                # Python Zenoh implementation
│   ├── ros2_zenoh/        # Python module wrappers for Zenoh
│   ├── requirements.txt   # Pip dependencies
│   ├── zenoh_ros2_bridge.py # ROS 2 <-> Zenoh bridge node
│   └── zenoh_test_sub.py  # Standalone Python subscriber
└── .gitignore             # Git ignore file for build & Python cache
```

---

## 🛠️ C++ Subscriber Project

The C++ project uses a wrapper pattern around `zenoh-c` (`libzenohc`) to simplify subscriber declarations and callbacks.

### 📋 Prerequisites
Ensure you have the Zenoh-C library installed on your system:
*   [zenoh-c](https://github.com/eclipse-zenoh/zenoh-c)

### 🔨 Building with CMake
To build the C++ subscriber node:
```bash
cd cpp
mkdir -p build && cd build
cmake ..
make
```

### 🚀 Running
After building, run the compiled binary:
```bash
./zenoh_test_sub
```

---

## 🐍 Python Nodes

The Python implementations use a helper wrapper module (`ros2_zenoh`) for rapid prototyping of Zenoh nodes.

### 📋 Prerequisites
Install python requirements using pip:
```bash
cd python
pip install -r requirements.txt
```

### 🚀 Running the Standalone Python Subscriber
```bash
python3 python/zenoh_test_sub.py
```

### 🚀 Running the ROS 2 Bridge Node
To bridge incoming Zenoh data (e.g., from the ESP32) into a native ROS 2 topic:
```bash
# Source your ROS 2 workspace first (Jazzy)
source /opt/ros/jazzy/setup.bash
python3 python/zenoh_ros2_bridge.py
```
