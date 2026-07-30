#!/usr/bin/env python3
import sys
import os
import time
import math
import numpy as np

# Ensure shared zenoh_ros package can be imported
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.custom_msgs import z_MPU6050Data

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
        print(f"[Node] {self.z_get_name()} started")
        self.sub = self.z_create_subscription(
            z_MPU6050Data,
            "robot/mpu6050",
            self.listener_callback,
            10
        )

    def listener_callback(self, msg: z_MPU6050Data) -> None:
        global current_roll, current_pitch, current_yaw, last_time, latest_msg
        latest_msg = msg

        now = time.time()
        dt = now - last_time
        if dt <= 0 or dt > 0.5:
            dt = 0.05
        last_time = now

        # 1. Accelerometer Pitch and Roll estimation (in radians)
        acc_x, acc_y, acc_z = msg.accel_x, msg.accel_y, msg.accel_z
        
        # Standard MPU6050 pitch & roll from gravity vector
        roll_acc = math.atan2(acc_y, acc_z)
        pitch_acc = math.atan2(-acc_x, math.sqrt(acc_y * acc_y + acc_z * acc_z))

        # 2. Convert Gyro values from rad/s to deg/s
        gyro_x_deg = math.degrees(msg.gyro_x) # Roll rate
        gyro_y_deg = math.degrees(msg.gyro_y) # Pitch rate
        gyro_z_deg = math.degrees(msg.gyro_z) # Yaw rate

        # 3. Complementary Filter (96% Gyro + 4% Accelerometer)
        current_roll = 0.96 * (current_roll + gyro_x_deg * dt) + 0.04 * math.degrees(roll_acc)
        current_pitch = 0.96 * (current_pitch + gyro_y_deg * dt) + 0.04 * math.degrees(pitch_acc)
        current_yaw += gyro_z_deg * dt

        # Keep yaw in range [-180, 180]
        if current_yaw > 180.0: current_yaw -= 360.0
        elif current_yaw < -180.0: current_yaw += 360.0


# Define 3D Airplane Mesh Geometry (Vertices & Polygons)
PLANE_VERTICES = np.array([
    # Nose & Fuselage
    [0.0, 0.0, 2.5],      # 0: Nose tip
    [-0.3, -0.2, 0.5],    # 1: Left canopy
    [0.3, -0.2, 0.5],     # 2: Right canopy
    [0.0, 0.3, 0.5],      # 3: Bottom cockpit
    [0.0, -0.4, -1.8],    # 4: Tail top
    [0.0, 0.2, -1.8],     # 5: Tail bottom
    
    # Main Wings
    [-3.2, 0.0, 0.2],     # 6: Left wingtip
    [3.2, 0.0, 0.2],      # 7: Right wingtip
    [-0.4, 0.0, 0.8],     # 8: Left wing root front
    [0.4, 0.0, 0.8],      # 9: Right wing root front
    [-0.4, 0.0, -0.4],    # 10: Left wing root back
    [0.4, 0.0, -0.4],     # 11: Right wing root back
    
    # Vertical Tail Fin
    [0.0, -1.2, -1.8],    # 12: Vertical stabilizer top
    [0.0, -0.4, -1.0],    # 13: Vertical stabilizer root
    
    # Horizontal Tail Wings
    [-1.2, 0.0, -1.8],    # 14: Left tail wingtip
    [1.2, 0.0, -1.8],     # 15: Right tail wingtip
], dtype=np.float32)

PLANE_EDGES = [
    # Fuselage
    (0, 1), (0, 2), (0, 3), (1, 4), (2, 4), (3, 5), (4, 5), (1, 2), (2, 3), (3, 1),
    # Left Wing
    (8, 6), (6, 10), (10, 8),
    # Right Wing
    (9, 7), (7, 11), (11, 9),
    # Tail Fin
    (13, 12), (12, 4),
    # Tail Wings
    (4, 14), (4, 15)
]

PLANE_FACES = [
    # Top Nose
    ([0, 1, 2], (50, 120, 220)),
    # Left Fuselage
    ([0, 1, 3], (40, 100, 190)),
    # Right Fuselage
    ([0, 2, 3], (40, 100, 190)),
    # Main Wings
    ([8, 6, 10], (220, 60, 60)),  # Left wing (Red accent)
    ([9, 7, 11], (220, 60, 60)),  # Right wing (Red accent)
    # Tail Fin
    ([13, 12, 4], (240, 200, 40)), # Yellow fin
    # Tail Wings
    ([4, 14, 5], (180, 180, 180)),
    ([4, 15, 5], (180, 180, 180))
]


