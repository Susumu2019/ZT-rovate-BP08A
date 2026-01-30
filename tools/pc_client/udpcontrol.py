
import socket
import threading
import struct
import tkinter as tk
from tkinter import messagebox
import time
# 3D描画用
import matplotlib
matplotlib.use('TkAgg')
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from mpl_toolkits.mplot3d import Axes3D, art3d
import matplotlib.pyplot as plt
import numpy as np

ROBOT_IP = "192.168.0.11"
ROBOT_PORT = 12345  # ESP32側UDP_LISTEN_PORTと合わせる
LOCAL_PORT = 12346  # PC側の受信ポート

class UdpRobotClient:
    def __init__(self, master):
        self.master = master
        master.title("UDP Robot Controller")
        master.geometry("500x850")

        self.status_var = tk.StringVar(value="Not connected")
        # IMU値用の変数
        self.imu_labels = {}
        self.servo_vars = [tk.IntVar(value=90) for _ in range(8)]

        tk.Label(master, text="Robot IP:").pack()
        self.ip_entry = tk.Entry(master)
        self.ip_entry.insert(0, ROBOT_IP)
        self.ip_entry.pack()

        servo_frame = tk.Frame(master)
        servo_frame.pack(pady=5)

        for i in range(8):
            tk.Label(servo_frame, text=f"Servo {i}").grid(row=i, column=0)
            scale = tk.Scale(servo_frame, from_=0, to=180, orient=tk.HORIZONTAL, variable=self.servo_vars[i], length=200,
                            command=lambda val, idx=i: self.on_slider_change(idx, val))
            scale.grid(row=i, column=1)

        # すべて中立ボタン
        self.neutral_btn = tk.Button(master, text="すべて中立(90度)", command=self.set_all_neutral)
        self.neutral_btn.pack(pady=5)

        # IMU値を表形式で表示
        imu_frame = tk.Frame(master)
        imu_frame.pack(pady=5)
        headers = ["roll", "pitch", "yaw", "gyro_x", "gyro_y", "gyro_z", "temp"]
        for i, h in enumerate(headers):
            tk.Label(imu_frame, text=h, borderwidth=1, relief="solid", width=8, fg="green").grid(row=0, column=i)
            v = tk.StringVar(value="-")
            lbl = tk.Label(imu_frame, textvariable=v, borderwidth=1, relief="solid", width=8)
            lbl.grid(row=1, column=i)
            self.imu_labels[h] = v

        # 3D立方体描画エリア
        self.fig = plt.Figure(figsize=(3, 3))
        self.ax = self.fig.add_subplot(111, projection='3d')
        self.ax.set_xlim([-1, 1])
        self.ax.set_ylim([-1, 1])
        self.ax.set_zlim([-1, 1])
        self.ax.set_box_aspect([1,1,1])
        self.ax.set_axis_off()
        self.cube = None
        self.roll = 0.0
        self.pitch = 0.0
        self.yaw = 0.0
        # 正面面の選択設定
        self.front_face_var = tk.StringVar(value="y-")
        face_options = ["x+", "x-", "y+", "y-"]
        face_frame = tk.Frame(master)
        face_frame.pack(pady=2)
        tk.Label(face_frame, text="Front face:").pack(side=tk.LEFT)
        self.face_menu = tk.OptionMenu(face_frame, self.front_face_var, *face_options, command=lambda _: self._draw_cube())
        self.face_menu.pack(side=tk.LEFT)
        self.canvas = FigureCanvasTkAgg(self.fig, master)
        self.canvas.get_tk_widget().pack(pady=5)
        self._draw_cube()

    def on_slider_change(self, idx, val):
        self.send_servo()


        # すべて中立ボタン
        self.neutral_btn = tk.Button(master, text="すべて中立(90度)", command=self.set_all_neutral)
        self.neutral_btn.pack(pady=5)

        # IMU値を表形式で表示
        imu_frame = tk.Frame(master)
        imu_frame.pack(pady=5)
        headers = ["roll", "pitch", "yaw", "gyro_x", "gyro_y", "gyro_z", "temp"]
        for i, h in enumerate(headers):
            tk.Label(imu_frame, text=h, borderwidth=1, relief="solid", width=8, fg="green").grid(row=0, column=i)
            v = tk.StringVar(value="-")
            lbl = tk.Label(imu_frame, textvariable=v, borderwidth=1, relief="solid", width=8)
            lbl.grid(row=1, column=i)
            self.imu_labels[h] = v

        # 3D立方体描画エリア
        self.fig = plt.Figure(figsize=(3, 3))
        self.ax = self.fig.add_subplot(111, projection='3d')
        self.ax.set_xlim([-1, 1])
        self.ax.set_ylim([-1, 1])
        self.ax.set_zlim([-1, 1])
        self.ax.set_box_aspect([1,1,1])
        self.ax.set_axis_off()
        self.cube = None
        self.roll = 0.0
        self.pitch = 0.0
        self.yaw = 0.0
        # 正面面の選択設定
        self.front_face_var = tk.StringVar(value="y-")
        face_options = ["x+", "x-", "y+", "y-"]
        face_frame = tk.Frame(master)
        face_frame.pack(pady=2)
        tk.Label(face_frame, text="Front face:").pack(side=tk.LEFT)
        self.face_menu = tk.OptionMenu(face_frame, self.front_face_var, *face_options, command=lambda _: self._draw_cube())
        self.face_menu.pack(side=tk.LEFT)
        self.canvas = FigureCanvasTkAgg(self.fig, master)
        self.canvas.get_tk_widget().pack(pady=5)
        self._draw_cube()

    def set_all_neutral(self):
        for v in self.servo_vars:
            v.set(90)
        self.send_servo()

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(("", LOCAL_PORT))
        self.running = True
        self.listen_thread = threading.Thread(target=self.listen_udp, daemon=True)
        self.listen_thread.start()

        self.auto_send = True  # 連続送信ON
        self.send_interval = 0.02  # 20ms周期（50Hz）
        self.auto_send_thread = threading.Thread(target=self.auto_send_loop, daemon=True)
        self.auto_send_thread.start()

    def auto_send_loop(self):
        while self.running and self.auto_send:
            self.send_servo(auto=True)
            time.sleep(self.send_interval)

    def _draw_cube(self):
        # 立方体の8頂点
        r = 0.5
        points = np.array([[-r,-r,-r], [r,-r,-r], [r,r,-r], [-r,r,-r],
                           [-r,-r,r], [r,-r,r], [r,r,r], [-r,r,r]])
        # 回転行列
        Rx = np.array([[1,0,0],[0,np.cos(self.roll),-np.sin(self.roll)],[0,np.sin(self.roll),np.cos(self.roll)]])
        Ry = np.array([[np.cos(self.pitch),0,np.sin(self.pitch)],[0,1,0],[-np.sin(self.pitch),0,np.cos(self.pitch)]])
        Rz = np.array([[np.cos(self.yaw),-np.sin(self.yaw),0],[np.sin(self.yaw),np.cos(self.yaw),0],[0,0,1]])
        R = Rz @ Ry @ Rx
        points_rot = points @ R.T

        # 立方体の6面
        faces = [ [points_rot[j] for j in [0,1,2,3]],   # 底面 (z=-r)
                  [points_rot[j] for j in [4,5,6,7]],   # 上面 (z=+r)
                  [points_rot[j] for j in [0,1,5,4]],   # y- 正面
                  [points_rot[j] for j in [2,3,7,6]],   # y+ 背面
                  [points_rot[j] for j in [1,2,6,5]],   # x+ 右面
                  [points_rot[j] for j in [4,7,3,0]] ]  # x- 左面

        # 正面面の選択に応じて色を割り当て
        front = self.front_face_var.get()
        # faces: [底, 上, y-, y+, x+, x-]
        face_colors = ['lightgray', 'cyan', 'cyan', 'cyan', '#ffcccc', '#cce6ff']
        # 右面(x+)淡赤 #ffcccc, 左面(x-)淡青 #cce6ff, 底面グレー, 正面(y-/y+)はデフォルト(シアン)
        # 正面面の選択は色変更しない（y-/y+はシアンのまま）

        self.ax.cla()
        self.ax.set_xlim([-1, 1])
        self.ax.set_ylim([-1, 1])
        self.ax.set_zlim([-1, 1])
        self.ax.set_box_aspect([1,1,1])
        self.ax.set_axis_off()
        for i, f in enumerate(faces):
            poly3d = art3d.Poly3DCollection([f], facecolors=face_colors[i], edgecolors='k', linewidths=1, alpha=0.7)
            self.ax.add_collection3d(poly3d)

        # XYZ矢印の描画
        origin = np.array([0,0,0])
        axis_len = 0.7
        # 回転後の軸ベクトル
        x_axis = (np.array([axis_len,0,0]) @ R.T)
        y_axis = (np.array([0,axis_len,0]) @ R.T)
        z_axis = (np.array([0,0,axis_len]) @ R.T)
        self.ax.quiver(*origin, *x_axis, color='r', arrow_length_ratio=0.15, linewidth=2)
        self.ax.quiver(*origin, *y_axis, color='g', arrow_length_ratio=0.15, linewidth=2)
        self.ax.quiver(*origin, *z_axis, color='b', arrow_length_ratio=0.15, linewidth=2)

        self.canvas.draw()

    def send_servo(self, auto=False):
        ip = self.ip_entry.get()
        # 送信パケット: SYNC(0xAA55) + 8ch u16リトルエンディアン（角度値0～180をそのまま送信）
        buf = bytearray()
        buf += b'\xAA\x55'
        for v in self.servo_vars:
            angle = v.get()
            if angle < 0: angle = 0
            if angle > 180: angle = 180
            buf += struct.pack('<H', angle)
        print(f"[DEBUG] 送信先: {ip}:{ROBOT_PORT} バイト列: {list(buf)}")
        try:
            self.sock.sendto(buf, (ip, ROBOT_PORT))
            if not auto:
                self.status_var.set(f"Sent servo angles to {ip}")
        except Exception as e:
            if not auto:
                self.status_var.set(f"Send error: {e}")

    def listen_udp(self):
        while self.running:
            try:
                data, addr = self.sock.recvfrom(128)
                print(f"[DEBUG] 受信: {addr} バイト列: {list(data)}")
                # ESP32からのIMUデータ（バイナリ）を受信した場合の例
                if len(data) >= 8 and data[0] == 0xAA and data[1] == 0x55:
                    # 例: [AA 55][roll][pitch][yaw][gx][gy][gz][temp] (float*6+uint8)
                    try:
                        floats = struct.unpack('<6fB', data[2:2+25])
                        roll, pitch, yaw, gx, gy, gz, temp = floats
                        # IMU値を表に反映
                        self.imu_labels["roll"].set(f"{roll:.2f}")
                        self.imu_labels["pitch"].set(f"{pitch:.2f}")
                        self.imu_labels["yaw"].set(f"{yaw:.2f}")
                        self.imu_labels["gyro_x"].set(f"{gx:.2f}")
                        self.imu_labels["gyro_y"].set(f"{gy:.2f}")
                        self.imu_labels["gyro_z"].set(f"{gz:.2f}")
                        self.imu_labels["temp"].set(f"{temp}")

                        # roll, pitch, yaw（degree）→ラジアン変換して3D回転角に反映
                        self.roll = np.deg2rad(roll)
                        self.pitch = np.deg2rad(pitch)
                        self.yaw = np.deg2rad(yaw)
                        self._draw_cube()
                    except Exception:
                        for v in self.imu_labels.values():
                            v.set("-")
                else:
                    # テキスト等
                    self.status_var.set(f"Received from {addr}: {data[:32]!r}")
            except Exception:
                pass

    def on_close(self):
        self.running = False
        if hasattr(self, 'sock') and self.sock:
            self.sock.close()
        self.master.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = UdpRobotClient(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()
