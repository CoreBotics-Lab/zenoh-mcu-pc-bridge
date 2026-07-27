#include "../include/ZenohWorkbenchPC.h"
#include <iostream>

class MultiTopicSubscriberNode : public ZenohNode {
public:
    MultiTopicSubscriberNode() : ZenohNode("multi_topic_subscriber") {
        std::cout << "[Node] " << this->z_get_name() << " has been started\n";

        // Create subscription to robot/sim_counter (Int32)
        counter_sub_ = this->z_create_subscription<z_std_msgs::Int32>(
            "robot/sim_counter",
            [this](const z_std_msgs::Int32& msg) -> void {
                this->counter_listener_callback(msg);
            },
            10
        );

        // Create subscription to robot/hello_string (String)
        string_sub_ = this->z_create_subscription<z_std_msgs::String>(
            "robot/hello_string",
            [this](const z_std_msgs::String& msg) -> void {
                this->string_listener_callback(msg);
            },
            10
        );
    }

    ~MultiTopicSubscriberNode() {
        if (counter_sub_) delete counter_sub_;
        if (string_sub_) delete string_sub_;
        std::cout << "Destroying the Zenoh Node...\n";
    }

private:
    ZenohSubscription<z_std_msgs::Int32>* counter_sub_ = nullptr;
    ZenohSubscription<z_std_msgs::String>* string_sub_ = nullptr;

    void counter_listener_callback(const z_std_msgs::Int32& msg) {
        std::cout << "[RECV COUNTER] sim_counter: " << msg.data << "\n";
    }

    void string_listener_callback(const z_std_msgs::String& msg) {
        std::cout << "[RECV STRING] hello_string: " << msg.data << "\n";
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // Initialize with default AP network settings (IP: 192.168.4.1, Port: 7447)
    ZenohConfig config;
    config.host = "192.168.4.1";
    config.port = 7447;

    if (!ZenohNode::init(config)) {
        return -1;
    }

    {
        MultiTopicSubscriberNode node;
        node.z_spin();
    } // Node goes out of scope and destroys subscriptions cleanly before session closes via atexit

    return 0;
}
