import math
import struct
import threading
import queue
import json
import argparse
import serial
import serial.tools.list_ports
import pygame
import pyperclip
import time
import struct
import threading
import queue
import json
import argparse
import serial
import serial.tools.list_ports
import pygame
import pyperclip

# --- 定数 ---
CUBE_SIZE = 60
SLIDER_MIN = 500
SLIDER_MAX = 2500
SLIDER_SEND_INTERVAL = 0.04
SERVO_COUNT = 8
RX_HISTORY_MAX = 4096

# --- CRC16-CCITT ---
def crc16_ccitt(data: bytes, poly=0x1021, init=0xFFFF) -> int:
    crc = init
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ poly
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc

# --- IMUデータクラス ---
class ImuData:
    def __init__(self):
        self.ax = 0.0
        self.ay = 0.0
        self.az = 0.0
        self.gx = 0.0
        self.gy = 0.0
        self.gz = 0.0
        self.temp = 0.0
        self.seq = 0

# --- シリアルクライアント ---
class SerialClient(threading.Thread):
    def __init__(self, port, baud, binary_mode=False, tx_history=None):
        super().__init__(daemon=True)
        self.port = port
        self.baud = baud
        self.binary_mode = binary_mode
        self.tx_history = tx_history if tx_history is not None else []
        self.rx_history = bytearray()
        self.rx_raw_history = bytearray()
        self.recv_queue = queue.Queue()
        self._stop_event = threading.Event()
        self.ser = None

    def open(self):
        self.ser = serial.Serial(self.port, self.baud, timeout=0.05)
        self.start()

    def close(self):
        self._stop_event.set()
        if self.ser:
            self.ser.close()

    def run(self):
        while not self._stop_event.is_set():
            try:
                if self.ser.in_waiting:
                    data = self.ser.read(self.ser.in_waiting)
                    self.rx_raw_history += data
                    if self.binary_mode:
                        # バイナリ: パケット分解
                        self._parse_binary(data)
                    else:
                        # テキスト: 行単位
                        lines = data.decode(errors='ignore').split('\n')
                        for line in lines:
                            if line.strip():
                                self.recv_queue.put(line.strip())
                    # RX履歴管理
                    self.rx_history += data
                    if len(self.rx_history) > RX_HISTORY_MAX:
                        self.rx_history = self.rx_history[-RX_HISTORY_MAX:]
            except Exception as e:
                pass
            time.sleep(0.01)

    def _parse_binary(self, data):
        # [AA55][VER][TYPE][SEQ(2)][LEN(2)][PAYLOAD][CRC(2)][ETX(1)]
        buf = getattr(self, '_binbuf', b'') + data
        while len(buf) >= 12:  # 最小フレーム長
            if buf[0:2] != b'\xAA\x55':
                buf = buf[1:]
                continue
            if len(buf) < 8:
                break
            VER = buf[2]
            TYPE = buf[3]
            SEQ = struct.unpack('<H', buf[4:6])[0]
            LEN = struct.unpack('<H', buf[6:8])[0]
            total_len = 2+1+1+2+2+LEN+2+1  # ヘッダ+ペイロード+CRC+ETX
            if len(buf) < total_len:
                break
            pkt = buf[:total_len]
            payload = pkt[8:8+LEN]
            crc_recv = struct.unpack('<H', pkt[8+LEN:8+LEN+2])[0]
            etx = pkt[-1]
            crc_calc = crc16_ccitt(pkt[2:8+LEN])  # VER～PAYLOAD
            if crc_recv == crc_calc and etx == 0x7E:
                try:
                    text = payload.decode(errors='ignore')
                    self.recv_queue.put(text)
                except Exception:
                    pass
                buf = buf[total_len:]
            else:
                buf = buf[1:]
        self._binbuf = buf

    def send_json(self, obj):
        # バイナリコマンド送信（マイコン仕様）
        if self.binary_mode and isinstance(obj, dict):
            VER = 0x01
            TYPE = 0x02  # TYPE_COMMAND
            if not hasattr(self, '_seq'):
                self._seq = 0
            self._seq = (self._seq + 1) & 0xFFFF
            SEQ = self._seq
            # --- コマンド種別 ---
            if obj.get('cmd') == 'ping':
                payload = bytes([0x04])
                desc = 'PING(BIN)'
            elif obj.get('cmd') == 'set' and 'id' in obj and 'val' in obj:
                # 1chサーボ
                payload = bytes([0x01, obj['id'], obj['val'] & 0xFF, (obj['val'] >> 8) & 0xFF])
                desc = f'SET_SERVO({obj["id"]},{obj["val"]})'
            elif obj.get('cmd') == 'set_all' and 'vals' in obj:
                # 全chサーボ
                vals = obj['vals']
                if len(vals) != 8:
                    return
                payload = bytes([0x02])
                for v in vals:
                    payload += bytes([v & 0xFF, (v >> 8) & 0xFF])
                desc = f'SET_ALL({vals})'
            elif obj.get('cmd') == 'reset':
                payload = bytes([0x03])
                desc = 'RESET(BIN)'
            else:
                # その他はJSON文字列
                msg = json.dumps(obj, ensure_ascii=False)
                payload = msg.encode('utf-8')
                desc = msg
            LEN = len(payload)
            header = b'\xAA\x55' + bytes([VER]) + bytes([TYPE]) + struct.pack('<H', SEQ) + struct.pack('<H', LEN)
            pkt_wo_crc = header + payload
            crc = crc16_ccitt(pkt_wo_crc[2:])
            pkt = pkt_wo_crc + struct.pack('<H', crc) + b'\x7E'
            self.tx_history.append(desc)
            self.ser.write(pkt)
        else:
            msg = json.dumps(obj, ensure_ascii=False)
            self.tx_history.append(msg)
            self.ser.write((msg + '\n').encode('utf-8'))

def accel_to_rp(imu):
    # 加速度からroll/pitchを計算（ラジアン）
    ax, ay, az = imu.ax, imu.ay, imu.az
    roll = math.atan2(ay, az)
    pitch = math.atan2(-ax, (ay*math.sin(roll) + az*math.cos(roll)))
    yaw = 0.0  # Yawは加速度のみでは算出不可
    return roll, pitch, yaw

def rotation_matrix(roll, pitch, yaw):
    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)
    # Rz * Ry * Rx
    return [
        [cy*cp, cy*sp*sr - sy*cr, cy*sp*cr + sy*sr],
        [sy*cp, sy*sp*sr + cy*cr, sy*sp*cr - cy*sr],
        [-sp,   cp*sr,            cp*cr           ],
    ]

def transform(point, m):
    x, y, z = point
    return (
        m[0][0]*x + m[0][1]*y + m[0][2]*z,
        m[1][0]*x + m[1][1]*y + m[1][2]*z,
        m[2][0]*x + m[2][1]*y + m[2][2]*z,
    )

def project(point, width, height, scale=300, zoffset=3.5):
    x, y, z = point
    z += zoffset
    if z == 0:
        z = 1e-3
    px = width/2 + (x * scale) / z
    py = height/2 - (y * scale) / z
    return int(px), int(py)

