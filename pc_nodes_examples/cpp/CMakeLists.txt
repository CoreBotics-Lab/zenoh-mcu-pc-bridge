cmake_minimum_required(VERSION 3.10)
project(zenoh_pc_cpp_sub VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define locations for local dependencies
set(LOCAL_3RDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../shared_libraries/cpp/3rdparty")

# 1. Handle nlohmann_json
# Check if local 3rdparty/nlohmann directory exists, include it.
if(EXISTS "${LOCAL_3RDPARTY_DIR}/nlohmann/json.hpp")
    message(STATUS "Using local nlohmann_json header from 3rdparty")
    include_directories(SYSTEM "${LOCAL_3RDPARTY_DIR}")
else()
    # Try system-wide or download on the fly
    find_package(nlohmann_json QUIET)
    if(nlohmann_json_FOUND)
        message(STATUS "Using system nlohmann_json")
    else()
        message(STATUS "nlohmann_json not found locally or on system, downloading via FetchContent...")
        include(FetchContent)
        FetchContent_Declare(
            json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.3
        )
        FetchContent_MakeAvailable(json)
    endif()
endif()

# 2. Handle zenoh-c
if(EXISTS "${LOCAL_3RDPARTY_DIR}/zenoh-c")
    message(STATUS "Using local zenoh-c library from 3rdparty")
    # Include headers from local zenoh-c
    include_directories(SYSTEM "${LOCAL_3RDPARTY_DIR}/zenoh-c/include")
    # Link to local libzenohc
    find_library(ZENOHC_LIB NAMES zenohc PATHS "${LOCAL_3RDPARTY_DIR}/zenoh-c/lib" NO_DEFAULT_PATH REQUIRED)
else()
    # System fallback
    message(STATUS "Local zenoh-c not found, looking for system-wide library")
    find_library(ZENOHC_LIB zenohc REQUIRED)
endif()

# Executable targets
add_executable(zenoh_test_sub src/zenoh_test_sub.cpp)
add_executable(multi_topic_sub src/multi_topic_sub.cpp)
add_executable(ws2812b_publisher src/ws2812b_publisher.cpp)
add_executable(mpu6050_sub src/mpu6050_sub.cpp)
add_executable(led_service_client src/led_service_client.cpp)
add_executable(echo ../../shared_libraries/tools/src/echo.cpp)

# Include directories
target_include_directories(zenoh_test_sub PRIVATE ../../shared_libraries/cpp/zenoh_ros)
target_include_directories(multi_topic_sub PRIVATE ../../shared_libraries/cpp/zenoh_ros)
target_include_directories(ws2812b_publisher PRIVATE ../../shared_libraries/cpp/zenoh_ros)
target_include_directories(mpu6050_sub PRIVATE ../../shared_libraries/cpp/zenoh_ros)
target_include_directories(led_service_client PRIVATE ../../shared_libraries/cpp/zenoh_ros)
target_include_directories(echo PRIVATE ../../shared_libraries/cpp/zenoh_ros)

# Link libraries
target_link_libraries(zenoh_test_sub PRIVATE ${ZENOHC_LIB})
target_link_libraries(multi_topic_sub PRIVATE ${ZENOHC_LIB})
target_link_libraries(ws2812b_publisher PRIVATE ${ZENOHC_LIB})
target_link_libraries(mpu6050_sub PRIVATE ${ZENOHC_LIB})
target_link_libraries(led_service_client PRIVATE ${ZENOHC_LIB})
target_link_libraries(echo PRIVATE ${ZENOHC_LIB})
if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(zenoh_test_sub PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(multi_topic_sub PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(ws2812b_publisher PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(mpu6050_sub PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(led_service_client PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(echo PRIVATE nlohmann_json::nlohmann_json)
endif()

# Add RPATH so the executables can locate the local libzenohc.so at runtime
if(EXISTS "${LOCAL_3RDPARTY_DIR}/zenoh-c")
    if(APPLE)
        set(CMAKE_INSTALL_RPATH "@loader_path/../3rdparty/zenoh-c/lib")
        set_target_properties(zenoh_test_sub PROPERTIES LINK_FLAGS "-Wl,-rpath,@loader_path/../3rdparty/zenoh-c/lib")
        set_target_properties(multi_topic_sub PROPERTIES LINK_FLAGS "-Wl,-rpath,@loader_path/../3rdparty/zenoh-c/lib")
        set_target_properties(ws2812b_publisher PROPERTIES LINK_FLAGS "-Wl,-rpath,@loader_path/../3rdparty/zenoh-c/lib")
        set_target_properties(mpu6050_sub PROPERTIES LINK_FLAGS "-Wl,-rpath,@loader_path/../3rdparty/zenoh-c/lib")
        set_target_properties(led_service_client PROPERTIES LINK_FLAGS "-Wl,-rpath,@loader_path/../3rdparty/zenoh-c/lib")
        set_target_properties(echo PROPERTIES LINK_FLAGS "-Wl,-rpath,@loader_path/../3rdparty/zenoh-c/lib")
    else()
        set(CMAKE_INSTALL_RPATH "$ORIGIN/../3rdparty/zenoh-c/lib")
        set_target_properties(zenoh_test_sub PROPERTIES LINK_FLAGS "-Wl,-rpath,\$ORIGIN/../3rdparty/zenoh-c/lib")
        set_target_properties(multi_topic_sub PROPERTIES LINK_FLAGS "-Wl,-rpath,\$ORIGIN/../3rdparty/zenoh-c/lib")
        set_target_properties(ws2812b_publisher PROPERTIES LINK_FLAGS "-Wl,-rpath,\$ORIGIN/../3rdparty/zenoh-c/lib")
        set_target_properties(mpu6050_sub PROPERTIES LINK_FLAGS "-Wl,-rpath,\$ORIGIN/../3rdparty/zenoh-c/lib")
        set_target_properties(led_service_client PROPERTIES LINK_FLAGS "-Wl,-rpath,\$ORIGIN/../3rdparty/zenoh-c/lib")
        set_target_properties(echo PROPERTIES LINK_FLAGS "-Wl,-rpath,\$ORIGIN/../3rdparty/zenoh-c/lib")
    endif()
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

# Master All-In-One Interface Validator Target
add_executable(master_interface_validator src/master_interface_validator.cpp)
target_include_directories(master_interface_validator PRIVATE ../../shared_libraries/cpp/zenoh_ros)
target_link_libraries(master_interface_validator PRIVATE ${ZENOHC_LIB})

# Continuous Stress Validator Target
