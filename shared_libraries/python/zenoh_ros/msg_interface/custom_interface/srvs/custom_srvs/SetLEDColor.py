import msgpack
from typing import Any, List, Optional, cast

class z_SetLEDColor:
    class Request:
        def __init__(self, led_data: Any = None) -> None:
            self.led_data = led_data

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({"led_data": None}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Request':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {}
            return cls(None)

        def __repr__(self) -> str:
            return f"custom_srvs.z_SetLEDColor.Request(led_data={self.led_data})"

    class Response:
        def __init__(self, success: bool = False, message: str = "") -> None:
            self.success = success
            self.message = message

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({"success": self.success, "message": self.message}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Response':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {}
            return cls(bool(data.get(b"success", data.get("success", False))), str(data.get(b"message", data.get("message", ""))))

        def __repr__(self) -> str:
            return f"custom_srvs.z_SetLEDColor.Response(success={self.success}, message={self.message})"
