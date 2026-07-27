import zenoh
import msgpack
import threading
import time
from enum import Enum
from typing import Type, Callable, List, Union, Any, Optional, ClassVar

class Reliability(Enum):
    RELIABLE = 1
    BEST_EFFORT = 2

class Durability(Enum):
    VOLATILE = 1
    TRANSIENT_LOCAL = 2

class QoS:
    def __init__(self, reliability: Reliability = Reliability.RELIABLE, durability: Durability = Durability.VOLATILE, depth: int = 10) -> None:
        self.reliability = reliability
        self.durability = durability
        self.depth = depth

class SensorDataQoS(QoS):
    def __init__(self) -> None:
        super().__init__(reliability=Reliability.BEST_EFFORT, durability=Durability.VOLATILE, depth=5)

class SystemDefaultsQoS(QoS):
    def __init__(self) -> None:
        super().__init__(reliability=Reliability.RELIABLE, durability=Durability.VOLATILE, depth=10)


class ZenohPublisher:
    def __init__(self, session: zenoh.Session, msg_type: Type, topic: str, qos: QoS) -> None:
        self.msg_type = msg_type
        self.topic = topic
        self.qos = qos
        
        # Congestion control mapping to QoS reliability
        cc = zenoh.CongestionControl.BLOCK
        if qos.reliability == Reliability.BEST_EFFORT:
            cc = zenoh.CongestionControl.DROP
            
        self.pub = session.declare_publisher(topic, congestion_control=cc)

    def publish(self, msg) -> None:
        if hasattr(msg, 'serialize'):
            payload = msg.serialize()
        elif isinstance(msg, (dict, list)):
            payload = msgpack.packb(msg)
        else:
            payload = msgpack.packb([msg])
            
        self.pub.put(payload)


class ZenohSubscription:
    def __init__(self, session: zenoh.Session, msg_type: Type, topic: str, callback: Callable[[Any], None], qos: QoS) -> None:
        self.msg_type = msg_type
        self.topic = topic
        self.callback = callback
        self.qos = qos
        
        def internal_callback(sample):
            try:
                raw_payload = sample.payload.to_bytes()
                if hasattr(self.msg_type, 'deserialize'):
                    msg = self.msg_type.deserialize(raw_payload)
                elif self.msg_type in (dict, list):
                    msg = msgpack.unpackb(raw_payload)
                else:
                    msg = msgpack.unpackb(raw_payload)
                self.callback(msg)
            except Exception as e:
                print(f"[ZenohSubscription] Error decoding message on {self.topic}: {e}")

        self.sub = session.declare_subscriber(topic, internal_callback)


class ZenohTimer:
    def __init__(self, period_ms: int, callback: Callable[[], None]) -> None:
        self.period_sec = period_ms / 1000.0
        self.callback = callback
        self.running = True
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()

    def _run(self) -> None:
        while self.running:
            time.sleep(self.period_sec)
            if self.running:
                try:
                    self.callback()
                except Exception as e:
                    print(f"[ZenohTimer] Error in timer callback: {e}")

    def cancel(self) -> None:
        self.running = False


class ZenohNode:
    _session: ClassVar[Optional[zenoh.Session]] = None
    _session_refcount = 0
    _lock = threading.RLock()

    def __init__(self, node_name: str) -> None:
        self.node_name = node_name
        self.publishers = []
        self.subscriptions = []
        self.timers = []

    @classmethod
    def init(cls, config_endpoints: Optional[List[str]] = None) -> None:
        with cls._lock:
            if cls._session is not None:
                cls._session_refcount += 1
                return

            conf = zenoh.Config()
            if config_endpoints is None:
                config_endpoints = ["tcp/192.168.4.1:7447"]

            if config_endpoints:
                conf.insert_json5("connect/endpoints", str(config_endpoints).replace("'", '"'))
            
            cls._session = zenoh.open(conf)
            cls._session_refcount = 1
            print(f"[Zenoh] Global session initialized (endpoints={config_endpoints})")

    @classmethod
    def shutdown(cls) -> None:
        with cls._lock:
            if cls._session is not None:
                cls._session_refcount -= 1
                if cls._session_refcount <= 0:
                    cls._session.close()
                    cls._session = None
                    print("[Zenoh] Global session closed.")

    def z_get_name(self) -> str:
        return self.node_name

    def z_create_publisher(self, msg_type: Type, topic: str, qos: Optional[Union[QoS, int]] = None) -> ZenohPublisher:
        if qos is None:
            qos = QoS()
        elif isinstance(qos, int):
            depth = qos
            qos = QoS()
            qos.depth = depth
            
        with self._lock:
            if self._session is None:
                ZenohNode.init()
            session = self._session
            assert session is not None
            
        pub = ZenohPublisher(session, msg_type, topic, qos)
        self.publishers.append(pub)
        return pub

    def z_create_subscription(self, msg_type: Type, topic: str, callback: Callable[[Any], None], qos: Optional[Union[QoS, int]] = None) -> ZenohSubscription:
        if qos is None:
            qos = QoS()
        elif isinstance(qos, int):
            depth = qos
            qos = QoS()
            qos.depth = depth
            
        with self._lock:
            if self._session is None:
                ZenohNode.init()
            session = self._session
            assert session is not None
            
        sub = ZenohSubscription(session, msg_type, topic, callback, qos)
        self.subscriptions.append(sub)
        return sub

    def z_create_timer(self, period_ms: int, callback: Callable[[], None]) -> ZenohTimer:
        timer = ZenohTimer(period_ms, callback)
        self.timers.append(timer)
        return timer

    def z_spin(self) -> None:
        try:
            while True:
                time.sleep(1.0)
        except KeyboardInterrupt:
            print(f"\n[Node {self.node_name}] Shutdown requested via KeyboardInterrupt.")
            self.z_destroy()

    def z_destroy(self) -> None:
        for timer in self.timers:
            timer.cancel()
        self.timers.clear()
        self.publishers.clear()
        self.subscriptions.clear()
        ZenohNode.shutdown()
