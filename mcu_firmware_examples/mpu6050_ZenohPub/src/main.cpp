/*
 * ====================================================================
 * ESP32-S3 ↔ MPU6050 (GY-87 / HW-290 Breakout) Wiring Diagram
 * ====================================================================
 *  MPU / GY-87 Pin | ESP32-S3 Pin | Function / Description
 * -----------------+--------------+-----------------------------------
 *  VCC_IN / VCC    | 5V or 3.3V   | Main Power Supply (5V or 3.3V)
 *  3.3V            | (Unconnected)| Direct 3.3V Output (Leave NC)
 *  GND             | GND          | Common Ground
 *  SCL             | GPIO 10      | I2C Clock Line
 *  SDA             | GPIO 9       | I2C Data Line
 *  INTA            | GPIO 18      | MPU6050 Hardware Interrupt Pin
 * ====================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>

#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/custom_msgs/z_MPU6050Data.h>

// --- Pin Definitions for ESP32-S3 ---
#define I2C_SDA_PIN  9
#define I2C_SCL_PIN  10
#define MPU_INT_PIN  18

// --- Configuration Struct (AP Mode) ---
ZenohConfig cfg = {
    .ssid = "ESP32S3_Zenoh_AP",
    .password = "zenoh1234",
    .port = 7447
};

// Sensor Globals
MPU6050 mpu(0x68);
MPU6050 mpu_alt(0x69);
bool mpu_use_alt = false;
bool mpu_detected = false;

class MPU6050_Publisher_Node : public ZenohNode {
public:
    MPU6050_Publisher_Node() : ZenohNode("mpu6050_publisher") {
        Serial.printf("[Node] %s has been started\n", this->z_get_name());
        
        // 1. Create a typed publisher
        publisher_ = this->z_create_publisher<z_MPU6050Data>("robot/mpu6050", 10);

        // 2. Create timer (triggers callback_timer every 50ms / 20Hz)
        timer_ = this->z_create_timer(50, [this]() -> void {
            this->callback_timer();
        });
    }

private:
    ZenohPublisher<z_MPU6050Data>* publisher_ = nullptr;
    ZenohTimer* timer_ = nullptr;
    z_MPU6050Data msg_;

    void callback_timer() {
        if (!this->publisher_ || !mpu_detected) return;

        int16_t ax = 0, ay = 0, az = 0;
        int16_t gx = 0, gy = 0, gz = 0;

        if (mpu_use_alt) {
            mpu_alt.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        } else {
            mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        }

        // Convert raw values to standard SI units:
        msg_.accel_x = (ax / 16384.0f) * 9.80665f;
        msg_.accel_y = (ay / 16384.0f) * 9.80665f;
        msg_.accel_z = (az / 16384.0f) * 9.80665f;

        msg_.gyro_x = (gx / 131.0f) * (3.14159265f / 180.0f);
        msg_.gyro_y = (gy / 131.0f) * (3.14159265f / 180.0f);
        msg_.gyro_z = (gz / 131.0f) * (3.14159265f / 180.0f);
        msg_.temperature = 0.0f;

        // Publish over Zenoh
        this->publisher_->publish(msg_);

        Serial.printf("[%s] Publishing Accel: (%6.2f, %6.2f, %6.2f) m/s² | Gyro: (%6.2f, %6.2f, %6.2f) rad/s\n",
                      this->z_get_name(),
                      msg_.accel_x, msg_.accel_y, msg_.accel_z,
                      msg_.gyro_x, msg_.gyro_y, msg_.gyro_z);
    }
};

MPU6050_Publisher_Node* node_instance = nullptr;

void init_i2c() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(400000);
    Serial.printf("[I2C] Wire initialized on SDA: GPIO %d, SCL: GPIO %d\n", I2C_SDA_PIN, I2C_SCL_PIN);
}

void init_mpu() {
    mpu.initialize();

    if (!mpu.testConnection()) {
        Serial.println("[MPU6050] Address 0x68 failed. Trying alternate address 0x69...");
        mpu_alt.initialize();
        if (mpu_alt.testConnection()) {
            mpu_use_alt = true;
            mpu_detected = true;
            Serial.println("[MPU6050] Connected successfully at address 0x69!");
            return;
        }
    } else {
        mpu_detected = true;
        Serial.println("[MPU6050] Connected successfully at address 0x68!");
        return;
    }

    mpu_detected = false;
    Serial.println("[MPU6050] ERROR: MPU6050 testConnection failed!");
}

void setup() {
    Serial.begin(115200);
    z_delay(2000);

    Serial.println("\n==========================================");
    Serial.println("  ESP32-S3 MPU6050 Zenoh SoftAP Publisher");
    Serial.println("==========================================");

    init_i2c();
    init_mpu();

    // 1. Initialize global Zenoh client context
    Serial.println("[System] Initializing the Zenoh Client...");
    if (ZenohNode::init(cfg)) {
        // 2. Spin up Node instance
        Serial.println("[System] Starting the Zenoh Node...");
        node_instance = new MPU6050_Publisher_Node();
    } else {
        Serial.println("[System] CRITICAL Error: Zenoh Client initialization failed!");
        while (1) { z_delay(1000); }
    }

    Serial.println("==========================================\n");
}

void loop() {
    // Spin the node to keep it alive
    if (node_instance) {
        node_instance->z_spin();
    }
}
