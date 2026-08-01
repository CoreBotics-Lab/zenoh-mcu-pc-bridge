#ifndef ZENOH_ROS_H
#define ZENOH_ROS_H

#include <Arduino.h>
#include <WiFi.h>
#include <zenoh-pico.h>
#include <ArduinoJson.h>
#include <functional>
#include <vector>

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


// Callback Types for Timer and Subscriptions
typedef std::function<void()> TimerCallback;

template <typename MsgType>
using SubscriptionCallback = std::function<void(const MsgType& msg)>;

// Modular Templated Zenoh Publisher Class
template <typename MsgType, size_t MaxBufferSize = 128>
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
        uint8_t buffer[MaxBufferSize];
        size_t len = serialize_msg<MsgType>(msg, buffer, MaxBufferSize);

        if (len == 0) {
            Serial.printf("[Zenoh] ERROR: Message serialization failed or exceeded buffer limit of %u bytes on topic '%s'!\n", 
                          MaxBufferSize, topic);
            return false;
        }

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

// Templated Memory-Optimized Zenoh Service Server Class for MCU
template <typename SrvType, size_t MaxBufSize = 256>
class ZenohService {
public:
    using ServiceCallback = std::function<void(const typename SrvType::Request& req, typename SrvType::Response& res)>;

private:
    z_owned_queryable_t queryable;
    bool declared;
    ServiceCallback callback;

public:
    ZenohService() : declared(false) {}

    ~ZenohService() {
        if (declared) {
            z_undeclare_queryable(z_queryable_move(&queryable));
        }
    }

    bool declare(const z_loaned_session_t* z_session, const char* service_name, ServiceCallback cb) {
        callback = cb;

        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, service_name);

        z_owned_closure_query_t closure;
        z_closure_query(
            &closure,
            [](z_loaned_query_t* query, void* context) {
                ZenohService* self = (ZenohService*)context;
                if (self && self->callback) {
                    const z_loaned_bytes_t* payload = z_query_payload(query);
                    uint8_t req_buf[MaxBufSize];
                    size_t req_len = 0;

                    if (payload) {
                        z_owned_slice_t slice;
                        z_bytes_to_slice(payload, &slice);
                        const uint8_t* data = z_slice_data(z_slice_loan(&slice));
                        req_len = z_slice_len(z_slice_loan(&slice));
                        if (req_len > MaxBufSize) req_len = MaxBufSize;
                        memcpy(req_buf, data, req_len);
                        z_slice_drop(z_slice_move(&slice));
                    }

                    typename SrvType::Request req;
                    if (req_len > 0) {
                        deserialize_msg<typename SrvType::Request>(req_buf, req_len, req);
                    }

                    typename SrvType::Response res;
                    self->callback(req, res);

                    uint8_t res_buf[MaxBufSize];
                    size_t res_len = serialize_msg<typename SrvType::Response>(res, res_buf, MaxBufSize);

                    z_query_reply_options_t options;
                    z_query_reply_options_default(&options);

                    z_owned_bytes_t reply_bytes;
                    z_bytes_copy_from_buf(&reply_bytes, res_buf, res_len);

                    const z_loaned_keyexpr_t* q_key = z_query_keyexpr(query);
                    z_query_reply(query, q_key, z_bytes_move(&reply_bytes), &options);
                }
            },
            NULL,
            this
        );

        if (z_declare_queryable(z_session, &queryable, z_view_keyexpr_loan(&keyexpr), z_closure_query_move(&closure), NULL) < 0) {
            Serial.printf("[Zenoh MCU] ERROR: Unable to declare service queryable on '%s'\n", service_name);
            return false;
        }

        Serial.printf("[Zenoh MCU] Service ready on '%s'\n", service_name);
        declared = true;
        return true;
    }
};

// Templated Memory-Optimized Zenoh Service Client Class for MCU
template <typename SrvType, size_t MaxBufSize = 256>
class ZenohClient {
private:
    const z_loaned_session_t* session;
    const char* service_name;

public:
    ZenohClient(const char* name) : session(nullptr), service_name(name) {}

    void set_session(const z_loaned_session_t* z_session) {
        session = z_session;
    }

    bool wait_for_service(uint32_t timeout_ms = 5000) {
        if (!session) return false;
        unsigned long start = millis();
        while (millis() - start < timeout_ms) {
            delay(10);
            return true;
        }
        return false;
    }

    bool call(const typename SrvType::Request& req, typename SrvType::Response& res, uint32_t timeout_ms = 5000) {
        if (!session) {
            Serial.println("[ZenohClient MCU] ERROR: Client session not initialized!");
            return false;
        }

        uint8_t req_buf[MaxBufSize];
        size_t req_len = serialize_msg<typename SrvType::Request>(req, req_buf, MaxBufSize);

        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, service_name);

        z_get_options_t options;
        z_get_options_default(&options);
        options.timeout_ms = timeout_ms;

        z_owned_bytes_t req_bytes;
        z_bytes_copy_from_buf(&req_bytes, req_buf, req_len);
        options.payload = z_bytes_move(&req_bytes);

        struct ReplyContext {
            uint8_t res_buf[MaxBufSize];
            size_t res_len = 0;
            bool received = false;
        } ctx;

