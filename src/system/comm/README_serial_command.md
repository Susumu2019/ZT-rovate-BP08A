
# シリアル通信プロトコル詳細

## テキスト通信（JSON/1文字コマンド）

- 通信方式: UART2 (デフォルト 921600bps)
- 改行(\r, \n)でコマンド区切り
- 1文字コマンド、JSONコマンド両対応

### 送信例
```
p\n
{"cmd": "ping"}\n
{"cmd": "servo", "pos": [1500,1500,1500,1500,1500,1500,1500,1500]}\n
```
### 応答例
```
pong
{"resp": "pong", "millis": 1234567}
```

### コマンド一覧
- `p` : pong応答
- `{ "cmd": "ping" }` : pong応答(JSON)
- `{ "cmd": "servo", "pos": [...] }` : サーボ一括制御
- `{ "cmd": "offset", "off": [...] }` : サーボオフセット一括設定
- `{ "cmd": "set", "id": n, "val": v }` : 単一サーボ制御
- `{ "cmd": "reset" }` : サーボ全リセット
- `?` : コマンド説明表示

#### 注意事項
- 1コマンドごとに改行(\r, \n)が必要です。
- JSONコマンドはダブルクォートで記述してください。
- 不正なコマンドやJSONパースエラー時は応答しません。

---

## バイナリ通信

- デバイス設定: AppSetupで「Mode = BINARY」を選択し、再起動
- フレーム形式: `[AA55][VER][TYPE][SEQ2][LEN2][PAYLOAD][CRC16][7E]`
	- `AA55`: シンク (リトルエンディアン)
	- `VER=1`, `TYPE=0x01` (センサ送信) / `0x02` (コマンド受信)
	- `SEQ2`: シーケンス番号(2byte)
	- `LEN2`: ペイロード長(2byte)
	- `PAYLOAD`: データ本体
	- `CRC16-CCITT`: ヘッダ+ペイロードに対する
	- `7E`: ETX(終端, UART用)

### 受信例（センサデータ TYPE=0x01）
| フィールド | サイズ | 内容 |
|---|---|---|
| ax,ay,az | float*3 | 加速度 |
| gx,gy,gz | float*3 | ジャイロ |
| temp | uint8 | 温度 |
| pos[8] | uint16*8 | サーボ位置 |
| off[8] | uint16*8 | サーボオフセット |

### 送信例（コマンド TYPE=0x02）
- `cmd=0x01` (SET_SERVO): payload = [id:2][val:2]
- `cmd=0x02` (SET_ALL): payload = [pos[0:16]]
- `cmd=0x03` (RESET): payload = empty
- `cmd=0x04` (PING): payload = empty

#### 注意事項
- CRC16-CCITT(0x1021, init 0xFFFF)で検証
- 1フレームごとに7E終端
- 受信/送信ともにバイナリ形式

---

最終更新: 2026-01-26
