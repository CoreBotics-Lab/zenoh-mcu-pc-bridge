#include <zenoh_ros/ZenohRosPC.h>
#include <zenoh_ros/std_msgs/z_Int32.h>
#include <zenoh_ros/std_msgs/z_String.h>

class MultiTopicSubscriberNode : public ZenohNode {
public:
    MultiTopicSubscriberNode() : ZenohNode("multi_topic_subscriber") {
        ZLOG_INFO(this->get_logger(), "Node has been started");

        // Create subscription to robot/sim_counter (Int32)
        counter_sub_ = this->z_create_subscription<z_Int32>(
            "robot/sim_counter",
            [this](const z_Int32& msg) -> void {
                this->counter_listener_callback(msg);
            },
            10
        );

        // Create subscription to robot/hello_string (String)
        string_sub_ = this->z_create_subscription<z_String>(
            "robot/hello_string",
            [this](const z_String& msg) -> void {
                this->string_listener_callback(msg);
            },
            10
        );
    }

private:
    ZenohSubscription<z_Int32>* counter_sub_ = nullptr;
    ZenohSubscription<z_String>* string_sub_ = nullptr;

    void counter_listener_callback(const z_Int32& msg) {
        ZLOG_INFO(this->get_logger(), "[RECV COUNTER] sim_counter: %d", msg.data);
    }

    void string_listener_callback(const z_String& msg) {
        ZLOG_INFO(this->get_logger(), "[RECV STRING] hello_string: %s", msg.data.c_str());
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
