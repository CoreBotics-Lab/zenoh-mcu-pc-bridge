#!/usr/bin/env python3
import sys
import os
import time
import math
import numpy as np

# Ensure shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.sensor_msgs import z_Imu

# Attempt to import pygame
try:
    import pygame
    from pygame.locals import *
except ImportError:
    print("Error: pygame-ce is required. Run: pip install pygame-ce")
    sys.exit(1)

# Global attitude variables (in degrees)
current_roll = 0.0
current_pitch = 0.0
current_yaw = 0.0

last_time = time.time()
latest_msg = None


class MPUSubscriberNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("flight_sim_subscriber")
        self.get_logger().info("Node has been started")
        self.sub = self.z_create_subscription(
            z_Imu,
            "robot/mpu6050",
            self.listener_callback,
            10
        )

    def listener_callback(self, msg: z_Imu) -> None:
        global current_roll, current_pitch, current_yaw, last_time, latest_msg
        latest_msg = msg

        now = time.time()
        dt = now - last_time
        if dt <= 0 or dt > 0.5:
            dt = 0.05
        last_time = now

        # 1. Accelerometer Pitch and Roll estimation (in radians)
        acc_x = msg.linear_acceleration.x
        acc_y = msg.linear_acceleration.y
        acc_z = msg.linear_acceleration.z
        
        # Standard MPU6050 pitch & roll from gravity vector
        roll_acc = math.atan2(acc_y, acc_z)
        pitch_acc = math.atan2(-acc_x, math.sqrt(acc_y * acc_y + acc_z * acc_z))

        # 2. Convert Gyro values from rad/s to deg/s
        gyro_x_deg = math.degrees(msg.angular_velocity.x)  # Roll rate
        gyro_y_deg = math.degrees(msg.angular_velocity.y)  # Pitch rate
        gyro_z_deg = math.degrees(msg.angular_velocity.z)  # Yaw rate

        # 3. Complementary Filter (96% Gyro + 4% Accelerometer)
        current_roll = 0.96 * (current_roll + gyro_x_deg * dt) + 0.04 * math.degrees(roll_acc)
        current_pitch = 0.96 * (current_pitch + gyro_y_deg * dt) + 0.04 * math.degrees(pitch_acc)
        current_yaw += gyro_z_deg * dt

        # Keep yaw in range [-180, 180]
        if current_yaw > 180.0: current_yaw -= 360.0
        elif current_yaw < -180.0: current_yaw += 360.0


# Define 3D Airplane Mesh Geometry (Scaled Compact Jet)
PLANE_VERTICES = np.array([
    # Nose & Fuselage (Pointing along +Z)
    [0.0, 0.0, 2.5],       # 0: Sharp Nose tip
    [-0.3, 0.25, 0.6],     # 1: Left canopy top
    [0.3, 0.25, 0.6],      # 2: Right canopy top
    [0.0, -0.25, 0.6],     # 3: Cockpit belly
    [0.0, 0.35, -1.8],     # 4: Rear fuselage top
    [0.0, -0.15, -1.8],    # 5: Rear fuselage bottom
    
    # Swept Main Wings (Extending along X)
    [-3.0, 0.0, -0.2],     # 6: Left wingtip
    [3.0, 0.0, -0.2],      # 7: Right wingtip
    [-0.4, 0.0, 0.8],      # 8: Left wing root front
    [0.4, 0.0, 0.8],       # 9: Right wing root front
    [-0.4, 0.0, -0.5],     # 10: Left wing root back
    [0.4, 0.0, -0.5],      # 11: Right wing root back
    
    # Twin Vertical Tail Fins (Extending UP +Y)
    [-0.5, 1.0, -1.8],     # 12: Left tail fin tip
    [0.5, 1.0, -1.8],      # 13: Right tail fin tip
    [-0.25, 0.35, -1.1],   # 14: Left tail fin root
    [0.25, 0.35, -1.1],    # 15: Right tail fin root
    
    # Horizontal Tail Stabilizers
    [-1.2, 0.0, -1.8],     # 16: Left tail wingtip
    [1.2, 0.0, -1.8],      # 17: Right tail wingtip
], dtype=np.float32)

PLANE_EDGES = [
    # Fuselage
    (0, 1), (0, 2), (0, 3), (1, 4), (2, 4), (3, 5), (4, 5), (1, 2), (2, 3), (3, 1),
    # Main Swept Wings
    (8, 6), (6, 10), (10, 8),
    (9, 7), (7, 11), (11, 9),
    # Twin Tail Fins
    (14, 12), (12, 4), (15, 13), (13, 4),
    # Tail Stabilizers
    (4, 16), (4, 17)
]

