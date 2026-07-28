import msgpack
from typing import Optional, cast, Any

class Vector3:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0) -> None:
        self.x = x
        self.y = y
        self.z = z

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.z]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'Vector3':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.Vector3(x={self.x}, y={self.y}, z={self.z})"


class Point:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0) -> None:
        self.x = x
        self.y = y
        self.z = z

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.z]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'Point':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.Point(x={self.x}, y={self.y}, z={self.z})"


class Quaternion:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0, w: float = 1.0) -> None:
        self.x = x
        self.y = y
        self.z = z
        self.w = w

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.z, self.w]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'Quaternion':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]), float(data[3]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.Quaternion(x={self.x}, y={self.y}, z={self.z}, w={self.w})"


class Pose:
    def __init__(self, position: Optional[Point] = None, orientation: Optional[Quaternion] = None) -> None:
        self.position = position if position is not None else Point()
        self.orientation = orientation if orientation is not None else Quaternion()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.position.x, self.position.y, self.position.z],
            [self.orientation.x, self.orientation.y, self.orientation.z, self.orientation.w]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'Pose':
        data = msgpack.unpackb(payload)
        pos = Point(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        ori = Quaternion(float(data[1][0]), float(data[1][1]), float(data[1][2]), float(data[1][3]))
        return cls(pos, ori)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.Pose(position={self.position}, orientation={self.orientation})"


class Twist:
    def __init__(self, linear: Optional[Vector3] = None, angular: Optional[Vector3] = None) -> None:
        self.linear = linear if linear is not None else Vector3()
        self.angular = angular if angular is not None else Vector3()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.linear.x, self.linear.y, self.linear.z],
            [self.angular.x, self.angular.y, self.angular.z]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'Twist':
        data = msgpack.unpackb(payload)
        lin = Vector3(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        ang = Vector3(float(data[1][0]), float(data[1][1]), float(data[1][2]))
        return cls(lin, ang)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.Twist(linear={self.linear}, angular={self.angular})"


class Wrench:
    def __init__(self, force: Optional[Vector3] = None, torque: Optional[Vector3] = None) -> None:
        self.force = force if force is not None else Vector3()
        self.torque = torque if torque is not None else Vector3()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.force.x, self.force.y, self.force.z],
            [self.torque.x, self.torque.y, self.torque.z]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'Wrench':
        data = msgpack.unpackb(payload)
        force = Vector3(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        torque = Vector3(float(data[1][0]), float(data[1][1]), float(data[1][2]))
        return cls(force, torque)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.Wrench(force={self.force}, torque={self.torque})"


class Transform:
    def __init__(self, translation: Optional[Vector3] = None, rotation: Optional[Quaternion] = None) -> None:
        self.translation = translation if translation is not None else Vector3()
        self.rotation = rotation if rotation is not None else Quaternion()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.translation.x, self.translation.y, self.translation.z],
            [self.rotation.x, self.rotation.y, self.rotation.z, self.rotation.w]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'Transform':
        data = msgpack.unpackb(payload)
        trans = Vector3(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        rot = Quaternion(float(data[1][0]), float(data[1][1]), float(data[1][2]), float(data[1][3]))
        return cls(trans, rot)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.Transform(translation={self.translation}, rotation={self.rotation})"


class Pose2D:
    def __init__(self, x: float = 0.0, y: float = 0.0, theta: float = 0.0) -> None:
        self.x = x
        self.y = y
        self.theta = theta

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.theta]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'Pose2D':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.Pose2D(x={self.x}, y={self.y}, theta={self.theta})"
