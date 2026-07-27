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

class CounterSubscriberNode : public ZenohNode {
public:
    CounterSubscriberNode() : ZenohNode("counter_subscriber") {
        std::cout << "[Node] " << this->z_get_name() << " has been started\n";

        // Create subscription to standard ROS 2 message type (Int32) and depth 10
        sub_ = this->z_create_subscription<z_std_msgs::Int32>(
            "robot/sim_counter",
            [this](const z_std_msgs::Int32& msg) -> void {
                this->listener_callback(msg);
            },
            10
        );
    }

    ~CounterSubscriberNode() {
        if (sub_) {
            delete sub_;
        }
        std::cout << "Destroying the Zenoh Node...\n";
    }

private:
    ZenohSubscription<z_std_msgs::Int32>* sub_ = nullptr;

    void listener_callback(const z_std_msgs::Int32& msg) {
        // Message is already deserialized into standard z_std_msgs::Int32 struct!
        std::cout << "[RECV FROM ESP32-S3] sim_counter: " << msg.data << "\n";
    }
};

int main(int argc, char** argv) {
    // Unused argc and argv
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
        CounterSubscriberNode node;
        
        // Spin the node to keep it alive (exactly like rclcpp::spin or the Python z_spin)
        node.z_spin();
    } // node goes out of scope and is cleaned up here

    return 0;
}