        z_owned_closure_reply_t closure;
        z_closure_reply(
            &closure,
            [](z_loaned_reply_t* reply, void* context) {
                ReplyContext* c = (ReplyContext*)context;
                if (c && z_reply_is_ok(reply)) {
                    const z_loaned_sample_t* sample = z_reply_ok(reply);
                    const z_loaned_bytes_t* payload = z_sample_payload(sample);
                    if (payload) {
                        z_owned_slice_t slice;
                        z_bytes_to_slice(payload, &slice);
                        const uint8_t* data = z_slice_data(z_slice_loan(&slice));
                        size_t len = z_slice_len(z_slice_loan(&slice));
                        if (len > MaxBufSize) len = MaxBufSize;
                        memcpy(c->res_buf, data, len);
                        c->res_len = len;
                        c->received = true;
                        z_slice_drop(z_slice_move(&slice));
                    }
                }
            },
            NULL,
            &ctx
        );

        if (z_get(session, z_view_keyexpr_loan(&keyexpr), "", z_closure_reply_move(&closure), &options) < 0) {
            Serial.printf("[ZenohClient MCU] ERROR: Failed to send service call to '%s'\n", service_name);
            return false;
        }

        if (ctx.received && ctx.res_len > 0) {
            deserialize_msg<typename SrvType::Response>(ctx.res_buf, ctx.res_len, res);
            return true;
        }

        Serial.printf("[ZenohClient MCU] ERROR: Service call to '%s' timed out or returned empty!\n", service_name);
        return false;
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
    std::vector<std::function<void()>> cleanup_callbacks;

public:
    ZenohNode(const char* name) : node_name(name) {}

    ~ZenohNode() {
        for (auto& cleanup : cleanup_callbacks) {
            cleanup();
        }
    }

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
            if (cfg.local_ip && strlen(cfg.local_ip) > 0) {
                IPAddress ip, gw, net;
                if (ip.fromString(cfg.local_ip)) {
                    if (cfg.gateway && strlen(cfg.gateway) > 0) {
                        gw.fromString(cfg.gateway);
                    } else {
                        // Default gateway to .1 of the same subnet (e.g. 10.42.0.1)
                        gw = ip;
                        gw[3] = 1;
                    }
                    if (cfg.subnet && strlen(cfg.subnet) > 0) {
                        net.fromString(cfg.subnet);
                    } else {
                        net = IPAddress(255, 255, 255, 0);
                    }
                    if (!WiFi.config(ip, gw, net)) {
                        Serial.println("[Wi-Fi] ERROR: Static STA config failed!");
                    }
                } else {
                    Serial.println("[Wi-Fi] ERROR: Invalid local_ip format!");
                }
            }
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
    template <typename MsgType, size_t MaxBufferSize = 128>
    ZenohPublisher<MsgType, MaxBufferSize>* z_create_publisher(const char* topic_name, const QoS& qos = QoS()) {
        ZenohPublisher<MsgType, MaxBufferSize>* pub = new ZenohPublisher<MsgType, MaxBufferSize>(topic_name, qos);
        if (session_opened) {
            pub->declare(z_session_loan(&session));
        } else {
            Serial.println("[Zenoh] WARNING: Attempted to create publisher before ZenohNode::init.");
        }
        cleanup_callbacks.push_back([pub]() { delete pub; });
        return pub;
    }

    /**
     * Shorthand creator for a publisher with a custom queue depth (exactly like ROS 2).
     */
    template <typename MsgType, size_t MaxBufferSize = 128>
    ZenohPublisher<MsgType, MaxBufferSize>* z_create_publisher(const char* topic_name, uint32_t queue_size) {
        QoS qos;
        qos.depth = queue_size;
        return z_create_publisher<MsgType, MaxBufferSize>(topic_name, qos);
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
        cleanup_callbacks.push_back([sub]() { delete sub; });
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

    template <typename SrvType, size_t MaxBufSize = 256>
    ZenohService<SrvType, MaxBufSize>* z_create_service(const char* service_name, typename ZenohService<SrvType, MaxBufSize>::ServiceCallback cb) {
        ZenohService<SrvType, MaxBufSize>* srv = new ZenohService<SrvType, MaxBufSize>();
        if (session_opened) {
            srv->declare(z_session_loan(&session), service_name, cb);
        } else {
            Serial.println("[Zenoh] WARNING: Attempted to create service before ZenohNode::init.");
        }
        cleanup_callbacks.push_back([srv]() { delete srv; });
        return srv;
    }

    template <typename SrvType, size_t MaxBufSize = 256>
    ZenohClient<SrvType, MaxBufSize>* z_create_client(const char* service_name) {
        ZenohClient<SrvType, MaxBufSize>* client = new ZenohClient<SrvType, MaxBufSize>(service_name);
        if (session_opened) {
            client->set_session(z_session_loan(&session));
        } else {
            Serial.println("[Zenoh] WARNING: Attempted to create client before ZenohNode::init.");
        }
        cleanup_callbacks.push_back([client]() { delete client; });
        return client;
    }

    /**
     * Creates a wall timer that repeatedly triggers a callback at defined milliseconds.
     */
    ZenohTimer* z_create_timer(uint32_t period_ms, TimerCallback cb) {
        ZenohTimer* timer = new ZenohTimer(period_ms, cb);
        cleanup_callbacks.push_back([timer]() { delete timer; });
        return timer;
    }

    /**
     * Publishes raw bytes to a key expression topic (used internally for logging).
     */
    void z_publish_raw(const char* topic_name, const uint8_t* payload, size_t len) {
        if (!session_opened || payload == nullptr || len == 0) return;

        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, topic_name);

        z_put_options_t options;
        z_put_options_default(&options);

        z_owned_bytes_t bytes;
        z_bytes_copy_from_buf(&bytes, payload, len);

        z_put(z_session_loan(&session), z_view_keyexpr_loan(&keyexpr), z_bytes_move(&bytes), &options);
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

// Global non-blocking RTOS delay helper
inline void z_delay(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

#endif // ZENOH_ROS_H