#include <zenoh_ros/ZenohRosPC.h>
#include <zenoh_ros/sensor_msgs/z_Imu.h>
#include <iomanip>
#include <sstream>

class MPU6050SubscriberNode : public ZenohNode {
public:
    MPU6050SubscriberNode() : ZenohNode("mpu6050_subscriber") {
        ZLOG_INFO(this->get_logger(), "Node has been started");

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
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2)
            << "[IMU RECV] [stamp: " << msg.header.stamp.sec << "." << msg.header.stamp.nanosec
            << "] Accel: ("
            << std::setw(6) << msg.linear_acceleration.x << ", "
            << std::setw(6) << msg.linear_acceleration.y << ", "
            << std::setw(6) << msg.linear_acceleration.z << ") m/s² | Gyro: ("
            << std::setw(6) << msg.angular_velocity.x << ", "
            << std::setw(6) << msg.angular_velocity.y << ", "
            << std::setw(6) << msg.angular_velocity.z << ") rad/s";
        ZLOG_INFO(this->get_logger(), "%s", oss.str().c_str());
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // Connect to ESP32-S3 SoftAP via Wi-Fi
    ZenohConfig config;
    config.communication_mode = ZenohCommunicationMode::ZENOH_COMM_WIFI;
    config.host = "192.168.4.1";
    config.port = 7447;

    if (!ZenohNode::init(config)) {
        return -1;
    }

    {
        MPU6050SubscriberNode node;
        node.z_spin();
    }

    return 0;
}
