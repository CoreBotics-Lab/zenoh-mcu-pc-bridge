#ifndef ZENOH_WORKBENCH_H
#define ZENOH_WORKBENCH_H

#include <Arduino.h>
#include <WiFi.h>
#include <zenoh-pico.h>
#include <ArduinoJson.h>
#include <functional>

// Configuration structure for the Zenoh node and Wi-Fi SoftAP
struct ZenohConfig {
    const char* ssid;
    const char* password;
    uint16_t port;
    const char* local_ip; // Optional: custom static IP for SoftAP (defaults to 192.168.4.1)
    const char* gateway;  // Optional: gateway IP for SoftAP (defaults to local_ip)
    const char* subnet;   // Optional: subnet mask for SoftAP (defaults to 255.255.255.0)
    WiFiMode_t wifi_mode; // Optional: Wi-Fi mode (WIFI_STA or WIFI_AP, defaults to WIFI_AP)
};

// --- ROS2-style QoS settings ---
enum class Reliability {
    RELIABLE,
    BEST_EFFORT
};

enum class Durability {
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

// --- Message serialization/deserialization helper template declarations ---
template <typename T>
size_t serialize_msg(const T& msg, uint8_t* buffer, size_t max_len);

template <typename T>
void deserialize_msg(const uint8_t* buffer, size_t len, T& msg);

// Include modular ROS2-style message libraries
#include "z_std_msgs.h"
#include "z_geometry_msgs.h"

// Callback Types for Timer and Subscriptions
typedef std::function<void()> TimerCallback;

template <typename MsgType>
using SubscriptionCallback = std::function<void(const MsgType& msg)>;

// Modular Templated Zenoh Publisher Class
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

    /**
     * Declares the publisher inside the active Zenoh session.
     */
    bool declare(const z_loaned_session_t* z_session) {
        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, topic);

        z_publisher_options_t options;
        z_publisher_options_default(&options);

        // Apply reliability QoS to Zenoh publisher options congestion control
        if (qos_profile.reliability == Reliability::BEST_EFFORT) {
            options.congestion_control = Z_CONGESTION_CONTROL_DROP;
        } else {
            options.congestion_control = Z_CONGESTION_CONTROL_BLOCK;
        }

        if (z_declare_publisher(z_session, &pub, z_view_keyexpr_loan(&keyexpr), &options) < 0) {
            Serial.printf("[Zenoh] ERROR: Unable to declare publisher on '%s'\n", topic);
            return false;
        }

        Serial.printf("[Zenoh] Publisher ready on '%s' with QoS Reliability: %s\n", 
                      topic, 
                      qos_profile.reliability == Reliability::BEST_EFFORT ? "BEST_EFFORT" : "RELIABLE");
        declared = true;
        return true;
    }

    /**
     * Publishes a structured message (automatically serializes to MessagePack).
     */
    bool publish(const MsgType& msg) {
        uint8_t buffer[128];
        size_t len = serialize_msg<MsgType>(msg, buffer, sizeof(buffer));

        z_publisher_put_options_t options;
        z_publisher_put_options_default(&options);

        z_owned_bytes_t bytes;
        z_bytes_copy_from_buf(&bytes, buffer, len);

        return z_publisher_put(z_publisher_loan(&pub), z_bytes_move(&bytes), &options) == 0;
    }
};

// Modular Templated Zenoh Subscription Class
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

    /**
     * Declares the subscription inside the active Zenoh session.
     */
    bool declare(const z_loaned_session_t* z_session, const char* topic, SubscriptionCallback<MsgType> cb) {
        callback = cb;
        
        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, topic);

        // Closure callback triggered on data arrival
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
                    
                    // Deserialize to the target MsgType
                    MsgType msg;
                    deserialize_msg<MsgType>(data, len, msg);
                    self->callback(msg);
                    
                    z_slice_drop(z_slice_move(&slice));
                }
            },
            NULL,
            this
        );

        if (z_declare_subscriber(z_session, &sub, z_view_keyexpr_loan(&keyexpr), z_closure_sample_move(&closure), NULL) < 0) {
            Serial.printf("[Zenoh] ERROR: Unable to declare subscriber on '%s'\n", topic);
            return false;
        }

        Serial.printf("[Zenoh] Subscription ready on '%s' with QoS Reliability: %s\n", 
                      topic,
                      qos_profile.reliability == Reliability::BEST_EFFORT ? "BEST_EFFORT" : "RELIABLE");
        declared = true;
        return true;
    }
};

