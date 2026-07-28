#ifndef ZENOH_ROS_PC_H
#define ZENOH_ROS_PC_H

#include <zenoh.h>
#include <cstdint>
#include <functional>
#include <thread>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include <cstring>
#include <cstdlib>

// Global flag to handle clean shutdown via Ctrl+C
inline volatile sig_atomic_t shutdown_requested = 0;

inline void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n[Signal] Shutdown requested via SIGINT.\n";
        shutdown_requested = 1;
    }
}

// QoS classes
enum class Reliability : std::uint8_t {
    RELIABLE,
    BEST_EFFORT
};

enum class Durability : std::uint8_t {
    VOLATILE,
    TRANSIENT_LOCAL
};

struct QoS {
    Reliability reliability = Reliability::RELIABLE;
    Durability durability = Durability::VOLATILE;
    uint32_t depth = 10;
};

struct SensorDataQoS : public QoS {
    SensorDataQoS() {
        reliability = Reliability::BEST_EFFORT;
        durability = Durability::VOLATILE;
        depth = 5;
    }
};

struct SystemDefaultsQoS : public QoS {
    SystemDefaultsQoS() {
        reliability = Reliability::RELIABLE;
        durability = Durability::VOLATILE;
        depth = 10;
    }
};

#include "msg_interface/pre_defined_interface/z_std_msgs_pc.h"
#include "msg_interface/pre_defined_interface/z_geometry_msgs_pc.h"

// Callback types
typedef std::function<void()> TimerCallback;

template <typename MsgType>
using SubscriptionCallback = std::function<void(const MsgType& msg)>;

// Templated Zenoh Publisher Class for PC
template <typename MsgType>
class ZenohPublisher {
private:
    z_owned_publisher_t pub;
    const char* topic;
    bool declared;
    QoS qos_profile;

public:
    ZenohPublisher(const char* topic_name, const QoS& qos = QoS())
        : topic(topic_name), declared(false), qos_profile(qos) {}

    ~ZenohPublisher() {
        if (declared) {
            z_undeclare_publisher(z_publisher_move(&pub));
        }
    }

    bool declare(const z_loaned_session_t* z_session) {
        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, topic);

        z_publisher_options_t options;
        z_publisher_options_default(&options);

        // Apply reliability QoS mapping
        if (qos_profile.reliability == Reliability::BEST_EFFORT) {
            options.congestion_control = Z_CONGESTION_CONTROL_DROP;
        } else {
            options.congestion_control = Z_CONGESTION_CONTROL_BLOCK;
        }

        if (z_declare_publisher(z_session, &pub, z_view_keyexpr_loan(&keyexpr), &options) < 0) {
            std::cerr << "[Zenoh PC] ERROR: Unable to declare publisher on '" << topic << "'\n";
            return false;
        }

        std::cout << "[Zenoh PC] Publisher ready on '" << topic 
                  << "' with QoS Reliability: " 
                  << (qos_profile.reliability == Reliability::BEST_EFFORT ? "BEST_EFFORT" : "RELIABLE") 
                  << '\n';
        declared = true;
        return true;
    }

    bool publish(const MsgType& msg) {
        std::vector<uint8_t> buffer = serialize_msg_pc<MsgType>(msg);

        z_publisher_put_options_t options;
        z_publisher_put_options_default(&options);

        z_owned_bytes_t bytes;
        z_bytes_copy_from_buf(&bytes, buffer.data(), buffer.size());

        return z_publisher_put(z_publisher_loan(&pub), z_bytes_move(&bytes), &options) == 0;
    }
};

// Templated Zenoh Subscription Class for PC
template <typename MsgType>
class ZenohSubscription {
private:
    z_owned_subscriber_t sub;
    bool declared;
    SubscriptionCallback<MsgType> callback;
    QoS qos_profile;

public:
    ZenohSubscription(const QoS& qos = QoS()) : declared(false), qos_profile(qos) {}

    ~ZenohSubscription() {
        if (declared) {
            z_undeclare_subscriber(z_subscriber_move(&sub));
        }
    }

    bool declare(const z_loaned_session_t* z_session, const char* topic, SubscriptionCallback<MsgType> cb) {
        callback = std::move(cb);

        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, topic);

        z_owned_closure_sample_t closure;
        z_closure_sample(
            &closure,
            [](z_loaned_sample_t* sample, void* context) {
                ZenohSubscription* self = (ZenohSubscription*)context;
                if (self && self->callback) {
                    const z_loaned_bytes_t* payload = z_sample_payload(sample);
                    z_owned_slice_t slice;
                    z_bytes_to_slice(payload, &slice);

                    const uint8_t* data = z_slice_data(z_slice_loan(&slice));
                    size_t len = z_slice_len(z_slice_loan(&slice));

                    std::vector<uint8_t> buffer(data, data + len);

                    MsgType msg;
                    deserialize_msg_pc<MsgType>(buffer, msg);
                    self->callback(msg);

                    z_slice_drop(z_slice_move(&slice));
                }
            },
            NULL,
            this
        );

        if (z_declare_subscriber(z_session, &sub, z_view_keyexpr_loan(&keyexpr), z_closure_sample_move(&closure), NULL) < 0) {
            std::cerr << "[Zenoh PC] ERROR: Unable to declare subscriber on '" << topic << "'\n";
            return false;
        }

        std::cout << "[Zenoh PC] Subscription ready on '" << topic 
                  << "' with QoS Reliability: " 
                  << (qos_profile.reliability == Reliability::BEST_EFFORT ? "BEST_EFFORT" : "RELIABLE") 
                  << '\n';
        declared = true;
        return true;
    }
};

