# Zenoh PC Nodes (Zenoh-MCU Workbench)

This directory contains the PC-side integration tools and runners for testing the **`zenoh_ros`** framework. These nodes receive and process the data sent from the ESP32 microcontroller.

---

## 📁 Directory Structure

```text
zenoh_pc_nodes/
├── cpp/                   # C++ PC Node implementation
│   ├── build/             # Compilation target output
│   ├── src/               # PC subscriber sources
│   │   ├── zenoh_test_sub.cpp # Standard single-topic subscriber
│   │   └── multi_topic_sub.cpp # AP-mode multi-topic subscriber
│   ├── CMakeLists.txt     # CMake configuration linking to shared C++ folder
│   └── zenoh_install.sh   # Automatically configures and builds C++ targets
│
└── python/                # Python PC Node implementation
    ├── requirements.txt   # Pip dependencies
    ├── zenoh_test_sub.py  # Standard single-topic Python subscriber
    └── multiTopicSub.py   # AP-mode multi-topic Python subscriber
```

---

## 🛠️ C++ Subscriber Projects

The C++ project uses a ROS 2-like wrapper pattern around `zenoh-c` defined in `shared_libraries/cpp/zenoh_ros/ZenohRosPC.h`.

### 🔨 Quick Build (Recommended)
Run the setup script which will automatically download `zenoh-c` and `nlohmann_json` to `shared_libraries/cpp/3rdparty/` (if not already downloaded) and build the executable targets:
```bash
./zenoh_install.sh
```

### 🔨 Manual Rebuild
If dependencies are already set up:
```bash
cd cpp
mkdir -p build && cd build
cmake ..
make
```

### 🚀 Running the executable
```bash
./build/zenoh_test_sub
# OR
./build/multi_topic_sub
```

---

## 🐍 Python Subscribers

The Python implementations use the shared package wrapper `shared_libraries/python/zenoh_ros` for rapid prototyping.

### 📋 Prerequisites
Install dependencies using pip:
```bash
cd python
pip install -r requirements.txt
```

### 🚀 Running
```bash
python3 zenoh_test_sub.py
# OR
python3 multiTopicSub.py
```
