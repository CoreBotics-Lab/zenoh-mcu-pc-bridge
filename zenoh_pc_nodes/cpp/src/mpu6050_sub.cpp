#include <zenoh_ros/ZenohRosPC.h>
#include <zenoh_ros/sensor_msgs/z_Imu.h>
#include <iostream>
#include <iomanip>

class MPU6050SubscriberNode : public ZenohNode {
public:
    MPU6050SubscriberNode() : ZenohNode("mpu6050_subscriber") {
        std::cout << "[Node] " << this->z_get_name() << " has been started\n";

        // Create subscription to topic "robot/mpu6050" with standard ROS 2 z_Imu interface
        sub_ = this->z_create_subscription<z_Imu>(
            "robot/mpu6050",
            [this](const z_Imu& msg) -> void {
                this->listener_callback(msg);
            },
            10
        );
    }

private:
    ZenohSubscription<z_Imu>* sub_ = nullptr;

    void listener_callback(const z_Imu& msg) {
        std::cout << std::fixed << std::setprecision(2)
                  << "[IMU RECV] [stamp: " << msg.header.stamp.sec << "." << msg.header.stamp.nanosec << "] Accel: (" 
                  << std::setw(6) << msg.linear_acceleration.x << ", " 
                  << std::setw(6) << msg.linear_acceleration.y << ", " 
                  << std::setw(6) << msg.linear_acceleration.z << ") m/s² | Gyro: ("
                  << std::setw(6) << msg.angular_velocity.x << ", " 
                  << std::setw(6) << msg.angular_velocity.y << ", " 
                  << std::setw(6) << msg.angular_velocity.z << ") rad/s\n";
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