PLANE_FACES = [
    # Cockpit Glass
    ([0, 1, 2], (80, 180, 240)),   # Cyan Glass
    # Left Fuselage
    ([0, 1, 3], (45, 55, 75)),     # Stealth Grey
    # Right Fuselage
    ([0, 2, 3], (45, 55, 75)),
    # Rear Fuselage
    ([1, 2, 4], (60, 70, 90)),
    # Left Wing (Red accent)
    ([8, 6, 10], (220, 50, 50)),
    # Right Wing (Red accent)
    ([9, 7, 11], (220, 50, 50)),
    # Left Tail Fin
    ([14, 12, 4], (240, 190, 40)), # Yellow fin
    # Right Tail Fin
    ([15, 13, 4], (240, 190, 40)),
    # Horizontal Stabilizers
    ([4, 16, 5], (120, 130, 150)),
    ([4, 17, 5], (120, 130, 150))
]


def rotation_matrix(roll_deg, pitch_deg, yaw_deg):
    """Calculates 3D Euler Rotation Matrix (Roll=Z, Pitch=X, Yaw=Y)."""
    r = math.radians(roll_deg)
    p = math.radians(pitch_deg)
    y = math.radians(yaw_deg)

    # Roll around Z (Nose-Tail axis)
    R_roll = np.array([
        [math.cos(r), -math.sin(r), 0],
        [math.sin(r), math.cos(r), 0],
        [0, 0, 1]
    ])

    # Pitch around X (Wing-to-Wing axis)
    R_pitch = np.array([
        [1, 0, 0],
        [0, math.cos(p), -math.sin(p)],
        [0, math.sin(p), math.cos(p)]
    ])

    # Yaw around Y (Up-Down axis)
    R_yaw = np.array([
        [math.cos(y), 0, math.sin(y)],
        [0, 1, 0],
        [-math.sin(y), 0, math.cos(y)]
    ])

    return R_yaw @ R_pitch @ R_roll


def project_3d_to_2d(vertex, R, width, height, scale=45, distance=6.0):
    """Rotates 3D vertex and projects it onto 2D screen coordinates."""
    rotated = R @ vertex
    x, y, z = rotated[0], rotated[1], rotated[2]
    
    # Perspective projection
    fz = distance / (distance - z + 0.001)
    screen_x = int(width / 2 + x * scale * fz)
    screen_y = int(height / 2 - y * scale * fz)
    return screen_x, screen_y, z


