import msgpack
from typing import Optional, cast, Any

class z_Vector3:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0) -> None:
        self.x = x
        self.y = y
        self.z = z

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.z]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Vector3':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Vector3(x={self.x}, y={self.y}, z={self.z})"


class z_Point:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0) -> None:
        self.x = x
        self.y = y
        self.z = z

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.z]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Point':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Point(x={self.x}, y={self.y}, z={self.z})"


class z_Quaternion:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0, w: float = 1.0) -> None:
        self.x = x
        self.y = y
        self.z = z
        self.w = w

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.z, self.w]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Quaternion':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]), float(data[3]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Quaternion(x={self.x}, y={self.y}, z={self.z}, w={self.w})"


class z_Pose:
    def __init__(self, position: Optional[z_Point] = None, orientation: Optional[z_Quaternion] = None) -> None:
        self.position = position if position is not None else z_Point()
        self.orientation = orientation if orientation is not None else z_Quaternion()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.position.x, self.position.y, self.position.z],
            [self.orientation.x, self.orientation.y, self.orientation.z, self.orientation.w]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Pose':
        data = msgpack.unpackb(payload)
        pos = z_Point(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        ori = z_Quaternion(float(data[1][0]), float(data[1][1]), float(data[1][2]), float(data[1][3]))
        return cls(pos, ori)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Pose(position={self.position}, orientation={self.orientation})"


class z_Twist:
    def __init__(self, linear: Optional[z_Vector3] = None, angular: Optional[z_Vector3] = None) -> None:
        self.linear = linear if linear is not None else z_Vector3()
        self.angular = angular if angular is not None else z_Vector3()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.linear.x, self.linear.y, self.linear.z],
            [self.angular.x, self.angular.y, self.angular.z]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Twist':
        data = msgpack.unpackb(payload)
        lin = z_Vector3(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        ang = z_Vector3(float(data[1][0]), float(data[1][1]), float(data[1][2]))
        return cls(lin, ang)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Twist(linear={self.linear}, angular={self.angular})"


class z_Wrench:
    def __init__(self, force: Optional[z_Vector3] = None, torque: Optional[z_Vector3] = None) -> None:
        self.force = force if force is not None else z_Vector3()
        self.torque = torque if torque is not None else z_Vector3()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.force.x, self.force.y, self.force.z],
            [self.torque.x, self.torque.y, self.torque.z]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Wrench':
        data = msgpack.unpackb(payload)
        force = z_Vector3(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        torque = z_Vector3(float(data[1][0]), float(data[1][1]), float(data[1][2]))
        return cls(force, torque)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Wrench(force={self.force}, torque={self.torque})"


class z_Transform:
    def __init__(self, translation: Optional[z_Vector3] = None, rotation: Optional[z_Quaternion] = None) -> None:
        self.translation = translation if translation is not None else z_Vector3()
        self.rotation = rotation if rotation is not None else z_Quaternion()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.translation.x, self.translation.y, self.translation.z],
            [self.rotation.x, self.rotation.y, self.rotation.z, self.rotation.w]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Transform':
        data = msgpack.unpackb(payload)
        trans = z_Vector3(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        rot = z_Quaternion(float(data[1][0]), float(data[1][1]), float(data[1][2]), float(data[1][3]))
        return cls(trans, rot)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Transform(translation={self.translation}, rotation={self.rotation})"


class z_Pose2D:
    def __init__(self, x: float = 0.0, y: float = 0.0, theta: float = 0.0) -> None:
        self.x = x
        self.y = y
        self.theta = theta

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.theta]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Pose2D':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Pose2D(x={self.x}, y={self.y}, theta={self.theta})"
