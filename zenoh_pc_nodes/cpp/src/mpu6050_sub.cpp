#include <zenoh_ros/ZenohRosPC.h>
#include <zenoh_ros/custom_msgs/z_MPU6050Data.h>
#include <iostream>
#include <iomanip>

class MPU6050SubscriberNode : public ZenohNode {
public:
    MPU6050SubscriberNode() : ZenohNode("mpu6050_subscriber") {
        std::cout << "[Node] " << this->z_get_name() << " has been started\n";

        // Create subscription to topic "robot/mpu6050"
        sub_ = this->z_create_subscription<z_MPU6050Data>(
            "robot/mpu6050",
            [this](const z_MPU6050Data& msg) -> void {
                this->listener_callback(msg);
            },
            10
        );
    }

private:
    ZenohSubscription<z_MPU6050Data>* sub_ = nullptr;

    void listener_callback(const z_MPU6050Data& msg) {
        std::cout << std::fixed << std::setprecision(2)
                  << "[IMU RECV] Accel: (" 
                  << std::setw(6) << msg.accel_x << ", " 
                  << std::setw(6) << msg.accel_y << ", " 
                  << std::setw(6) << msg.accel_z << ") m/s² | Gyro: ("
                  << std::setw(6) << msg.gyro_x << ", " 
                  << std::setw(6) << msg.gyro_y << ", " 
                  << std::setw(6) << msg.gyro_z << ") rad/s\n";
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // Connect to ESP32-S3 SoftAP IP (192.168.4.1)
    ZenohConfig config;
    config.host = "192.168.4.1";
    config.port = 7447;

    if (!ZenohNode::init(config)) {
        std::cerr << "Failed to initialize Zenoh session!\n";
        return -1;
    }

    {
        MPU6050SubscriberNode node;
        node.z_spin();
    }

    return 0;
}
