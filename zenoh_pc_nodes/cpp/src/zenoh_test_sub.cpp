#include <zenoh_ros/ZenohRosPC.h>
#include <zenoh_ros/std_msgs/z_Int32.h>

class CounterSubscriberNode : public ZenohNode {
public:
    CounterSubscriberNode() : ZenohNode("counter_subscriber") {
        ZLOG_INFO(this->get_logger(), "Node has been started");

        // Create subscription to standard ROS 2 message type (Int32) and depth 10
        sub_ = this->z_create_subscription<z_Int32>(
            "robot/sim_counter",
            [this](const z_Int32& msg) -> void {
                this->listener_callback(msg);
            },
            10
        );
    }

private:
    ZenohSubscription<z_Int32>* sub_ = nullptr;

    void listener_callback(const z_Int32& msg) {
        ZLOG_INFO(this->get_logger(), "[RECV FROM MCU] sim_counter: %d", msg.data);
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // Initialize Zenoh session — connect to MCU SoftAP via Wi-Fi
    ZenohConfig config;
    config.communication_mode = ZenohCommunicationMode::ZENOH_COMM_WIFI;
    config.host = "10.42.0.50";
    config.port = 7447;

    if (!ZenohNode::init(config)) {
        return -1;
    }

    {
        CounterSubscriberNode node;
        node.z_spin();
    } // Node goes out of scope and destroys subscriptions cleanly before session closes via atexit

    return 0;
}
