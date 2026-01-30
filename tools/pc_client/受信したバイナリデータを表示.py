# -*- coding: utf-8 -*-
import serial

ser = serial.Serial("COM8", 921600, timeout=0)
buf = bytearray()

while True:
    data = ser.read(64)
    buf += data
    while len(buf) >= 81:
        # 先頭がAA 55か確認
        if buf[0] == 0xAA and buf[1] == 0x55:
            pkt = buf[:81]
            print(" ".join(f"{x:02X}" for x in pkt))
            buf = buf[81:]
        else:
            # 先頭がAA 55でなければ1バイト捨てる
            buf = buf[1:]