// Modular Zenoh Wall Timer Class
class ZenohTimer {
private:
    TimerCallback callback;
    uint32_t period_ms;
    TaskHandle_t task_handle;
    bool running;

    static void timerTask(void* pvParameters) {
        ZenohTimer* self = (ZenohTimer*)pvParameters;
        while (self->running) {
            self->callback();
            vTaskDelay(pdMS_TO_TICKS(self->period_ms));
        }
        vTaskDelete(NULL);
    }

public:
    ZenohTimer(uint32_t period, TimerCallback cb)
        : callback(cb), period_ms(period), task_handle(NULL), running(true) {
        xTaskCreatePinnedToCore(
            timerTask,
            "ZenohTimer",
            4096,
            this,
            1,
            &task_handle,
            1 // Run timers on Core 1 to separate execution
        );
    }

    ~ZenohTimer() {
        running = false;
        // FreeRTOS task deletes itself upon loop termination
    }
};

// Main Zenoh Node Class (ROS2-style abstraction)
class ZenohNode {
private:
    static z_owned_session_t session;
    static bool session_opened;
    const char* node_name;

public:
    ZenohNode(const char* name) : node_name(name) {}

    /**
     * Initializes the Wi-Fi AP network and starts the global Zenoh peer session.
     */
    static bool init(const ZenohConfig& cfg = ZenohConfig()) {
        if (session_opened) return true;
        
        // 1. Initialize Wi-Fi
        WiFiMode_t mode = cfg.wifi_mode;
        if (mode == WIFI_OFF) {
            mode = WIFI_AP; // Default to AP mode for backwards compatibility
        }

        if (mode == WIFI_STA) {
            WiFi.mode(WIFI_STA);
            WiFi.begin(cfg.ssid ? cfg.ssid : "", cfg.password ? cfg.password : "");
            Serial.printf("[Wi-Fi] Connecting to SSID: %s ", cfg.ssid ? cfg.ssid : "");
            unsigned long start_time = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - start_time < 10000) {
                delay(500);
                Serial.print(".");
            }
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("\n[Wi-Fi] ERROR: Connection failed/timed out!");
                return false;
            }
            Serial.println("\n[Wi-Fi] Connected successfully!");
            Serial.print("[Wi-Fi] IP Address        : "); Serial.println(WiFi.localIP());
        } else {
            WiFi.mode(WIFI_AP);
            if (cfg.local_ip && strlen(cfg.local_ip) > 0) {
                IPAddress ip, gw, net;
                if (ip.fromString(cfg.local_ip)) {
                    if (cfg.gateway && strlen(cfg.gateway) > 0) {
                        gw.fromString(cfg.gateway);
                    } else {
                        gw = ip; // Default gateway to the AP's local IP
                    }
                    if (cfg.subnet && strlen(cfg.subnet) > 0) {
                        net.fromString(cfg.subnet);
                    } else {
                        net = IPAddress(255, 255, 255, 0); // Default subnet
                    }
                    if (!WiFi.softAPConfig(ip, gw, net)) {
                        Serial.println("[Wi-Fi] ERROR: SoftAP config failed!");
                    }
                } else {
                    Serial.println("[Wi-Fi] ERROR: Invalid local_ip format!");
                }
            }
            if (!WiFi.softAP(cfg.ssid ? cfg.ssid : "ESP32S3_Zenoh_AP", 
                             cfg.password ? cfg.password : "zenoh1234")) {
                Serial.println("[Wi-Fi] ERROR: SoftAP failed to start!");
                return false;
            }
            Serial.println("[Wi-Fi] SoftAP launched successfully!");
            Serial.printf("[Wi-Fi] Access Point SSID : %s\n", cfg.ssid ? cfg.ssid : "ESP32S3_Zenoh_AP");
            Serial.print("[Wi-Fi] Gateway IP        : "); Serial.println(WiFi.softAPIP());
        }

        // 2. Initialize Zenoh session with peer listener endpoint
        Serial.println("[Zenoh] Initializing Zenoh Session...");
        z_owned_config_t z_config;
        z_config_default(&z_config);
        
        // P2P Peer Mode
        zp_config_insert(z_config_loan_mut(&z_config), Z_CONFIG_MODE_KEY, "peer");
        
        // TCP Listening port
        char listen_endpoint[64];
        snprintf(listen_endpoint, sizeof(listen_endpoint), "tcp/0.0.0.0:%u", cfg.port ? cfg.port : 7447);
        zp_config_insert(z_config_loan_mut(&z_config), Z_CONFIG_LISTEN_KEY, listen_endpoint);

        if (z_open(&session, z_config_move(&z_config), NULL) < 0) {
            Serial.println("[Zenoh] ERROR: Unable to open Zenoh session!");
            return false;
        }

        Serial.println("[Zenoh] Session opened successfully!");
        session_opened = true;
        return true;
    }

    /**
     * Returns the name of this node.
     */
    const char* z_get_name() const {
        return node_name;
    }

    /**
     * Creates a new publisher and registers it within the active Zenoh session.
     * Returns a dynamically allocated publisher pointer.
     */
    template <typename MsgType>
    ZenohPublisher<MsgType>* z_create_publisher(const char* topic_name, const QoS& qos = QoS()) {
        ZenohPublisher<MsgType>* pub = new ZenohPublisher<MsgType>(topic_name, qos);
        if (session_opened) {
            pub->declare(z_session_loan(&session));
        } else {
            Serial.println("[Zenoh] WARNING: Attempted to create publisher before ZenohNode::init.");
        }
        return pub;
    }

    /**
     * Shorthand creator for a publisher with a custom queue depth (exactly like ROS 2).
     */
    template <typename MsgType>
    ZenohPublisher<MsgType>* z_create_publisher(const char* topic_name, uint32_t queue_size) {
        QoS qos;
        qos.depth = queue_size;
        return z_create_publisher<MsgType>(topic_name, qos);
    }

    /**
     * Creates a new subscription and registers it within the active Zenoh session.
     * Returns a dynamically allocated subscription pointer.
     */
    template <typename MsgType>
    ZenohSubscription<MsgType>* z_create_subscription(const char* topic_name, SubscriptionCallback<MsgType> cb, const QoS& qos = QoS()) {
        ZenohSubscription<MsgType>* sub = new ZenohSubscription<MsgType>(qos);
        if (session_opened) {
            sub->declare(z_session_loan(&session), topic_name, cb);
        } else {
            Serial.println("[Zenoh] WARNING: Attempted to create subscription before ZenohNode::init.");
        }
        return sub;
    }

    /**
     * Shorthand creator for a subscription with a custom queue depth (exactly like ROS 2).
     */
    template <typename MsgType>
    ZenohSubscription<MsgType>* z_create_subscription(const char* topic_name, SubscriptionCallback<MsgType> cb, uint32_t queue_size) {
        QoS qos;
        qos.depth = queue_size;
        return z_create_subscription<MsgType>(topic_name, cb, qos);
    }

    /**
     * Creates a wall timer that repeatedly triggers a callback at defined milliseconds.
     */
    ZenohTimer* z_create_timer(uint32_t period_ms, TimerCallback cb) {
        return new ZenohTimer(period_ms, cb);
    }

    /**
     * Blocks the thread and yields CPU, keeping the node alive (similar to rclcpp::spin).
     */
    void z_spin() {
        while (session_opened) {
            delay(1000); // Sleep to yield CPU and feed watchdog
        }
    }
};

// C++11 Static member definitions
z_owned_session_t ZenohNode::session;
bool ZenohNode::session_opened = false;

#endif // ZENOH_WORKBENCH_H