def rotation_matrix(roll_deg, pitch_deg, yaw_deg):
    """Calculates 3D Rotation Matrix for Roll, Pitch, Yaw in degrees."""
    r = math.radians(roll_deg)
    p = math.radians(pitch_deg)
    y = math.radians(yaw_deg)

    # Roll (X-axis)
    Rx = np.array([
        [1, 0, 0],
        [0, math.cos(r), -math.sin(r)],
        [0, math.sin(r), math.cos(r)]
    ])

    # Pitch (Y-axis)
    Ry = np.array([
        [math.cos(p), 0, math.sin(p)],
        [0, 1, 0],
        [-math.sin(p), 0, math.cos(p)]
    ])

    # Yaw (Z-axis)
    Rz = np.array([
        [math.cos(y), -math.sin(y), 0],
        [math.sin(y), math.cos(y), 0],
        [0, 0, 1]
    ])

    return Rz @ Ry @ Rx


def project_3d_to_2d(vertex, R, width, height, scale=120, distance=6.0):
    """Rotates 3D vertex and projects it onto 2D screen coordinates."""
    rotated = R @ vertex
    x, y, z = rotated[0], rotated[1], rotated[2]
    
    # Perspective projection
    fz = distance / (distance - z + 0.001)
    screen_x = int(width / 2 + x * scale * fz)
    screen_y = int(height / 2 - y * scale * fz)
    return screen_x, screen_y, z


def draw_artificial_horizon(surface, rect, roll, pitch):
    """Draws a classic aviation Attitude Indicator / Artificial Horizon gauge."""
    cx, cy, radius = rect.centerx, rect.centery, rect.width // 2
    
    # Create circular clip mask
    horizon_surf = pygame.Surface((rect.width, rect.height))
    horizon_surf.fill((30, 30, 30))

    # Horizon background (Sky Blue top, Earth Brown bottom)
    sky_color = (60, 140, 220)
    ground_color = (130, 80, 40)

    # Offset horizon line based on pitch angle (-40 to +40 deg)
    pitch_offset = int(pitch * 2.5)
    
    # Create pitch surface
    p_surf = pygame.Surface((rect.width * 2, rect.height * 2))
    p_surf.fill(sky_color)
    pygame.draw.rect(p_surf, ground_color, (0, rect.height + pitch_offset, rect.width * 2, rect.height * 2))
    
    # Pitch ladder lines
    for p_deg in range(-60, 65, 10):
        if p_deg == 0:
            line_y = rect.height + pitch_offset
            pygame.draw.line(p_surf, (255, 255, 255), (rect.width // 2, line_y), (rect.width * 3 // 2, line_y), 3)
        else:
            line_y = rect.height + pitch_offset - int(p_deg * 2.5)
            w_len = 30 if abs(p_deg) % 20 == 0 else 18
            pygame.draw.line(p_surf, (255, 255, 255), (rect.width - w_len, line_y), (rect.width + w_len, line_y), 2)

    # Rotate horizon surface by roll angle
    rot_p_surf = pygame.transform.rotate(p_surf, roll)
    rot_rect = rot_p_surf.get_rect(center=(radius, radius))
    horizon_surf.blit(rot_p_surf, rot_rect.topleft)

    # Fixed Aircraft Symbol (Yellow reticle)
    reticle_color = (255, 220, 0)
    pygame.draw.circle(horizon_surf, reticle_color, (radius, radius), 4)
    pygame.draw.line(horizon_surf, reticle_color, (radius - 40, radius), (radius - 15, radius), 4)
    pygame.draw.line(horizon_surf, reticle_color, (radius + 15, radius), (radius + 40, radius), 4)
    pygame.draw.line(horizon_surf, reticle_color, (radius - 15, radius), (radius - 15, radius + 10), 4)
    pygame.draw.line(horizon_surf, reticle_color, (radius + 15, radius), (radius + 15, radius + 10), 4)

    # Draw outer bezel
    pygame.draw.circle(horizon_surf, (200, 200, 200), (radius, radius), radius, 3)

    surface.blit(horizon_surf, rect.topleft)


def main():
    # Initialize Zenoh Node
    config = ZenohConfig(host="192.168.4.1", port=7447)
    ZenohNode.init(config)
    
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
    print("[Flight Sim] 3D Renderer started. Press ESC or close window to exit.")

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
                f"Accel: ({latest_msg.accel_x:5.1f}, {latest_msg.accel_y:5.1f}, {latest_msg.accel_z:5.1f}) m/s²",
                f"Gyro : ({latest_msg.gyro_x:5.1f}, {latest_msg.gyro_y:5.1f}, {latest_msg.gyro_z:5.1f}) rad/s"
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

    # Cleanup
    pygame.quit()
    node.z_destroy()
    print("[Flight Sim] Closed successfully.")


if __name__ == '__main__':
    main()