def draw_cube(screen, imu, area):
    x0, y0, w, h = area
    cx = x0 + w//2
    cy = y0 + h//2
    size = CUBE_SIZE / 100.0  # normalized
    roll, pitch, yaw = accel_to_rp(imu)
    m = rotation_matrix(roll, pitch, yaw)
    verts = [
        (-size, -size, -size), ( size, -size, -size), ( size,  size, -size), (-size,  size, -size),
        (-size, -size,  size), ( size, -size,  size), ( size,  size,  size), (-size,  size,  size),
    ]
    verts_r = [transform(v, m) for v in verts]
    # edges
    edges = [(0,1),(1,2),(2,3),(3,0),(4,5),(5,6),(6,7),(7,4),(0,4),(1,5),(2,6),(3,7)]
    # axes
    axes = [
        ((0,0,0),(1.2*size,0,0),(255,0,0)),
        ((0,0,0),(0,1.2*size,0),(0,255,0)),
        ((0,0,0),(0,0,1.2*size),(0,160,255)),
    ]
    points = [project(v, w, h, scale=260, zoffset=3.0) for v in verts_r]
    # draw cube
    for a,b in edges:
        pygame.draw.line(screen, (200,200,200), (x0+points[a][0]-w//2+cx, y0+points[a][1]-h//2+cy), (x0+points[b][0]-w//2+cx, y0+points[b][1]-h//2+cy), 2)
    # axes
    for origin, end, col in axes:
        o = project(transform(origin, m), w, h, scale=260, zoffset=3.0)
        e = project(transform(end, m), w, h, scale=260, zoffset=3.0)
        pygame.draw.line(screen, col, (x0+o[0]-w//2+cx, y0+o[1]-h//2+cy), (x0+e[0]-w//2+cx, y0+e[1]-h//2+cy), 4)

# --- スクリプトランナー ---
class ScriptRunner:
    def __init__(self, client, servo_values):
        self.client = client
        self.servo_values = servo_values
        self.running = False
        self.thread = None

    def start(self, script_text):
        if self.running:
            return
        self.running = True
        self.thread = threading.Thread(target=self._run, args=(script_text,), daemon=True)
        self.thread.start()

    def _run(self, script_text):
        lines = script_text.split('\n')
        for line in lines:
            if not self.running:
                break
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            try:
                # 例: set 0 1500
                parts = line.split()
                if parts[0] == 'set' and len(parts) == 3:
                    idx = int(parts[1])
                    val = int(parts[2])
                    if 0 <= idx < SERVO_COUNT:
                        self.servo_values[idx] = val
                        self.client.send_json({"cmd":"set","id":idx,"val":val})
                elif parts[0] == 'sleep' and len(parts) == 2:
                    t = float(parts[1])
                    time.sleep(t)
                else:
                    # その他コマンドはそのまま送信
                    self.client.send_json({"cmd":line})
            except Exception as e:
                pass
            time.sleep(0.05)
        self.running = False

    def stop(self):
        self.running = False
        if self.thread:
            self.thread.join(timeout=0.5)

# --- メイン関数 ---
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', required=False, default=None)
    parser.add_argument('--baud', type=int, default=921600)
    parser.add_argument('--binary', action='store_true', help='Use binary protocol instead of text JSON')
    parser.add_argument('--width', type=int, default=1200)
    parser.add_argument('--height', type=int, default=700)
    parser.add_argument('--fullscreen', action='store_true')
    args = parser.parse_args()

    pygame.init()
    flags = pygame.FULLSCREEN if args.fullscreen else 0
    screen = pygame.display.set_mode((args.width, args.height), flags)
    pygame.display.set_caption('Bipedal Serial Client')
    clock = pygame.time.Clock()
    font = pygame.font.SysFont('consolas', 18)
    small = pygame.font.SysFont('consolas', 14)

    # シリアルポート自動検出
    port = args.port
    if port is None:
        ports = list(serial.tools.list_ports.comports())
        if ports:
            print('利用可能なシリアルポート:')
            for i, p in enumerate(ports):
                print(f'  [{i}] {p.device} - {p.description}')
            port = ports[0].device
            print(f'自動選択: {port}')
    if port is None:
        print('エラー: シリアルポートが見つかりません。')
        return

    tx_history = []
    client = SerialClient(port, args.baud, binary_mode=args.binary, tx_history=tx_history)
    try:
        print(f'シリアルポート {port} を開いています (baud={args.baud})...')
        client.open()
        print('接続成功!')
    except Exception as e:
        print(f'エラー: シリアルポート {port} を開けませんでした。詳細: {e}')
        return

    script_runner = ScriptRunner(client, [1500]*SERVO_COUNT)
    imu = ImuData()
    servo_values = [1500]*SERVO_COUNT
    last_seq = 0
    last_send_times = [0.0]*SERVO_COUNT
    status_msg = ""
    status_msg_time = 0.0
    rx_history = bytearray()
    script_text = ""
    input_active = False
    copy_status = ""
    copy_status_time = 0.0

    # レイアウト
    slider_rects = []
    slider_area_x = 20
    slider_area_y = 80
    slider_w = 50
    slider_h = int(args.height * 0.34)
    spacing = 20
    for i in range(SERVO_COUNT):
        slider_rects.append((slider_area_x + i*(slider_w+spacing), slider_area_y, slider_w, slider_h))

    cube_w = int(args.width * 0.35)
    cube_h = int(args.height * 0.6)
    cube_area = (slider_area_x + SERVO_COUNT*(slider_w+spacing) + 40, 60, cube_w, cube_h)
    script_area = (20, slider_area_y + slider_h + 30, int(args.width*0.42), int(args.height*0.36))

    protocol_mode = "BINARY" if args.binary else "TEXT"
    running = True

    def draw_sliders(screen, slider_rects, servo_values, font, client, last_send_times):
        mouse = pygame.mouse.get_pos()
        pressed = pygame.mouse.get_pressed()[0]
        updated = False
        small_font = pygame.font.SysFont('consolas', 12)
        for i, rect in enumerate(slider_rects):
            x,y,w,h = rect
            pygame.draw.rect(screen, (50,50,60), rect, border_radius=4)
            pygame.draw.rect(screen, (80,80,90), rect, 2, border_radius=4)
            lbl = font.render(f"CH{i}", True, (255,255,255))
            lbl_rect = lbl.get_rect(center=(x + w//2, y-18))
            screen.blit(lbl, lbl_rect)
            val = servo_values[i]
            t = (val - SLIDER_MIN) / (SLIDER_MAX - SLIDER_MIN)
            handle_y = y + h - int(t * h)
            bar_height = y + h - handle_y
            pygame.draw.rect(screen, (70,140,220), (x+8, handle_y, w-16, bar_height), border_radius=2)
            pygame.draw.rect(screen, (120,200,255), (x+4, handle_y-8, w-8, 16), border_radius=6)
            pygame.draw.circle(screen, (160,220,255), (x+w//2, handle_y), 6)
            vtxt = small_font.render(str(val), True, (220,255,220))
            vtxt_rect = vtxt.get_rect(center=(x + w//2, y+h+12))
            screen.blit(vtxt, vtxt_rect)
            for mark_val in [500, 1000, 1500, 2000, 2500]:
                mark_t = (mark_val - SLIDER_MIN) / (SLIDER_MAX - SLIDER_MIN)
                mark_y = y + h - int(mark_t * h)
                pygame.draw.line(screen, (100,100,110), (x+w-4, mark_y), (x+w, mark_y), 2)
                if mark_val in [500, 1500, 2500]:
                    mark_txt = small_font.render(str(mark_val), True, (140,140,150))
                    screen.blit(mark_txt, (x+w+4, mark_y-6))
            if pressed and x <= mouse[0] <= x+w and y <= mouse[1] <= y+h:
                t_new = (y + h - mouse[1]) / h
                t_new = max(0.0, min(1.0, t_new))
                new_val = int(SLIDER_MIN + t_new * (SLIDER_MAX - SLIDER_MIN))
                if new_val != servo_values[i]:
                    servo_values[i] = new_val
                    updated = True
                    now = time.time()
                    if now - last_send_times[i] > SLIDER_SEND_INTERVAL:
                        last_send_times[i] = now
                        client.send_json({"cmd":"set","id":i,"val":new_val})
        return updated

    def wrap_text(text, font, max_width):
        words = text.split(' ')
        lines = []
        cur = ''
        for w in words:
            test = cur + (' ' if cur else '') + w
            if font.size(test)[0] > max_width:
                if cur:
                    lines.append(cur)
                cur = w
            else:
                cur = test
        if cur:
            lines.append(cur)
        return lines

    ping_button_pressed = False
    while running:
        dt = clock.tick(60) / 1000.0
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if input_active:
                    if event.key == pygame.K_ESCAPE:
                        input_active = False
                    elif event.key == pygame.K_BACKSPACE:
                        script_text = script_text[:-1]
                    elif event.key == pygame.K_RETURN:
                        script_text += '\n'
                    else:
                        script_text += event.unicode
                else:
                    if event.key == pygame.K_ESCAPE:
                        running = False
            elif event.type == pygame.MOUSEBUTTONDOWN:
                mx,my = event.pos
                if script_area[0] <= mx <= script_area[0]+script_area[2] and script_area[1] <= my <= script_area[1]+script_area[3]:
                    input_active = True
                else:
                    input_active = False
                if 560 <= mx <= 640 and 360 <= my <= 390:
                    if not ping_button_pressed:
                        client.send_json({"cmd":"ping"})
                        status_msg = "Ping送信中..."
                        status_msg_time = time.time()
                        ping_button_pressed = True
                if 560 <= mx <= 660 and 400 <= my <= 430:
                    if not script_runner.running:
                        script_runner.start(script_text)
                if 560 <= mx <= 660 and 440 <= my <= 470:
                    script_runner.stop()
                if 700 <= mx <= 860 and args.height-120 <= my <= args.height-90:
                    pyperclip.copy('\n'.join(tx_history))
                    copy_status = "TX履歴をクリップボードにコピーしました"
                    copy_status_time = time.time()
            elif event.type == pygame.MOUSEBUTTONUP:
                mx, my = event.pos
                if 560 <= mx <= 640 and 360 <= my <= 390:
                    ping_button_pressed = False
                if script_area[0] <= mx <= script_area[0]+script_area[2] and script_area[1] <= my <= script_area[1]+script_area[3]:
                    input_active = True
                else:
                    input_active = False

        # --- 受信処理 ---
        while not client.recv_queue.empty():
            line = client.recv_queue.get()
            try:
                obj = json.loads(line)
                last_seq = obj.get('seq', last_seq)
                if 'resp' in obj:
                    resp_type = obj['resp']
                    if resp_type == 'pong':
                        millis = obj.get('millis', 0)
                        status_msg = f'Ping OK! Device millis: {millis}'
                    else:
                        status_msg = f'Response: {resp_type}'
                    status_msg_time = time.time()
                if 'imu' in obj:
                    imu_obj = obj['imu']
                    imu.ax = float(imu_obj.get('ax', imu.ax))
                    imu.ay = float(imu_obj.get('ay', imu.ay))
                    imu.az = float(imu_obj.get('az', imu.az))
                    imu.gx = float(imu_obj.get('gx', imu.gx))
                    imu.gy = float(imu_obj.get('gy', imu.gy))
                    imu.gz = float(imu_obj.get('gz', imu.gz))
                    imu.temp = float(imu_obj.get('temp', imu.temp))
                imu.seq = last_seq
                if 'pos' in obj and isinstance(obj['pos'], list):
                    for i,v in enumerate(obj['pos'][:SERVO_COUNT]):
                        servo_values[i] = int(v)
            except json.JSONDecodeError:
                status_msg = 'JSON parse error'
                status_msg_time = time.time()

        if status_msg and time.time() - status_msg_time > 3.0:
            status_msg = ""

        # --- UI描画 ---
        screen.fill((20, 22, 28))
        draw_sliders(screen, slider_rects, servo_values, font, client, last_send_times)
        screen.blit(font.render(f'Serial: {client.port} @ {client.baud} ({protocol_mode})', True, (200,200,200)), (20, 20))
        screen.blit(font.render(f'Seq: {last_seq}', True, (120,200,120)), (20, 50))
        pygame.draw.rect(screen, (35,40,45), cube_area, border_radius=8)
        draw_cube(screen, imu, cube_area)
        screen.blit(font.render('IMU Cube (roll/pitch from accel)', True, (200,200,200)), (cube_area[0], cube_area[1]-24))
        pygame.draw.rect(screen, (35,40,45), script_area, border_radius=8)
        script_lines = script_text.split('\n')
        for idx, line in enumerate(script_lines[-14:]):
            screen.blit(small.render(line, True, (230,230,230)), (script_area[0]+8, script_area[1]+8+idx*18))
        pygame.draw.rect(screen, (80,120,200) if input_active else (90,90,100), script_area, 2, border_radius=8)
        screen.blit(font.render('Script (one cmd per line)', True, (200,200,200)), (script_area[0], script_area[1]-24))
        def draw_btn(x,y,w,h,text,color):
            pygame.draw.rect(screen, color, (x,y,w,h), border_radius=6)
            screen.blit(small.render(text, True, (20,20,20)), (x+8,y+6))
        draw_btn(560, 360, 80, 30, 'Ping', (120,200,255))
        draw_btn(560, 400, 100, 30, 'Run Script', (120,255,120))
        draw_btn(560, 440, 100, 30, 'Stop Script', (255,120,120))
        bottom_margin = 60
        rx_h = 36
        msg_h = font.get_linesize() + 8
        tx_h = 90
        tx_x = 20
        tx_w = int(args.width * 0.55)
        tx_y = args.height - (rx_h + msg_h + tx_h + bottom_margin)
        screen.blit(small.render('TX History (Sent Commands):', True, (255,200,120)), (tx_x, tx_y))
        tx_line_h = small.get_linesize()
        tx_lines_max = (tx_h // tx_line_h) - 1
        tx_lines = []
        for tx in tx_history[-10:]:
            tx_lines.extend(wrap_text(tx, small, tx_w-40))
        tx_lines = tx_lines[-tx_lines_max:]
        for i, line in enumerate(tx_lines):
            screen.blit(small.render(line, True, (255,220,180)), (tx_x+20, tx_y + 18 + i*tx_line_h))
        btn_x = tx_x + tx_w + 20
        btn_y = tx_y
        draw_btn(btn_x, btn_y, 160, 30, 'TX履歴をコピー', (255,255,180))
        if copy_status and time.time() - copy_status_time < 2.0:
            screen.blit(small.render(copy_status, True, (255,180,120)), (btn_x, btn_y+36))
        rx_y = args.height - (rx_h + msg_h + bottom_margin)
        rx_x = 20
        rx_w = int(args.width * 0.85)
        rx_line_h = small.get_linesize()
        rx_lines_max = rx_h // rx_line_h
        if args.binary:
            hexstr = ' '.join(f'{b:02X}' for b in client.rx_raw_history)
            rx_lines = wrap_text(f'RX Raw (Hex): {hexstr}', small, rx_w)
        else:
            try:
                txt = client.rx_history.decode('utf-8')
            except UnicodeDecodeError:
                try:
                    txt = client.rx_history.decode('cp932')
                except Exception:
                    txt = ' '.join(f'{b:02X}' for b in client.rx_history)
            rx_lines = wrap_text(f'RX History (Text): {txt}', small, rx_w)
        rx_lines = rx_lines[-rx_lines_max:]
        for i, line in enumerate(rx_lines):
            screen.blit(small.render(line, True, (120,255,120)), (rx_x, rx_y + i*rx_line_h))
        msg_y = args.height - (msg_h + bottom_margin//2)
        if status_msg:
            status_lines = wrap_text(status_msg, font, int(args.width*0.8))
            for i, line in enumerate(status_lines):
                screen.blit(font.render(line, True, (255,120,120)), (20, msg_y + i*font.get_linesize()))
        pygame.display.flip()

    script_runner.stop()
    client.close()
    pygame.quit()


if __name__ == '__main__':
    main()

# -*- coding: utf-8 -*-
# main.py: バイナリ/テキスト両対応・IMU可視化・スクリプト・履歴・エラー処理付きGUIクライアント

import sys
import os
import time
import math
import struct
import threading
import queue
import json
import argparse
import serial
import serial.tools.list_ports
import pygame
import pyperclip

# --- 定数 ---
CUBE_SIZE = 60
SLIDER_MIN = 500
SLIDER_MAX = 2500
SLIDER_SEND_INTERVAL = 0.04
SERVO_COUNT = 8
RX_HISTORY_MAX = 4096

# --- CRC16-CCITT ---
def crc16_ccitt(data: bytes, poly=0x1021, init=0xFFFF) -> int:
    crc = init
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ poly
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc

# --- IMUデータクラス ---
class ImuData:
    def __init__(self):
        self.ax = 0.0
        self.ay = 0.0
        self.az = 0.0
        self.gx = 0.0
        self.gy = 0.0
        self.gz = 0.0
        self.temp = 0.0
        self.seq = 0

# --- シリアルクライアント ---
class SerialClient(threading.Thread):
    def __init__(self, port, baud, binary_mode=False, tx_history=None):
        super().__init__(daemon=True)
        self.port = port
        self.baud = baud
        self.binary_mode = binary_mode
        self.tx_history = tx_history if tx_history is not None else []
        self.rx_history = bytearray()
        self.rx_raw_history = bytearray()
        self.recv_queue = queue.Queue()
        self._stop_event = threading.Event()
        self.ser = None

    def open(self):
        self.ser = serial.Serial(self.port, self.baud, timeout=0.05)
        self.start()

    def close(self):
        self._stop_event.set()
        if self.ser:
            self.ser.close()

    def run(self):
        while not self._stop_event.is_set():
            try:
                if self.ser.in_waiting:
                    data = self.ser.read(self.ser.in_waiting)
                    self.rx_raw_history += data
                    if self.binary_mode:
                        # バイナリ: パケット分解
                        self._parse_binary(data)
                    else:
                        # テキスト: 行単位
                        lines = data.decode(errors='ignore').split('\n')
                        for line in lines:
                            if line.strip():
                                self.recv_queue.put(line.strip())
                    # RX履歴管理
                    self.rx_history += data
                    if len(self.rx_history) > RX_HISTORY_MAX:
                        self.rx_history = self.rx_history[-RX_HISTORY_MAX:]
            except Exception as e:
                pass
            time.sleep(0.01)

    def _parse_binary(self, data):
        # AA55ヘッダ・長さ・CRC付きバイナリパケット分解
        buf = getattr(self, '_binbuf', b'') + data
        packets = []
        while len(buf) >= 6:
            if buf[0:2] != b'\xAA\x55':
                buf = buf[1:]
                continue
            if len(buf) < 6:
                break
            length = buf[2]
            if len(buf) < 3 + length + 2:
                break
            pkt = buf[:3+length+2]
            crc = struct.unpack('<H', pkt[-2:])[0]
            if crc16_ccitt(pkt[:-2]) == crc:
                packets.append(pkt)
                # 例: JSON部抽出
                try:
                    payload = pkt[3:-2]
                    text = payload.decode(errors='ignore')
                    self.recv_queue.put(text)
                except Exception:
                    pass
                buf = buf[3+length+2:]
            else:
                buf = buf[1:]
        self._binbuf = buf

    def send_json(self, obj):
        msg = json.dumps(obj, ensure_ascii=False)
        self.tx_history.append(msg)
        if self.binary_mode:
            # バイナリパケット化
            payload = msg.encode('utf-8')
            pkt = b'\xAA\x55' + bytes([len(payload)]) + payload
            crc = crc16_ccitt(pkt)
            pkt += struct.pack('<H', crc)
            self.ser.write(pkt)
        else:
            self.ser.write((msg + '\n').encode('utf-8'))

def accel_to_rp(imu):
    # 加速度からroll/pitchを計算（ラジアン）
    ax, ay, az = imu.ax, imu.ay, imu.az
    roll = math.atan2(ay, az)
    pitch = math.atan2(-ax, (ay*math.sin(roll) + az*math.cos(roll)))
    yaw = 0.0  # Yawは加速度のみでは算出不可
    return roll, pitch, yaw

def rotation_matrix(roll, pitch, yaw):
    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)
    # Rz * Ry * Rx
    return [
        [cy*cp, cy*sp*sr - sy*cr, cy*sp*cr + sy*sr],
        [sy*cp, sy*sp*sr + cy*cr, sy*sp*cr - cy*sr],
        [-sp,   cp*sr,            cp*cr           ],
    ]

def transform(point, m):
    x, y, z = point
    return (
        m[0][0]*x + m[0][1]*y + m[0][2]*z,
        m[1][0]*x + m[1][1]*y + m[1][2]*z,
        m[2][0]*x + m[2][1]*y + m[2][2]*z,
    )

def project(point, width, height, scale=300, zoffset=3.5):
    x, y, z = point
    z += zoffset
    if z == 0:
        z = 1e-3
    px = width/2 + (x * scale) / z
    py = height/2 - (y * scale) / z
    return int(px), int(py)

def draw_cube(screen, imu, area):
    x0, y0, w, h = area
    cx = x0 + w//2
    cy = y0 + h//2
    size = CUBE_SIZE / 100.0  # normalized
    roll, pitch, yaw = accel_to_rp(imu)
    m = rotation_matrix(roll, pitch, yaw)
    verts = [
        (-size, -size, -size), ( size, -size, -size), ( size,  size, -size), (-size,  size, -size),
        (-size, -size,  size), ( size, -size,  size), ( size,  size,  size), (-size,  size,  size),
    ]
    verts_r = [transform(v, m) for v in verts]
    # edges
    edges = [(0,1),(1,2),(2,3),(3,0),(4,5),(5,6),(6,7),(7,4),(0,4),(1,5),(2,6),(3,7)]
    # axes
    axes = [
        ((0,0,0),(1.2*size,0,0),(255,0,0)),
        ((0,0,0),(0,1.2*size,0),(0,255,0)),
        ((0,0,0),(0,0,1.2*size),(0,160,255)),
    ]
    points = [project(v, w, h, scale=260, zoffset=3.0) for v in verts_r]
    # draw cube
    for a,b in edges:
        pygame.draw.line(screen, (200,200,200), (x0+points[a][0]-w//2+cx, y0+points[a][1]-h//2+cy), (x0+points[b][0]-w//2+cx, y0+points[b][1]-h//2+cy), 2)
    # axes
    for origin, end, col in axes:
        o = project(transform(origin, m), w, h, scale=260, zoffset=3.0)
        e = project(transform(end, m), w, h, scale=260, zoffset=3.0)
        pygame.draw.line(screen, col, (x0+o[0]-w//2+cx, y0+o[1]-h//2+cy), (x0+e[0]-w//2+cx, y0+e[1]-h//2+cy), 4)

# --- スクリプトランナー ---
class ScriptRunner:
    def __init__(self, client, servo_values):
        self.client = client
        self.servo_values = servo_values
        self.running = False
        self.thread = None

    def start(self, script_text):
        if self.running:
            return
        self.running = True
        self.thread = threading.Thread(target=self._run, args=(script_text,), daemon=True)
        self.thread.start()

    def _run(self, script_text):
        lines = script_text.split('\n')
        for line in lines:
            if not self.running:
                break
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            try:
                # 例: set 0 1500
                parts = line.split()
                if parts[0] == 'set' and len(parts) == 3:
                    idx = int(parts[1])
                    val = int(parts[2])
                    if 0 <= idx < SERVO_COUNT:
                        self.servo_values[idx] = val
                        self.client.send_json({"cmd":"set","id":idx,"val":val})
                elif parts[0] == 'sleep' and len(parts) == 2:
                    t = float(parts[1])
                    time.sleep(t)
                else:
                    # その他コマンドはそのまま送信
                    self.client.send_json({"cmd":line})
            except Exception as e:
                pass
            time.sleep(0.05)
        self.running = False

    def stop(self):
        self.running = False
        if self.thread:
            self.thread.join(timeout=0.5)

# --- メイン関数 ---
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', required=False, default=None)
    parser.add_argument('--baud', type=int, default=921600)
    parser.add_argument('--binary', action='store_true', help='Use binary protocol instead of text JSON')
    parser.add_argument('--width', type=int, default=1200)
    parser.add_argument('--height', type=int, default=700)
    parser.add_argument('--fullscreen', action='store_true')
    args = parser.parse_args()

    pygame.init()
    flags = pygame.FULLSCREEN if args.fullscreen else 0
    screen = pygame.display.set_mode((args.width, args.height), flags)
    pygame.display.set_caption('Bipedal Serial Client')
    clock = pygame.time.Clock()
    font = pygame.font.SysFont('consolas', 18)
    small = pygame.font.SysFont('consolas', 14)

    # シリアルポート自動検出
    port = args.port
    if port is None:
        ports = list(serial.tools.list_ports.comports())
        if ports:
            print('利用可能なシリアルポート:')
            for i, p in enumerate(ports):
                print(f'  [{i}] {p.device} - {p.description}')
            port = ports[0].device
            print(f'自動選択: {port}')
    if port is None:
        print('エラー: シリアルポートが見つかりません。')
        return

    tx_history = []
    client = SerialClient(port, args.baud, binary_mode=args.binary, tx_history=tx_history)
    try:
        print(f'シリアルポート {port} を開いています (baud={args.baud})...')
        client.open()
        print('接続成功!')
    except Exception as e:
        print(f'エラー: シリアルポート {port} を開けませんでした。詳細: {e}')
        return

    script_runner = ScriptRunner(client, [1500]*SERVO_COUNT)
    imu = ImuData()
    servo_values = [1500]*SERVO_COUNT
    last_seq = 0
    last_send_times = [0.0]*SERVO_COUNT
    status_msg = ""
    status_msg_time = 0.0
    rx_history = bytearray()
    script_text = ""
    input_active = False
    copy_status = ""
    copy_status_time = 0.0

    # レイアウト
    slider_rects = []
    slider_area_x = 20
    slider_area_y = 80
    slider_w = 50
    slider_h = int(args.height * 0.34)
    spacing = 20
    for i in range(SERVO_COUNT):
        slider_rects.append((slider_area_x + i*(slider_w+spacing), slider_area_y, slider_w, slider_h))

    cube_w = int(args.width * 0.35)
    cube_h = int(args.height * 0.6)
    cube_area = (slider_area_x + SERVO_COUNT*(slider_w+spacing) + 40, 60, cube_w, cube_h)
    script_area = (20, slider_area_y + slider_h + 30, int(args.width*0.42), int(args.height*0.36))

    protocol_mode = "BINARY" if args.binary else "TEXT"
    running = True

    def draw_sliders(screen, slider_rects, servo_values, font, client, last_send_times):
        mouse = pygame.mouse.get_pos()
        pressed = pygame.mouse.get_pressed()[0]
        updated = False
        small_font = pygame.font.SysFont('consolas', 12)
        for i, rect in enumerate(slider_rects):
            x,y,w,h = rect
            pygame.draw.rect(screen, (50,50,60), rect, border_radius=4)
            pygame.draw.rect(screen, (80,80,90), rect, 2, border_radius=4)
            lbl = font.render(f"CH{i}", True, (255,255,255))
            lbl_rect = lbl.get_rect(center=(x + w//2, y-18))
            screen.blit(lbl, lbl_rect)
            val = servo_values[i]
            t = (val - SLIDER_MIN) / (SLIDER_MAX - SLIDER_MIN)
            handle_y = y + h - int(t * h)
            bar_height = y + h - handle_y
            pygame.draw.rect(screen, (70,140,220), (x+8, handle_y, w-16, bar_height), border_radius=2)
            pygame.draw.rect(screen, (120,200,255), (x+4, handle_y-8, w-8, 16), border_radius=6)
            pygame.draw.circle(screen, (160,220,255), (x+w//2, handle_y), 6)
            vtxt = small_font.render(str(val), True, (220,255,220))
            vtxt_rect = vtxt.get_rect(center=(x + w//2, y+h+12))
            screen.blit(vtxt, vtxt_rect)
            for mark_val in [500, 1000, 1500, 2000, 2500]:
                mark_t = (mark_val - SLIDER_MIN) / (SLIDER_MAX - SLIDER_MIN)
                mark_y = y + h - int(mark_t * h)
                pygame.draw.line(screen, (100,100,110), (x+w-4, mark_y), (x+w, mark_y), 2)
                if mark_val in [500, 1500, 2500]:
                    mark_txt = small_font.render(str(mark_val), True, (140,140,150))
                    screen.blit(mark_txt, (x+w+4, mark_y-6))
            if pressed and x <= mouse[0] <= x+w and y <= mouse[1] <= y+h:
                t_new = (y + h - mouse[1]) / h
                t_new = max(0.0, min(1.0, t_new))
                new_val = int(SLIDER_MIN + t_new * (SLIDER_MAX - SLIDER_MIN))
                if new_val != servo_values[i]:
                    servo_values[i] = new_val
                    updated = True
                    now = time.time()
                    if now - last_send_times[i] > SLIDER_SEND_INTERVAL:
                        last_send_times[i] = now
                        client.send_json({"cmd":"set","id":i,"val":new_val})
        return updated

    def wrap_text(text, font, max_width):
        words = text.split(' ')
        lines = []
        cur = ''
        for w in words:
            test = cur + (' ' if cur else '') + w
            if font.size(test)[0] > max_width:
                if cur:
                    lines.append(cur)
                cur = w
            else:
                cur = test
        if cur:
            lines.append(cur)
        return lines

    ping_button_pressed = False
    while running:
        dt = clock.tick(60) / 1000.0
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if input_active:
                    if event.key == pygame.K_ESCAPE:
                        input_active = False
                    elif event.key == pygame.K_BACKSPACE:
                        script_text = script_text[:-1]
                    elif event.key == pygame.K_RETURN:
                        script_text += '\n'
                    else:
                        script_text += event.unicode
                else:
                    if event.key == pygame.K_ESCAPE:
                        running = False
            elif event.type == pygame.MOUSEBUTTONDOWN:
                mx,my = event.pos
                if script_area[0] <= mx <= script_area[0]+script_area[2] and script_area[1] <= my <= script_area[1]+script_area[3]:
                    input_active = True
                else:
                    input_active = False
                if 560 <= mx <= 640 and 360 <= my <= 390:
                    if not ping_button_pressed:
                        client.send_json({"cmd":"ping"})
                        status_msg = "Ping送信中..."
                        status_msg_time = time.time()
                        ping_button_pressed = True
                if 560 <= mx <= 660 and 400 <= my <= 430:
                    if not script_runner.running:
                        script_runner.start(script_text)
                if 560 <= mx <= 660 and 440 <= my <= 470:
                    script_runner.stop()
                if 700 <= mx <= 860 and args.height-120 <= my <= args.height-90:
                    pyperclip.copy('\n'.join(tx_history))
                    copy_status = "TX履歴をクリップボードにコピーしました"
                    copy_status_time = time.time()
            elif event.type == pygame.MOUSEBUTTONUP:
                mx, my = event.pos
                if 560 <= mx <= 640 and 360 <= my <= 390:
                    ping_button_pressed = False
                if script_area[0] <= mx <= script_area[0]+script_area[2] and script_area[1] <= my <= script_area[1]+script_area[3]:
                    input_active = True
                else:
                    input_active = False

        # --- 受信処理 ---
        while not client.recv_queue.empty():
            line = client.recv_queue.get()
            try:
                obj = json.loads(line)
                last_seq = obj.get('seq', last_seq)
                if 'resp' in obj:
                    resp_type = obj['resp']
                    if resp_type == 'pong':
                        millis = obj.get('millis', 0)
                        status_msg = f'Ping OK! Device millis: {millis}'
                    else:
                        status_msg = f'Response: {resp_type}'
                    status_msg_time = time.time()
                if 'imu' in obj:
                    imu_obj = obj['imu']
                    imu.ax = float(imu_obj.get('ax', imu.ax))
                    imu.ay = float(imu_obj.get('ay', imu.ay))
                    imu.az = float(imu_obj.get('az', imu.az))
                    imu.gx = float(imu_obj.get('gx', imu.gx))
                    imu.gy = float(imu_obj.get('gy', imu.gy))
                    imu.gz = float(imu_obj.get('gz', imu.gz))
                    imu.temp = float(imu_obj.get('temp', imu.temp))
                imu.seq = last_seq
                if 'pos' in obj and isinstance(obj['pos'], list):
                    for i,v in enumerate(obj['pos'][:SERVO_COUNT]):
                        servo_values[i] = int(v)
            except json.JSONDecodeError:
                status_msg = 'JSON parse error'
                status_msg_time = time.time()

        if status_msg and time.time() - status_msg_time > 3.0:
            status_msg = ""

        # --- UI描画 ---
        screen.fill((20, 22, 28))
        draw_sliders(screen, slider_rects, servo_values, font, client, last_send_times)
        screen.blit(font.render(f'Serial: {client.port} @ {client.baud} ({protocol_mode})', True, (200,200,200)), (20, 20))
        screen.blit(font.render(f'Seq: {last_seq}', True, (120,200,120)), (20, 50))
        pygame.draw.rect(screen, (35,40,45), cube_area, border_radius=8)
        draw_cube(screen, imu, cube_area)
        screen.blit(font.render('IMU Cube (roll/pitch from accel)', True, (200,200,200)), (cube_area[0], cube_area[1]-24))
        pygame.draw.rect(screen, (35,40,45), script_area, border_radius=8)
        script_lines = script_text.split('\n')
        for idx, line in enumerate(script_lines[-14:]):
            screen.blit(small.render(line, True, (230,230,230)), (script_area[0]+8, script_area[1]+8+idx*18))
        pygame.draw.rect(screen, (80,120,200) if input_active else (90,90,100), script_area, 2, border_radius=8)
        screen.blit(font.render('Script (one cmd per line)', True, (200,200,200)), (script_area[0], script_area[1]-24))
        def draw_btn(x,y,w,h,text,color):
            pygame.draw.rect(screen, color, (x,y,w,h), border_radius=6)
            screen.blit(small.render(text, True, (20,20,20)), (x+8,y+6))
        draw_btn(560, 360, 80, 30, 'Ping', (120,200,255))
        draw_btn(560, 400, 100, 30, 'Run Script', (120,255,120))
        draw_btn(560, 440, 100, 30, 'Stop Script', (255,120,120))
        bottom_margin = 60
        rx_h = 36
        msg_h = font.get_linesize() + 8
        tx_h = 90
        tx_x = 20
        tx_w = int(args.width * 0.55)
        tx_y = args.height - (rx_h + msg_h + tx_h + bottom_margin)
        screen.blit(small.render('TX History (Sent Commands):', True, (255,200,120)), (tx_x, tx_y))
        tx_line_h = small.get_linesize()
        tx_lines_max = (tx_h // tx_line_h) - 1
        tx_lines = []
        for tx in tx_history[-10:]:
            tx_lines.extend(wrap_text(tx, small, tx_w-40))
        tx_lines = tx_lines[-tx_lines_max:]
        for i, line in enumerate(tx_lines):
            screen.blit(small.render(line, True, (255,220,180)), (tx_x+20, tx_y + 18 + i*tx_line_h))
        btn_x = tx_x + tx_w + 20
        btn_y = tx_y
        draw_btn(btn_x, btn_y, 160, 30, 'TX履歴をコピー', (255,255,180))
        if copy_status and time.time() - copy_status_time < 2.0:
            screen.blit(small.render(copy_status, True, (255,180,120)), (btn_x, btn_y+36))
        rx_y = args.height - (rx_h + msg_h + bottom_margin)
        rx_x = 20
        rx_w = int(args.width * 0.85)
        rx_line_h = small.get_linesize()
        rx_lines_max = rx_h // rx_line_h
        if args.binary:
            hexstr = ' '.join(f'{b:02X}' for b in client.rx_raw_history)
            rx_lines = wrap_text(f'RX Raw (Hex): {hexstr}', small, rx_w)
        else:
            try:
                txt = client.rx_history.decode('utf-8')
            except UnicodeDecodeError:
                try:
                    txt = client.rx_history.decode('cp932')
                except Exception:
                    txt = ' '.join(f'{b:02X}' for b in client.rx_history)
            rx_lines = wrap_text(f'RX History (Text): {txt}', small, rx_w)
        rx_lines = rx_lines[-rx_lines_max:]
        for i, line in enumerate(rx_lines):
            screen.blit(small.render(line, True, (120,255,120)), (rx_x, rx_y + i*rx_line_h))
        msg_y = args.height - (msg_h + bottom_margin//2)
        if status_msg:
            status_lines = wrap_text(status_msg, font, int(args.width*0.8))
            for i, line in enumerate(status_lines):
                screen.blit(font.render(line, True, (255,120,120)), (20, msg_y + i*font.get_linesize()))
        pygame.display.flip()

    script_runner.stop()
    client.close()
    pygame.quit()


if __name__ == '__main__':
    main()

# -*- coding: utf-8 -*-
# main.py: バイナリ/テキスト両対応・IMU可視化・スクリプト・履歴・エラー処理付きGUIクライアント

import sys
import os
import time
import math
import struct
import threading
import queue
import json
import argparse
import serial
import serial.tools.list_ports
import pygame
import pyperclip

# --- 定数 ---
CUBE_SIZE = 60
SLIDER_MIN = 500
SLIDER_MAX = 2500
SLIDER_SEND_INTERVAL = 0.04
SERVO_COUNT = 8
RX_HISTORY_MAX = 4096

# --- CRC16-CCITT ---
def crc16_ccitt(data: bytes, poly=0x1021, init=0xFFFF) -> int:
    crc = init
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ poly
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc

# --- IMUデータクラス ---
class ImuData:
    def __init__(self):
        self.ax = 0.0
        self.ay = 0.0
        self.az = 0.0
        self.gx = 0.0
        self.gy = 0.0
        self.gz = 0.0
        self.temp = 0.0
        self.seq = 0

# --- シリアルクライアント ---
class SerialClient(threading.Thread):
    def __init__(self, port, baud, binary_mode=False, tx_history=None):
        super().__init__(daemon=True)
        self.port = port
        self.baud = baud
        self.binary_mode = binary_mode
        self.tx_history = tx_history if tx_history is not None else []
        self.rx_history = bytearray()
        self.rx_raw_history = bytearray()
        self.recv_queue = queue.Queue()
        self._stop_event = threading.Event()
        self.ser = None

    def open(self):
        self.ser = serial.Serial(self.port, self.baud, timeout=0.05)
        self.start()

    def close(self):
        self._stop_event.set()
        if self.ser:
            self.ser.close()

    def run(self):
        while not self._stop_event.is_set():
            try:
                if self.ser.in_waiting:
                    data = self.ser.read(self.ser.in_waiting)
                    self.rx_raw_history += data
                    if self.binary_mode:
                        # バイナリ: パケット分解
                        self._parse_binary(data)
                    else:
                        # テキスト: 行単位
                        lines = data.decode(errors='ignore').split('\n')
                        for line in lines:
                            if line.strip():
                                self.recv_queue.put(line.strip())
                    # RX履歴管理
                    self.rx_history += data
                    if len(self.rx_history) > RX_HISTORY_MAX:
                        self.rx_history = self.rx_history[-RX_HISTORY_MAX:]
            except Exception as e:
                pass
            time.sleep(0.01)

    def _parse_binary(self, data):
        # AA55ヘッダ・長さ・CRC付きバイナリパケット分解
        buf = getattr(self, '_binbuf', b'') + data
        packets = []
        while len(buf) >= 6:
            if buf[0:2] != b'\xAA\x55':
                buf = buf[1:]
                continue
            if len(buf) < 6:
                break
            length = buf[2]
            if len(buf) < 3 + length + 2:
                break
            pkt = buf[:3+length+2]
            crc = struct.unpack('<H', pkt[-2:])[0]
            if crc16_ccitt(pkt[:-2]) == crc:
                packets.append(pkt)
                # 例: JSON部抽出
                try:
                    payload = pkt[3:-2]
                    text = payload.decode(errors='ignore')
                    self.recv_queue.put(text)
                except Exception:
                    pass
                buf = buf[3+length+2:]
            else:
                buf = buf[1:]
        self._binbuf = buf

    def send_json(self, obj):
        msg = json.dumps(obj, ensure_ascii=False)
        self.tx_history.append(msg)
        if self.binary_mode:
            # バイナリパケット化
            payload = msg.encode('utf-8')
            pkt = b'\xAA\x55' + bytes([len(payload)]) + payload
            crc = crc16_ccitt(pkt)
            pkt += struct.pack('<H', crc)
            self.ser.write(pkt)
        else:
            self.ser.write((msg + '\n').encode('utf-8'))

def accel_to_rp(imu):
    # 加速度からroll/pitchを計算（ラジアン）
    ax, ay, az = imu.ax, imu.ay, imu.az
    roll = math.atan2(ay, az)
    pitch = math.atan2(-ax, (ay*math.sin(roll) + az*math.cos(roll)))
    yaw = 0.0  # Yawは加速度のみでは算出不可
    return roll, pitch, yaw

def rotation_matrix(roll, pitch, yaw):
    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)
    # Rz * Ry * Rx
    return [
        [cy*cp, cy*sp*sr - sy*cr, cy*sp*cr + sy*sr],
        [sy*cp, sy*sp*sr + cy*cr, sy*sp*cr - cy*sr],
        [-sp,   cp*sr,            cp*cr           ],
    ]

def transform(point, m):
    x, y, z = point
    return (
        m[0][0]*x + m[0][1]*y + m[0][2]*z,
        m[1][0]*x + m[1][1]*y + m[1][2]*z,
        m[2][0]*x + m[2][1]*y + m[2][2]*z,
    )

def project(point, width, height, scale=300, zoffset=3.5):
    x, y, z = point
    z += zoffset
    if z == 0:
        z = 1e-3
    px = width/2 + (x * scale) / z
    py = height/2 - (y * scale) / z
    return int(px), int(py)

def draw_cube(screen, imu, area):
    x0, y0, w, h = area
    cx = x0 + w//2
    cy = y0 + h//2
    size = CUBE_SIZE / 100.0  # normalized
    roll, pitch, yaw = accel_to_rp(imu)
    m = rotation_matrix(roll, pitch, yaw)
    verts = [
        (-size, -size, -size), ( size, -size, -size), ( size,  size, -size), (-size,  size, -size),
        (-size, -size,  size), ( size, -size,  size), ( size,  size,  size), (-size,  size,  size),
    ]
    verts_r = [transform(v, m) for v in verts]
    # edges
    edges = [(0,1),(1,2),(2,3),(3,0),(4,5),(5,6),(6,7),(7,4),(0,4),(1,5),(2,6),(3,7)]
    # axes
    axes = [
        ((0,0,0),(1.2*size,0,0),(255,0,0)),
        ((0,0,0),(0,1.2*size,0),(0,255,0)),
        ((0,0,0),(0,0,1.2*size),(0,160,255)),
    ]
    points = [project(v, w, h, scale=260, zoffset=3.0) for v in verts_r]
    # draw cube
    for a,b in edges:
        pygame.draw.line(screen, (200,200,200), (x0+points[a][0]-w//2+cx, y0+points[a][1]-h//2+cy), (x0+points[b][0]-w//2+cx, y0+points[b][1]-h//2+cy), 2)
    # axes
    for origin, end, col in axes:
        o = project(transform(origin, m), w, h, scale=260, zoffset=3.0)
        e = project(transform(end, m), w, h, scale=260, zoffset=3.0)
        pygame.draw.line(screen, col, (x0+o[0]-w//2+cx, y0+o[1]-h//2+cy), (x0+e[0]-w//2+cx, y0+e[1]-h//2+cy), 4)

# --- スクリプトランナー ---
class ScriptRunner:
    def __init__(self, client, servo_values):
        self.client = client
        self.servo_values = servo_values
        self.running = False
        self.thread = None

    def start(self, script_text):
        if self.running:
            return
        self.running = True
        self.thread = threading.Thread(target=self._run, args=(script_text,), daemon=True)
        self.thread.start()

    def _run(self, script_text):
        lines = script_text.split('\n')
        for line in lines:
            if not self.running:
                break
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            try:
                # 例: set 0 1500
                parts = line.split()
                if parts[0] == 'set' and len(parts) == 3:
                    idx = int(parts[1])
                    val = int(parts[2])
                    if 0 <= idx < SERVO_COUNT:
                        self.servo_values[idx] = val
                        self.client.send_json({"cmd":"set","id":idx,"val":val})
                elif parts[0] == 'sleep' and len(parts) == 2:
                    t = float(parts[1])
                    time.sleep(t)
                else:
                    # その他コマンドはそのまま送信
                    self.client.send_json({"cmd":line})
            except Exception as e:
                pass
            time.sleep(0.05)
        self.running = False

    def stop(self):
        self.running = False
        if self.thread:
            self.thread.join(timeout=0.5)

# --- メイン関数 ---
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', required=False, default=None)
    parser.add_argument('--baud', type=int, default=921600)
    parser.add_argument('--binary', action='store_true', help='Use binary protocol instead of text JSON')
    parser.add_argument('--width', type=int, default=1200)
    parser.add_argument('--height', type=int, default=700)
    parser.add_argument('--fullscreen', action='store_true')
    args = parser.parse_args()

    pygame.init()
    flags = pygame.FULLSCREEN if args.fullscreen else 0
    screen = pygame.display.set_mode((args.width, args.height), flags)
    pygame.display.set_caption('Bipedal Serial Client')
    clock = pygame.time.Clock()
    font = pygame.font.SysFont('consolas', 18)
    small = pygame.font.SysFont('consolas', 14)

    # シリアルポート自動検出
    port = args.port
    if port is None:
        ports = list(serial.tools.list_ports.comports())
        if ports:
            print('利用可能なシリアルポート:')
            for i, p in enumerate(ports):
                print(f'  [{i}] {p.device} - {p.description}')
            port = ports[0].device
            print(f'自動選択: {port}')
    if port is None:
        print('エラー: シリアルポートが見つかりません。')
        return

    tx_history = []
    client = SerialClient(port, args.baud, binary_mode=args.binary, tx_history=tx_history)
    try:
        print(f'シリアルポート {port} を開いています (baud={args.baud})...')
        client.open()
        print('接続成功!')
    except Exception as e:
        print(f'エラー: シリアルポート {port} を開けませんでした。詳細: {e}')
        return

    script_runner = ScriptRunner(client, [1500]*SERVO_COUNT)
    imu = ImuData()
    servo_values = [1500]*SERVO_COUNT
    last_seq = 0
    last_send_times = [0.0]*SERVO_COUNT
    status_msg = ""
    status_msg_time = 0.0
    rx_history = bytearray()
    script_text = ""
    input_active = False
    copy_status = ""
    copy_status_time = 0.0

    # レイアウト
    slider_rects = []
    slider_area_x = 20
    slider_area_y = 80
    slider_w = 50
    slider_h = int(args.height * 0.34)
    spacing = 20
    for i in range(SERVO_COUNT):
        slider_rects.append((slider_area_x + i*(slider_w+spacing), slider_area_y, slider_w, slider_h))

    cube_w = int(args.width * 0.35)
    cube_h = int(args.height * 0.6)
    cube_area = (slider_area_x + SERVO_COUNT*(slider_w+spacing) + 40, 60, cube_w, cube_h)
    script_area = (20, slider_area_y + slider_h + 30, int(args.width*0.42), int(args.height*0.36))

    protocol_mode = "BINARY" if args.binary else "TEXT"
    running = True

    def draw_sliders(screen, slider_rects, servo_values, font, client, last_send_times):
        mouse = pygame.mouse.get_pos()
        pressed = pygame.mouse.get_pressed()[0]
        updated = False
        small_font = pygame.font.SysFont('consolas', 12)
        for i, rect in enumerate(slider_rects):
            x,y,w,h = rect
            pygame.draw.rect(screen, (50,50,60), rect, border_radius=4)
            pygame.draw.rect(screen, (80,80,90), rect, 2, border_radius=4)
            lbl = font.render(f"CH{i}", True, (255,255,255))
            lbl_rect = lbl.get_rect(center=(x + w//2, y-18))
            screen.blit(lbl, lbl_rect)
            val = servo_values[i]
            t = (val - SLIDER_MIN) / (SLIDER_MAX - SLIDER_MIN)
            handle_y = y + h - int(t * h)
            bar_height = y + h - handle_y
            pygame.draw.rect(screen, (70,140,220), (x+8, handle_y, w-16, bar_height), border_radius=2)
            pygame.draw.rect(screen, (120,200,255), (x+4, handle_y-8, w-8, 16), border_radius=6)
            pygame.draw.circle(screen, (160,220,255), (x+w//2, handle_y), 6)
            vtxt = small_font.render(str(val), True, (220,255,220))
            vtxt_rect = vtxt.get_rect(center=(x + w//2, y+h+12))
            screen.blit(vtxt, vtxt_rect)
            for mark_val in [500, 1000, 1500, 2000, 2500]:
                mark_t = (mark_val - SLIDER_MIN) / (SLIDER_MAX - SLIDER_MIN)
                mark_y = y + h - int(mark_t * h)
                pygame.draw.line(screen, (100,100,110), (x+w-4, mark_y), (x+w, mark_y), 2)
                if mark_val in [500, 1500, 2500]:
                    mark_txt = small_font.render(str(mark_val), True, (140,140,150))
                    screen.blit(mark_txt, (x+w+4, mark_y-6))
            if pressed and x <= mouse[0] <= x+w and y <= mouse[1] <= y+h:
                t_new = (y + h - mouse[1]) / h
                t_new = max(0.0, min(1.0, t_new))
                new_val = int(SLIDER_MIN + t_new * (SLIDER_MAX - SLIDER_MIN))
                if new_val != servo_values[i]:
                    servo_values[i] = new_val
                    updated = True
                    now = time.time()
                    if now - last_send_times[i] > SLIDER_SEND_INTERVAL:
                        last_send_times[i] = now
                        client.send_json({"cmd":"set","id":i,"val":new_val})
        return updated

    def wrap_text(text, font, max_width):
        words = text.split(' ')
        lines = []
        cur = ''
        for w in words:
            test = cur + (' ' if cur else '') + w
            if font.size(test)[0] > max_width:
                if cur:
                    lines.append(cur)
                cur = w
            else:
                cur = test
        if cur:
            lines.append(cur)
        return lines

    ping_button_pressed = False
    while running:
        dt = clock.tick(60) / 1000.0
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if input_active:
                    if event.key == pygame.K_ESCAPE:
                        input_active = False
                    elif event.key == pygame.K_BACKSPACE:
                        script_text = script_text[:-1]
                    elif event.key == pygame.K_RETURN:
                        script_text += '\n'
                    else:
                        script_text += event.unicode
                else:
                    if event.key == pygame.K_ESCAPE:
                        running = False
            elif event.type == pygame.MOUSEBUTTONDOWN:
                mx,my = event.pos
                if script_area[0] <= mx <= script_area[0]+script_area[2] and script_area[1] <= my <= script_area[1]+script_area[3]:
                    input_active = True
                else:
                    input_active = False
                if 560 <= mx <= 640 and 360 <= my <= 390:
                    if not ping_button_pressed:
                        client.send_json({"cmd":"ping"})
                        status_msg = "Ping送信中..."
                        status_msg_time = time.time()
                        ping_button_pressed = True
                if 560 <= mx <= 660 and 400 <= my <= 430:
                    if not script_runner.running:
                        script_runner.start(script_text)
                if 560 <= mx <= 660 and 440 <= my <= 470:
                    script_runner.stop()
                if 700 <= mx <= 860 and args.height-120 <= my <= args.height-90:
                    pyperclip.copy('\n'.join(tx_history))
                    copy_status = "TX履歴をクリップボードにコピーしました"
                    copy_status_time = time.time()
            elif event.type == pygame.MOUSEBUTTONUP:
                mx, my = event.pos
                if 560 <= mx <= 640 and 360 <= my <= 390:
                    ping_button_pressed = False
                if script_area[0] <= mx <= script_area[0]+script_area[2] and script_area[1] <= my <= script_area[1]+script_area[3]:
                    input_active = True
                else:
                    input_active = False

        # --- 受信処理 ---
        while not client.recv_queue.empty():
            line = client.recv_queue.get()
            try:
                obj = json.loads(line)
                last_seq = obj.get('seq', last_seq)
                if 'resp' in obj:
                    resp_type = obj['resp']
                    if resp_type == 'pong':
                        millis = obj.get('millis', 0)
                        status_msg = f'Ping OK! Device millis: {millis}'
                    else:
                        status_msg = f'Response: {resp_type}'
                    status_msg_time = time.time()
                if 'imu' in obj:
                    imu_obj = obj['imu']
                    imu.ax = float(imu_obj.get('ax', imu.ax))
                    imu.ay = float(imu_obj.get('ay', imu.ay))
                    imu.az = float(imu_obj.get('az', imu.az))
                    imu.gx = float(imu_obj.get('gx', imu.gx))
                    imu.gy = float(imu_obj.get('gy', imu.gy))
                    imu.gz = float(imu_obj.get('gz', imu.gz))
                    imu.temp = float(imu_obj.get('temp', imu.temp))
                imu.seq = last_seq
                if 'pos' in obj and isinstance(obj['pos'], list):
                    for i,v in enumerate(obj['pos'][:SERVO_COUNT]):
                        servo_values[i] = int(v)
            except json.JSONDecodeError:
                status_msg = 'JSON parse error'
                status_msg_time = time.time()

        if status_msg and time.time() - status_msg_time > 3.0:
            status_msg = ""

        # --- UI描画 ---
        screen.fill((20, 22, 28))
        draw_sliders(screen, slider_rects, servo_values, font, client, last_send_times)