def draw_artificial_horizon(surface, rect, roll, pitch):
    """Draws a professional aviation Attitude Indicator with circular alpha mask."""
    radius = rect.width // 2

    # Invert roll and pitch for authentic cockpit perspective (horizon rotates opposite to aircraft bank)
    horizon_roll = -roll
    horizon_pitch = -pitch

    # 1. Main gauge surface with Alpha channel (SRCALPHA)
    gauge_surf = pygame.Surface((rect.width, rect.height), pygame.SRCALPHA)

    # 2. Pitch / Roll horizon background
    horizon_size = rect.width * 2
    p_surf = pygame.Surface((horizon_size, horizon_size))
    p_surf.fill((60, 140, 220)) # Sky Blue

    # Pitch offset (-50 to +50 deg mapped to pixels)
    pitch_offset = int(horizon_pitch * 2.0)
    
    # Earth Brown ground half
    pygame.draw.rect(p_surf, (130, 80, 40), (0, horizon_size // 2 + pitch_offset, horizon_size, horizon_size))
    # White Horizon Line
    pygame.draw.line(p_surf, (255, 255, 255), (0, horizon_size // 2 + pitch_offset), (horizon_size, horizon_size // 2 + pitch_offset), 4)

    # Pitch ladder markings (-60 deg to +60 deg)
    for p_deg in range(-60, 65, 10):
        if p_deg == 0: continue
        line_y = horizon_size // 2 + pitch_offset - int(p_deg * 2.0)
        w_len = 25 if abs(p_deg) % 20 == 0 else 14
        center_x = horizon_size // 2
        pygame.draw.line(p_surf, (255, 255, 255), (center_x - w_len, line_y), (center_x + w_len, line_y), 2)

    # Rotate horizon surface by inverted roll angle
    rot_p_surf = pygame.transform.rotate(p_surf, horizon_roll)
    rot_rect = rot_p_surf.get_rect(center=(radius, radius))
    gauge_surf.blit(rot_p_surf, rot_rect.topleft)

    # 3. Apply Circular Alpha Mask (Cuts off corners perfectly!)
    mask_surf = pygame.Surface((rect.width, rect.height), pygame.SRCALPHA)
    pygame.draw.circle(mask_surf, (255, 255, 255, 255), (radius, radius), radius - 3)
    gauge_surf.blit(mask_surf, (0, 0), special_flags=pygame.BLEND_RGBA_MIN)

    # 4. Fixed Aircraft Symbol (Yellow reticle)
    reticle_color = (255, 220, 0)
    pygame.draw.circle(gauge_surf, reticle_color, (radius, radius), 4)
    pygame.draw.line(gauge_surf, reticle_color, (radius - 40, radius), (radius - 15, radius), 4)
    pygame.draw.line(gauge_surf, reticle_color, (radius + 15, radius), (radius + 40, radius), 4)
    pygame.draw.line(gauge_surf, reticle_color, (radius - 15, radius), (radius - 15, radius + 8), 4)
    pygame.draw.line(gauge_surf, reticle_color, (radius + 15, radius), (radius + 15, radius + 8), 4)

    # 5. Bezel Ring
    pygame.draw.circle(gauge_surf, (220, 220, 220), (radius, radius), radius - 2, 4)
    pygame.draw.circle(gauge_surf, (15, 15, 15), (radius, radius), radius, 2)

    surface.blit(gauge_surf, rect.topleft)


def main():
    # Initialize Zenoh Node
    cfg = ZenohConfig(host="192.168.4.1", port=7447)
    if not ZenohNode.init(cfg):
        return
    
    node = MPUSubscriberNode()
    
    # Initialize Pygame Window
    pygame.init()
    width, height = 1000, 700
    screen = pygame.display.set_mode((width, height), DOUBLEBUF)
    pygame.display.set_caption("ESP32-S3 MPU6050 3D Flight Simulator")
    clock = pygame.time.Clock()

    font = pygame.font.SysFont("monospace", 16, bold=True)
    title_font = pygame.font.SysFont("monospace", 20, bold=True)

    running = True
    node.get_logger().info("3D Renderer started. Press ESC or close window to exit.")

    while running:
        # Handle Pygame Events
        for event in pygame.event.get():
            if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
                running = False

        # Clear Screen (Dark cockpit background)
        screen.fill((18, 22, 30))

        # 1. Render 3D Airplane Model
        R = rotation_matrix(current_roll, current_pitch, current_yaw)
        
        projected_points = []
        for v in PLANE_VERTICES:
            px, py, z = project_3d_to_2d(v, R, width - 200, height, scale=110, distance=6.0)
            projected_points.append((px, py, z))

        # Render 3D Polygon Faces sorted by Z depth
        face_list = []
        for face_indices, color in PLANE_FACES:
            pts = [projected_points[idx] for idx in face_indices]
            avg_z = sum(p[2] for p in pts) / len(pts)
            face_list.append((avg_z, pts, color))
        
        face_list.sort(key=lambda item: item[0]) # Back-to-front sorting

        for avg_z, pts, color in face_list:
            poly_2d = [(p[0], p[1]) for p in pts]
            pygame.draw.polygon(screen, color, poly_2d)
            pygame.draw.polygon(screen, (30, 30, 30), poly_2d, 1)

        # Render Wireframe Edges
        for edge in PLANE_EDGES:
            p1 = projected_points[edge[0]]
            p2 = projected_points[edge[1]]
            pygame.draw.line(screen, (240, 240, 240), (p1[0], p1[1]), (p2[0], p2[1]), 2)

        # 2. Render Artificial Horizon Gauge (Attitude Indicator)
        gauge_rect = pygame.Rect(width - 220, 20, 200, 200)
        draw_artificial_horizon(screen, gauge_rect, current_roll, current_pitch)

        # 3. Draw Telemetry Dashboard Box
        dash_rect = pygame.Rect(20, 20, 320, 220)
        pygame.draw.rect(screen, (28, 36, 48), dash_rect, border_radius=8)
        pygame.draw.rect(screen, (70, 130, 220), dash_rect, width=2, border_radius=8)

        txt_title = title_font.render("IMU FLIGHT TELEMETRY", True, (240, 240, 240))
        screen.blit(txt_title, (35, 32))

        lines = [
            f"ROLL  (Tilt L/R): {current_roll:6.1f}°",
            f"PITCH (Nose U/D): {current_pitch:6.1f}°",
            f"YAW   (Heading) : {current_yaw:6.1f}°",
            "----------------------------",
        ]
        if latest_msg:
            lines.extend([
                f"Accel: ({latest_msg.linear_acceleration.x:5.1f}, {latest_msg.linear_acceleration.y:5.1f}, {latest_msg.linear_acceleration.z:5.1f}) m/s²",
                f"Gyro : ({latest_msg.angular_velocity.x:5.1f}, {latest_msg.angular_velocity.y:5.1f}, {latest_msg.angular_velocity.z:5.1f}) rad/s"
            ])
        else:
            lines.append("Status: Waiting for Zenoh stream...")

        y_offset = 65
        for line in lines:
            txt = font.render(line, True, (0, 220, 180) if "°" in line else (180, 200, 220))
            screen.blit(txt, (35, y_offset))
            y_offset += 24

        # Update Display
        pygame.display.flip()
        clock.tick(60)

    # Clean Pygame & Zenoh Shutdown
    pygame.quit()
    if node:
        node.get_logger().info("Closed successfully.")
    ZenohNode.shutdown()


if __name__ == '__main__':
    import signal
    signal.signal(signal.SIGINT, lambda sig, frame: sys.exit(0))
    try:
        main()
    except (KeyboardInterrupt, SystemExit):
        sys.exit(0)
