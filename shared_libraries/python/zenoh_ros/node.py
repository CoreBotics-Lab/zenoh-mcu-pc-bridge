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


class Future:
    """ROS 2-style Future for asynchronous service requests."""
    def __init__(self) -> None:
        self._event = threading.Event()
        self._result: Any = None
        self._exception: Optional[Exception] = None

    def done(self) -> bool:
        return self._event.is_set()

    def result(self, timeout_sec: Optional[float] = None) -> Any:
        if self._event.wait(timeout=timeout_sec):
            if self._exception:
                raise self._exception
            return self._result
        raise TimeoutError("Future result timed out.")

    def set_result(self, result: Any) -> None:
        self._result = result
        self._event.set()

    def set_exception(self, exception: Exception) -> None:
        self._exception = exception
        self._event.set()


class ZenohService:
    def __init__(self, session: zenoh.Session, srv_type: Type, service_name: str, callback: Callable[[Any, Any], None]) -> None:
        self.srv_type = srv_type
        self.service_name = service_name
        self.callback = callback

        def queryable_handler(query: zenoh.Query):
            try:
                raw_payload = query.payload.to_bytes() if query.payload else b""
                req_cls = getattr(self.srv_type, 'Request', None)
                res_cls = getattr(self.srv_type, 'Response', None)

                # Skip handling if this is an empty probe query from wait_for_service
                if not raw_payload:
                    query.reply(self.service_name, b"")
                    return

                if req_cls and hasattr(req_cls, 'deserialize'):
                    req = req_cls.deserialize(raw_payload)
                else:
                    req = msgpack.unpackb(raw_payload)

                res = res_cls() if res_cls else {}
                self.callback(req, res)

                if hasattr(res, 'serialize'):
                    reply_payload = res.serialize()
                elif isinstance(res, (dict, list)):
                    reply_payload = msgpack.packb(res)
                else:
                    reply_payload = msgpack.packb([res])

                query.reply(self.service_name, reply_payload)
            except Exception as e:
                print(f"[ZenohService] Error handling service query on '{self.service_name}': {e}")

        self.queryable = session.declare_queryable(service_name, queryable_handler)

    def undeclare(self) -> None:
        if hasattr(self, 'queryable') and self.queryable:
            self.queryable.undeclare()
            self.queryable = None


class ZenohClient:
    def __init__(self, session: zenoh.Session, srv_type: Type, service_name: str) -> None:
        self.session = session
        self.srv_type = srv_type
        self.service_name = service_name

    def wait_for_service(self, timeout_sec: float = 5.0) -> bool:
        """Wait for service queryable to be available on Zenoh network."""
        start = time.time()
        while time.time() - start < timeout_sec:
            try:
                replies = self.session.get(self.service_name, timeout=0.2)
                for reply in replies:
                    if reply and hasattr(reply, 'ok'):
                        return True
            except Exception:
                pass
            time.sleep(0.1)
        return False

    def call_async(self, request: Any) -> Future:
        """Asynchronously call service and return a Future object (like ROS 2)."""
        future = Future()

        def _worker():
            try:
                if hasattr(request, 'serialize'):
                    payload = request.serialize()
                elif isinstance(request, (dict, list)):
                    payload = msgpack.packb(request)
                else:
                    payload = msgpack.packb([request])

                replies = self.session.get(self.service_name, payload=payload)
                raw_reply = None
                for reply in replies:
                    if reply and hasattr(reply, 'ok') and reply.ok:
                        raw_reply = reply.ok.payload.to_bytes()
                        break

                if raw_reply is not None:
                    res_cls = getattr(self.srv_type, 'Response', None)

                    if res_cls and hasattr(res_cls, 'deserialize'):
                        response = res_cls.deserialize(raw_reply)
                    else:
                        response = msgpack.unpackb(raw_reply)
                    future.set_result(response)
                else:
                    future.set_exception(RuntimeError("Service call failed or no response received."))
            except Exception as e:
                future.set_exception(e)

        t = threading.Thread(target=_worker, daemon=True)
        t.start()
        return future

    def call(self, request: Any, timeout_sec: float = 5.0) -> Any:
        """Synchronously call service and return response (or raise TimeoutError)."""
        future = self.call_async(request)
        return future.result(timeout_sec=timeout_sec)


class ZenohConfig:
    def __init__(self, host: str = "192.168.4.1", port: int = 7447, connect_endpoint: str = "") -> None:
        self.host = host
        self.port = port
        self.connect_endpoint = connect_endpoint


class ZenohNode:
    _session: ClassVar[Optional[zenoh.Session]] = None
    _session_refcount = 0
    _lock = threading.RLock()

    def __init__(self, node_name: str) -> None:
        self.node_name = node_name
        self.publishers = []
        self.subscriptions = []
        self.timers = []
        self.services = []
        self.clients = []

    @classmethod
    def init(cls, config: Optional[ZenohConfig] = None) -> None:
        with cls._lock:
            if cls._session is not None:
                cls._session_refcount += 1
                return

            if config is None:
                config = ZenohConfig()

            conf = zenoh.Config()
            
            # Resolve connection endpoints from config
            endpoints = []
            if config.connect_endpoint:
                endpoints = [config.connect_endpoint]
            elif config.host:
                endpoints = [f"tcp/{config.host}:{config.port}"]

            if endpoints:
                conf.insert_json5("connect/endpoints", str(endpoints).replace("'", '"'))
                cls._session = zenoh.open(conf)
                print(f"[Zenoh] Global session initialized (endpoints={endpoints})")
            else:
                cls._session = zenoh.open(conf)
                print("[Zenoh] Global session initialized (scouting for peers)")

            cls._session_refcount = 1

            # Wire the Zenoh session into the logging module so log messages
            # are published to the 'zenoh_ros/log' topic automatically.
            try:
                from .logging import _set_zenoh_session
                _set_zenoh_session(cls._session)
            except Exception:
                pass

    @classmethod
    def shutdown(cls) -> None:
        with cls._lock:
            if cls._session is not None:
                cls._session_refcount -= 1
                if cls._session_refcount <= 0:
                    try:
                        cls._session.close()
                    except Exception as e:
                        print(f"[Zenoh] Notice on session close: {e}")
                    finally:
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

    def z_create_service(self, srv_type: Type, service_name: str, callback: Callable[[Any, Any], None]) -> ZenohService:
        with self._lock:
            if self._session is None:
                ZenohNode.init()
            session = self._session
            assert session is not None

        service = ZenohService(session, srv_type, service_name, callback)
        self.services.append(service)
        return service

    def z_create_client(self, srv_type: Type, service_name: str) -> ZenohClient:
        with self._lock:
            if self._session is None:
                ZenohNode.init()
            session = self._session
            assert session is not None

        client = ZenohClient(session, srv_type, service_name)
        self.clients.append(client)
        return client

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
        for srv in self.services:
            srv.undeclare()
        self.services.clear()
        self.clients.clear()
        self.publishers.clear()
        self.subscriptions.clear()
        ZenohNode.shutdown()
