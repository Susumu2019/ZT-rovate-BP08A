import serial

ser = serial.Serial(
    port="COM8",
    baudrate=115200,
    timeout=0  # ノンブロッキング
)

buf = bytearray()

while True:
    data = ser.read(64)
    for b in data:
        if b in (0x0D, 0x0A):  # CR or LF
            if buf:
                try:
                    # ASCII / UTF-8 を想定
                    text = buf.decode("utf-8", errors="replace")
                except Exception:
                    text = "<decode error>"
                print(text, flush=True)
                buf.clear()
        else:
            buf.append(b)
