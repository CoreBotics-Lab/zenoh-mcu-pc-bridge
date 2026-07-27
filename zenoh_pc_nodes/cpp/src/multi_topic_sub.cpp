#include "../include/ZenohWorkbenchPC.h"
#include <iostream>
#include <csignal>

// Global flag to handle clean shutdown via Ctrl+C
volatile sig_atomic_t shutdown_requested = 0;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n[Signal] Shutdown requested via SIGINT.\n";
        ZenohNode::shutdown();
    }
}

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
        if (counter_sub_) {
            delete counter_sub_;
        }
        if (string_sub_) {
            delete string_sub_;
        }
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

    // Register signal handler for Ctrl+C
    std::signal(SIGINT, signal_handler);

    // Initialize Zenoh session with ESP32 SoftAP gateway address
    const char* connect_endpoint = "tcp/192.168.4.1:7447";
    
    std::cout << "Opening Zenoh session to ESP32-S3 (" << connect_endpoint << ")...\n";
    if (!ZenohNode::init(connect_endpoint)) {
        std::cerr << "CRITICAL Error: Zenoh Client initialization failed!\n";
        return -1;
    }

    {
        MultiTopicSubscriberNode node;
        
        // Spin the node to keep it alive (exactly like rclcpp::spin or the Python z_spin)
        node.z_spin();
    } // node goes out of scope and is cleaned up here

    return 0;
}
