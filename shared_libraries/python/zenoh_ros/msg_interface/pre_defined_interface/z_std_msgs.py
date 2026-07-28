import msgpack
from typing import Any, List, Optional, cast

class Empty:
    def __init__(self) -> None:
        pass

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'Empty':
        return cls()


class Raw:
    def __init__(self, data: bytes = b"") -> None:
        self.data: bytes = data

    def serialize(self) -> bytes:
        return self.data

    @classmethod
    def deserialize(cls, payload: bytes) -> 'Raw':
        return cls(payload)


# --- Helper class factory for primitive message types ---
def _make_primitive_msg(cls_name: str, default_val: Any):
    class PrimitiveMessage:
        def __init__(self, data: Any = default_val) -> None:
            self.data = data

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb([self.data]))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'PrimitiveMessage':
            unpacked = msgpack.unpackb(payload)
            val = unpacked[0] if isinstance(unpacked, (list, tuple)) else unpacked
            return cls(val)

        def __repr__(self) -> str:
            return f"z_std_msgs.{cls_name}(data={self.data})"

    PrimitiveMessage.__name__ = cls_name
    PrimitiveMessage.__qualname__ = cls_name
    return PrimitiveMessage


# --- Helper class factory for array message types ---
def _make_array_msg(cls_name: str):
    class ArrayMessage:
        def __init__(self, data: Optional[List[Any]] = None) -> None:
            self.data = data if data is not None else []

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb(self.data))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'ArrayMessage':
            val = msgpack.unpackb(payload)
            if not isinstance(val, list):
                val = list(val) if isinstance(val, tuple) else [val]
            return cls(val)

        def __repr__(self) -> str:
            return f"z_std_msgs.{cls_name}(data={self.data})"

    ArrayMessage.__name__ = cls_name
    ArrayMessage.__qualname__ = cls_name
    return ArrayMessage


# Create all standard primitives
Bool = _make_primitive_msg("Bool", False)
Byte = _make_primitive_msg("Byte", 0)
Char = _make_primitive_msg("Char", 0)
Int8 = _make_primitive_msg("Int8", 0)
UInt8 = _make_primitive_msg("UInt8", 0)
Int16 = _make_primitive_msg("Int16", 0)
UInt16 = _make_primitive_msg("UInt16", 0)
Int32 = _make_primitive_msg("Int32", 0)
UInt32 = _make_primitive_msg("UInt32", 0)
Int64 = _make_primitive_msg("Int64", 0)
UInt64 = _make_primitive_msg("UInt64", 0)
Float32 = _make_primitive_msg("Float32", 0.0)
Float64 = _make_primitive_msg("Float64", 0.0)
String = _make_primitive_msg("String", "")

# Create all standard arrays
ByteMultiArray = _make_array_msg("ByteMultiArray")
Int8MultiArray = _make_array_msg("Int8MultiArray")
UInt8MultiArray = _make_array_msg("UInt8MultiArray")
Int16MultiArray = _make_array_msg("Int16MultiArray")
UInt16MultiArray = _make_array_msg("UInt16MultiArray")
Int32MultiArray = _make_array_msg("Int32MultiArray")
UInt32MultiArray = _make_array_msg("UInt32MultiArray")
Int64MultiArray = _make_array_msg("Int64MultiArray")
UInt64MultiArray = _make_array_msg("UInt64MultiArray")
Float32MultiArray = _make_array_msg("Float32MultiArray")
Float64MultiArray = _make_array_msg("Float64MultiArray")
