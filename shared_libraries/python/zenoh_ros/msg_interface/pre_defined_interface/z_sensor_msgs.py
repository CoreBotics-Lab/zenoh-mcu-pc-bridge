import msgpack
from typing import List, Optional, cast
from .z_std_msgs import z_Header
from .z_geometry_msgs import z_Vector3, z_Quaternion, z_Pose, z_Twist
from .z_builtin_interfaces import z_Time

class z_Imu:
    def __init__(self,
                 header: Optional[z_Header] = None,
                 orientation: Optional[z_Quaternion] = None,
                 orientation_covariance: Optional[List[float]] = None,
                 angular_velocity: Optional[z_Vector3] = None,
                 angular_velocity_covariance: Optional[List[float]] = None,
                 linear_acceleration: Optional[z_Vector3] = None,
                 linear_acceleration_covariance: Optional[List[float]] = None) -> None:
        self.header = header if header is not None else z_Header()
        self.orientation = orientation if orientation is not None else z_Quaternion()
        self.orientation_covariance = orientation_covariance if orientation_covariance is not None else [0.0]*9
        self.angular_velocity = angular_velocity if angular_velocity is not None else z_Vector3()
        self.angular_velocity_covariance = angular_velocity_covariance if angular_velocity_covariance is not None else [0.0]*9
        self.linear_acceleration = linear_acceleration if linear_acceleration is not None else z_Vector3()
        self.linear_acceleration_covariance = linear_acceleration_covariance if linear_acceleration_covariance is not None else [0.0]*9

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "orientation": {"x": self.orientation.x, "y": self.orientation.y, "z": self.orientation.z, "w": self.orientation.w},
            "orientation_covariance": self.orientation_covariance,
            "angular_velocity": {"x": self.angular_velocity.x, "y": self.angular_velocity.y, "z": self.angular_velocity.z},
            "angular_velocity_covariance": self.angular_velocity_covariance,
            "linear_acceleration": {"x": self.linear_acceleration.x, "y": self.linear_acceleration.y, "z": self.linear_acceleration.z},
            "linear_acceleration_covariance": self.linear_acceleration_covariance
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Imu':
        data = msgpack.unpackb(payload)
        if not isinstance(data, dict): data = {}
        hdr_data = data.get(b"header", data.get("header", {}))
        stamp_data = hdr_data.get(b"stamp", hdr_data.get("stamp", {}))
        header = z_Header(
            stamp=z_Time(sec=int(stamp_data.get(b"sec", stamp_data.get("sec", 0))), nanosec=int(stamp_data.get(b"nanosec", stamp_data.get("nanosec", 0)))),
            frame_id=str(hdr_data.get(b"frame_id", hdr_data.get("frame_id", "")))
        )
        ori_d = data.get(b"orientation", data.get("orientation", {}))
        ori = z_Quaternion(
            float(ori_d.get(b"x", ori_d.get("x", 0.0))),
            float(ori_d.get(b"y", ori_d.get("y", 0.0))),
            float(ori_d.get(b"z", ori_d.get("z", 0.0))),
            float(ori_d.get(b"w", ori_d.get("w", 1.0)))
        )
        ang_d = data.get(b"angular_velocity", data.get("angular_velocity", {}))
        ang = z_Vector3(
            float(ang_d.get(b"x", ang_d.get("x", 0.0))),
            float(ang_d.get(b"y", ang_d.get("y", 0.0))),
            float(ang_d.get(b"z", ang_d.get("z", 0.0)))
        )
        lin_d = data.get(b"linear_acceleration", data.get("linear_acceleration", {}))
        lin = z_Vector3(
            float(lin_d.get(b"x", lin_d.get("x", 0.0))),
            float(lin_d.get(b"y", lin_d.get("y", 0.0))),
            float(lin_d.get(b"z", lin_d.get("z", 0.0)))
        )
        return cls(header=header, orientation=ori, angular_velocity=ang, linear_acceleration=lin)

    def __repr__(self) -> str:
        return f"z_sensor_msgs.z_Imu(header={self.header}, orientation={self.orientation}, angular_velocity={self.angular_velocity}, linear_acceleration={self.linear_acceleration})"


class z_Temperature:
    def __init__(self, header: Optional[z_Header] = None, temperature: float = 0.0, variance: float = 0.0) -> None:
        self.header = header if header is not None else z_Header()
        self.temperature = temperature
        self.variance = variance

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "temperature": self.temperature,
            "variance": self.variance
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Temperature':
        data = msgpack.unpackb(payload)
        if not isinstance(data, dict): data = {}
        hdr_data = data.get(b"header", data.get("header", {}))
        stamp_data = hdr_data.get(b"stamp", hdr_data.get("stamp", {}))
        header = z_Header(
            stamp=z_Time(sec=int(stamp_data.get(b"sec", stamp_data.get("sec", 0))), nanosec=int(stamp_data.get(b"nanosec", stamp_data.get("nanosec", 0)))),
            frame_id=str(hdr_data.get(b"frame_id", hdr_data.get("frame_id", "")))
        )
        return cls(header=header, temperature=float(data.get(b"temperature", data.get("temperature", 0.0))), variance=float(data.get(b"variance", data.get("variance", 0.0))))

    def __repr__(self) -> str:
        return f"z_sensor_msgs.z_Temperature(header={self.header}, temperature={self.temperature}, variance={self.variance})"


class z_Range:
    def __init__(self, header: Optional[z_Header] = None, radiation_type: int = 0, field_of_view: float = 0.0, min_range: float = 0.0, max_range: float = 0.0, range: float = 0.0) -> None:
        self.header = header if header is not None else z_Header()
        self.radiation_type = radiation_type
        self.field_of_view = field_of_view
        self.min_range = min_range
        self.max_range = max_range
        self.range = range

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "radiation_type": self.radiation_type,
            "field_of_view": self.field_of_view,
            "min_range": self.min_range,
            "max_range": self.max_range,
            "range": self.range
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Range':
        data = msgpack.unpackb(payload)
        if not isinstance(data, dict): data = {}
        hdr_data = data.get(b"header", data.get("header", {}))
        stamp_data = hdr_data.get(b"stamp", hdr_data.get("stamp", {}))
        header = z_Header(
            stamp=z_Time(sec=int(stamp_data.get(b"sec", stamp_data.get("sec", 0))), nanosec=int(stamp_data.get(b"nanosec", stamp_data.get("nanosec", 0)))),
            frame_id=str(hdr_data.get(b"frame_id", hdr_data.get("frame_id", "")))
        )
        return cls(
            header=header,
            radiation_type=int(data.get(b"radiation_type", data.get("radiation_type", 0))),
            field_of_view=float(data.get(b"field_of_view", data.get("field_of_view", 0.0))),
            min_range=float(data.get(b"min_range", data.get("min_range", 0.0))),
            max_range=float(data.get(b"max_range", data.get("max_range", 0.0))),
            range=float(data.get(b"range", data.get("range", 0.0)))
        )

    def __repr__(self) -> str:
        return f"z_sensor_msgs.z_Range(header={self.header}, range={self.range})"