// Zenoh Timer Class for PC
class ZenohTimer {
private:
    TimerCallback callback;
    uint32_t period_ms;
    std::thread timer_thread;
    bool running;

    void run() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(period_ms));
            if (running) {
                try {
                    callback();
                } catch (const std::exception& e) {
                    std::cerr << "[ZenohTimer PC] Exception in callback: " << e.what() << '\n';
                }
            }
        }
    }

public:
    ZenohTimer(uint32_t period, TimerCallback cb)
        : callback(std::move(cb)), period_ms(period), running(true) {
        timer_thread = std::thread(&ZenohTimer::run, this);
    }

    ~ZenohTimer() {
        running = false;
        if (timer_thread.joinable()) {
            timer_thread.join();
        }
    }
};

// Configuration structure for the Zenoh node on PC
struct ZenohConfig {
    std::string host = "192.168.4.1";
    uint16_t port = 7447;
    std::string connect_endpoint = ""; // If specified, overrides host/port
};

// Main Zenoh Node Class for PC
class ZenohNode {
private:
    static z_owned_session_t session;
    static bool session_opened;
    const char* node_name;

    static bool init_session(const char* connect_endpoint) {
        if (session_opened) return true;

        // Register signal handler for Ctrl+C
        std::signal(SIGINT, signal_handler);

        // Register automatic session shutdown upon exit
        std::atexit(ZenohNode::shutdown);

        z_owned_config_t z_config;
        z_config_default(&z_config);

        if (connect_endpoint && std::strlen(connect_endpoint) > 0) {
            std::string endpoint_json = "[\"" + std::string(connect_endpoint) + "\"]";
            zc_config_insert_json5(z_config_loan_mut(&z_config), Z_CONFIG_CONNECT_KEY, endpoint_json.c_str());
            std::cout << "[Zenoh PC] Connecting to endpoint: " << connect_endpoint << "\n";
        } else {
            std::cout << "[Zenoh PC] Scouting for peers...\n";
        }

        if (z_open(&session, z_config_move(&z_config), NULL) < 0) {
            std::cerr << "[Zenoh PC] ERROR: Unable to open Zenoh session!\n";
            return false;
        }

        std::cout << "[Zenoh PC] Session opened successfully!\n";
        session_opened = true;
        return true;
    }

public:
    ZenohNode(const char* name) : node_name(name) {}

    static bool init(const ZenohConfig& config = ZenohConfig()) {
        std::string endpoint;
        if (!config.connect_endpoint.empty()) {
            endpoint = config.connect_endpoint;
        } else if (!config.host.empty()) {
            endpoint = "tcp/" + config.host + ":" + std::to_string(config.port);
        } else {
            return init_session(nullptr); // Scouting
        }
        return init_session(endpoint.c_str());
    }

    static bool init(uint16_t port) {
        ZenohConfig config;
        config.port = port;
        return init(config);
    }

    static bool init(const char* connect_endpoint) {
        ZenohConfig config;
        config.connect_endpoint = connect_endpoint ? connect_endpoint : "";
        return init(config);
    }

    static void shutdown() {
        if (session_opened) {
            z_close(z_session_loan_mut(&session), nullptr);
            z_session_drop(z_session_move(&session));
            session_opened = false;
            std::cout << "[Zenoh PC] Global session closed.\n";
        }
    }

    const char* z_get_name() const {
        return node_name;
    }

    template <typename MsgType>
    ZenohPublisher<MsgType>* z_create_publisher(const char* topic_name, const QoS& qos = QoS()) {
        ZenohPublisher<MsgType>* pub = new ZenohPublisher<MsgType>(topic_name, qos);
        if (session_opened) {
            pub->declare(z_session_loan(&session));
        } else {
            std::cerr << "[Zenoh PC] WARNING: Create publisher before ZenohNode::init.\n";
        }
        return pub;
    }

    template <typename MsgType>
    ZenohPublisher<MsgType>* z_create_publisher(const char* topic_name, uint32_t queue_size) {
        QoS qos;
        qos.depth = queue_size;
        return z_create_publisher<MsgType>(topic_name, qos);
    }

    template <typename MsgType>
    ZenohSubscription<MsgType>* z_create_subscription(const char* topic_name, SubscriptionCallback<MsgType> cb, const QoS& qos = QoS()) {
        ZenohSubscription<MsgType>* sub = new ZenohSubscription<MsgType>(qos);
        if (session_opened) {
            sub->declare(z_session_loan(&session), topic_name, std::move(cb));
        } else {
            std::cerr << "[Zenoh PC] WARNING: Create subscription before ZenohNode::init.\n";
        }
        return sub;
    }

    template <typename MsgType>
    ZenohSubscription<MsgType>* z_create_subscription(const char* topic_name, SubscriptionCallback<MsgType> cb, uint32_t queue_size) {
        QoS qos;
        qos.depth = queue_size;
        return z_create_subscription<MsgType>(topic_name, std::move(cb), qos);
    }

    ZenohTimer* z_create_timer(uint32_t period_ms, TimerCallback cb) {
        return new ZenohTimer(period_ms, std::move(cb));
    }

    void z_spin() {
        while (session_opened && !shutdown_requested) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
};

// Static definitions
z_owned_session_t ZenohNode::session;
bool ZenohNode::session_opened = false;

#endif // ZENOH_ROS_PC_H
