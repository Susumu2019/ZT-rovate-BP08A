#include "SerialSender.h"
#include "../Settings.h"
#include <ArduinoJson.h>

bool SerialSender::begin() {
    // UART2 初期化（TX/RX ピンは config.h の指定、ボーレートは Settings から）
    uint32_t baud = Settings::getInstance().getSerialBaud();
    Serial2.begin(baud, SERIAL_8N1, SERIAL_RX_PIN, SERIAL_TX_PIN);
    delay(50);
    _ready = true;
    Serial.printf("Serial: ready @%lu baud TX=%d RX=%d\n", (unsigned long)baud, SERIAL_TX_PIN, SERIAL_RX_PIN);
    Serial.printf("Serial2 object size: %zu bytes\n", sizeof(Serial2));
    Serial.println("Serial2: now ready for text command reception");
    return true;
}

bool SerialSender::sendControl(
    float ax, float ay, float az,
    float gx, float gy, float gz,
    uint8_t tempByte,
    const uint16_t* servoPos8,
    const uint16_t* servoOff8,
    uint16_t seq,
    bool includeImu) {
    if (!_ready) return false;
    
    // IMU出力無効時は0を送信
    float _ax = includeImu ? ax : 0.0f;
    float _ay = includeImu ? ay : 0.0f;
    float _az = includeImu ? az : 0.0f;
    float _gx = includeImu ? gx : 0.0f;
    float _gy = includeImu ? gy : 0.0f;
    float _gz = includeImu ? gz : 0.0f;
    uint8_t _t8 = includeImu ? tempByte : 0;
    
    uint8_t buf[81];
    // バイナリパケット生成（先頭に0xAA, 0x55のSYNCヘッダーを付加）
    // SYNCヘッダーは CommProtocol::buildControlPacket() 内で付加される
    size_t n = CommProtocol::buildControlPacket(buf, sizeof(buf),
        _ax, _ay, _az, _gx, _gy, _gz, _t8,
        servoPos8, servoOff8, seq, true /* add ETX */);
    if (n == 0) return false;
    Serial2.write(buf, n);
    return true;
}

bool SerialSender::sendControlText(
    float ax, float ay, float az,
    float gx, float gy, float gz,
    uint8_t tempByte,
    const uint16_t* servoPos8,
    const uint16_t* servoOff8,
    uint16_t seq,
    bool includeImu) {
    if (!_ready) return false;
    
    // JSON形式で送信（軽量化のため小数点2桁に丸める）
    JsonDocument doc;
    doc["seq"] = seq;
    
    // IMUデータ（有効時のみ含む）
    if (includeImu) {
        JsonObject imu = doc["imu"].to<JsonObject>();
        imu["ax"] = round(ax * 100) / 100.0f;
        imu["ay"] = round(ay * 100) / 100.0f;
        imu["az"] = round(az * 100) / 100.0f;
        imu["gx"] = round(gx * 100) / 100.0f;
        imu["gy"] = round(gy * 100) / 100.0f;
        imu["gz"] = round(gz * 100) / 100.0f;
        imu["temp"] = tempByte;
    }
    JsonArray spos = doc["pos"].to<JsonArray>();
    for (int i = 0; i < 8; i++) {
        spos.add(servoPos8 ? servoPos8[i] : 0);
    }
    JsonArray soff = doc["off"].to<JsonArray>();
    for (int i = 0; i < 8; i++) {
        soff.add(servoOff8 ? servoOff8[i] : 0);
    }
    
    serializeJson(doc, Serial2);
    Serial2.println();  // 改行追加
    return true;
}

bool SerialSender::processTextCommand(uint16_t* servoPos8, uint16_t* servoOff8) {
    if (!_ready) return false;
    
    bool commandProcessed = false;                                   
    //uint16_t val = (_rxBinBuf[10] << 8) | _rxBinBuf[11];
   /// シリアルから受信
    while (Serial2.available()) {
        char c = Serial2.read();
        Serial.printf("[RX] 0x%02X ('%c')\n", (uint8_t)c, (c >= 32 && c < 127) ? c : '?');

        // 改行で1コマンド終了
        if (c == '\n' || c == '\r') {
            if (_rxBuffer.length() > 0) {
                // 受信バッファからスペース・タブを除去
                String cleanBuffer = "";
                for (size_t i = 0; i < _rxBuffer.length(); ++i) {
                    char cc = _rxBuffer[i];
                    if (cc != ' ' && cc != '\t') cleanBuffer += cc;
                }
                // バッファ内容が"p"だけならpong応答
                if (cleanBuffer == "p") {
                    Serial2.println("ping ok.");
                    Serial.println("SerialCmd: single 'p' (with CR/LF) received, pong sent");
                    commandProcessed = true;
                    _rxBuffer = "";
                    continue;
                }
                // バッファ内容が"?"だけならコマンド説明を返す
                if (cleanBuffer == "?") {
                    Serial2.println("[コマンド一覧]");
                    Serial2.println("p         : ping応答 (通信確認)\r\n 例: p");
                    Serial2.println("{\"cmd\":\"ping\"} : ping応答(JSON)\r\n 例: {\\\"cmd\\\":\\\"ping\\\"}");
                    Serial2.println("{\"cmd\":\"servo\",\"pos\":[90,90,...]} : サーボ一括制御（角度0～180,中立90）\r\n 例: {\\\"cmd\\\":\\\"servo\\\",\\\"pos\\\":[90,90,90,90,90,90,90,90]}");
                    Serial2.println("{\"cmd\":\"offset\",\"off\":[0,0,...]} : サーボオフセット一括設定\r\n 例: {\\\"cmd\\\":\\\"offset\\\",\\\"off\\\":[0,0,0,0,0,0,0,0]}");
                    Serial2.println("{\"cmd\":\"set\",\"id\":0,\"val\":90} : 単一サーボ制御（角度0～180,中立90）\r\n 例: {\\\"cmd\\\":\\\"set\\\",\\\"id\\\":0,\\\"val\\\":90}");
                    Serial2.println("{\"cmd\":\"reset\"} : サーボ全リセット\r\n 例: {\\\"cmd\\\":\\\"reset\\\"}");
                    Serial2.println("?         : この説明を表示\r\n 例: ?");
                    Serial.println("SerialCmd: '?' received, help sent");
                    commandProcessed = true;
                    _rxBuffer = "";
                    continue;
                }
                Serial.printf("SerialCmd: received buffer: %s\n", cleanBuffer.c_str());

                // JSON解析
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, cleanBuffer);

                if (!error) {
                    Serial.printf("SerialCmd: JSON parsed successfully\n");
                    String cmd = doc["cmd"].as<String>();
                    Serial.printf("SerialCmd: cmd = %s\n", cmd.c_str());
                    // サーボ位置コマンド: {"cmd":"servo","pos":[val0,val1,...]}（角度0～180,中立90）
                    if (doc["cmd"] == "servo" && doc["pos"].is<JsonArray>()) {
                        JsonArray posArray = doc["pos"].as<JsonArray>();
                        int i = 0;
                        for (JsonVariant v : posArray) {
                            if (i >= 8) break;
                            if (servoPos8) servoPos8[i] = v.as<uint16_t>(); // 0～180度で格納
                            i++;
                        }
                        commandProcessed = true;
                        Serial.printf("SerialCmd: servo pos updated (deg, count=%d)\n", i);
                    }
                    // set_allコマンド: {"cmd":"set_all","vals":[val0,val1,...]}（角度0～180,中立90）
                    else if (doc["cmd"] == "set_all" && doc["vals"].is<JsonArray>()) {
                        JsonArray valsArray = doc["vals"].as<JsonArray>();
                        int i = 0;
                        for (JsonVariant v : valsArray) {
                            if (i >= 8) break;
                            if (servoPos8) servoPos8[i] = v.as<uint16_t>(); // 0～180度で格納
                            i++;
                        }
                        commandProcessed = true;
                        Serial.printf("SerialCmd: set_all pos updated (deg, count=%d)\n", i);
                    }
                    // サーボオフセットコマンド: {"cmd":"offset","off":[val0,val1,...]}
                    else if (doc["cmd"] == "offset" && doc["off"].is<JsonArray>()) {
                        JsonArray offArray = doc["off"].as<JsonArray>();
                        int i = 0;
                        for (JsonVariant v : offArray) {
                            if (i >= 8) break;
                            if (servoOff8) servoOff8[i] = v.as<uint16_t>();
                            i++;
                        }
                        commandProcessed = true;
                        Serial.printf("SerialCmd: servo offset updated (count=%d)\n", i);
                    }
                    // 単一サーボ制御: {"cmd":"set","id":0,"val":90}（角度0～180,中立90）
                    else if (doc["cmd"] == "set" && doc["id"].is<int>() && doc["val"].is<uint16_t>()) {
                        int id = doc["id"].as<int>();
                        uint16_t val = doc["val"].as<uint16_t>();
                        if (id >= 0 && id < 8 && servoPos8) {
                            servoPos8[id] = val; // 0～180度で格納
                            commandProcessed = true;
                            Serial.printf("SerialCmd: servo[%d] = %u deg\n", id, val);
                        }
                    }
                    // 全サーボリセット: {"cmd":"reset"}（角度0～180,中立90）
                    else if (doc["cmd"] == "reset") {
                        for (int i = 0; i < 8; i++) {
                            if (servoPos8) servoPos8[i] = 90; // 中立90度
                            if (servoOff8) servoOff8[i] = 0;
                        }
                        commandProcessed = true;
                        Serial.println("SerialCmd: all servos reset to 90 deg");
                    }
                    // 通信確認: {"cmd":"ping"}
                    else if (doc["cmd"] == "ping") {
                        JsonDocument resp;
                        resp["resp"] = "ping";
                        resp["millis"] = millis();
                        serializeJson(resp, Serial2);
                        Serial2.println();
                        commandProcessed = true;
                        Serial.println("SerialCmd: ping received, pong sent");
                    }
                } else {
                    Serial.printf("SerialCmd: JSON parse error: %s\n", error.c_str());
                }

                _rxBuffer = "";
            }
        } else {
            // バッファに追加（サイズ制限）
            if (_rxBuffer.length() < MAX_BUFFER_SIZE) {
                _rxBuffer += c;
            } else {
                // バッファオーバーフロー対策（古いデータを破棄）
                _rxBuffer = "";
                Serial.println("SerialCmd: buffer overflow, reset");
            }
        }
    }
    
    return commandProcessed;
}

bool SerialSender::processBinaryCommand(uint16_t* servoPos8, uint16_t* servoOff8) {
    if (!_ready) return false;
    
    bool commandProcessed = false;
    
    // バイナリフレームを受信
    while (Serial2.available()) {
        uint8_t c = Serial2.read();
        
        // SYNC 待ち
        if (_rxBinIdx == 0) {
            if (c == CommProtocol::SYNC0) {
                _rxBinBuf[_rxBinIdx++] = c;
            }
        } else if (_rxBinIdx == 1) {
            if (c == CommProtocol::SYNC1) {
                _rxBinBuf[_rxBinIdx++] = c;
            } else {
                _rxBinIdx = 0;  // SYNC失敗、リセット
            }
        } else {
            // ヘッダ＆ペイロードを受信
            _rxBinBuf[_rxBinIdx++] = c;
            
            // 最小フレーム: SYNC(2) + VER(1) + TYPE(1) + SEQ(2) + LEN(2) + CRC(2) + ETX(1) = 11B
            // 最大フレーム: SYNC(2) + VER(1) + TYPE(1) + SEQ(2) + LEN(2) + PAYLOAD(57) + CRC(2) + ETX(1) = 68B
            if (_rxBinIdx >= 11) {
                uint8_t ver = _rxBinBuf[2];
                uint8_t type = _rxBinBuf[3];
                // Little-endianで解釈
                uint16_t seq = (_rxBinBuf[5] << 8) | _rxBinBuf[4];
                uint16_t len = (_rxBinBuf[7] << 8) | _rxBinBuf[6];

                // フルフレーム受信確認: SYNC(2) + VER(1) + TYPE(1) + SEQ(2) + LEN(2) + PAYLOAD(len) + CRC(2) + ETX(1)
                size_t expected_size = 2 + 1 + 1 + 2 + 2 + len + 2 + 1;

                if (_rxBinIdx >= expected_size) {
                    // 受信バイナリパケットを16進数で出力（この行を追加）【約182行目】
                    Serial.print("Recv packet: ");
                    for (size_t i = 0; i < expected_size; ++i) {
                        Serial.printf("%02X ", _rxBinBuf[i]);
                    }
                    Serial.println();

                    // CRC検証（SYNC前後を除く）
                    uint16_t calc_crc = CommProtocol::crc16_ccitt(_rxBinBuf + 2, 2 + 2 + 2 + len);
                    // CRC16はリトルエンディアンで格納されているため、下位バイト＋上位バイトで取得
                    uint16_t frame_crc = _rxBinBuf[8 + len] | (_rxBinBuf[8 + len + 1] << 8);
                    //uint16_t frame_crc = _rxBinBuf[2 + 2 + 2 + len] | (_rxBinBuf[2 + 2 + 2 + len + 1] << 8);
                    uint8_t etx = _rxBinBuf[_rxBinIdx - 1];

                    if (calc_crc == frame_crc && etx == 0x7E) {
                        // フレーム有効
                        if (type == 0x02) {  // TYPE_COMMAND
                            uint8_t cmd = _rxBinBuf[8];
                            if (cmd == 0x01) {  // SET_SERVO (id, val)
                                if (len >= 4) {
                                    uint8_t id = _rxBinBuf[9];
                                    uint16_t val = _rxBinBuf[10] | (_rxBinBuf[11] << 8); // リトルエンディアンで取得
                                    //uint16_t val = (_rxBinBuf[10] << 8) | _rxBinBuf[11];
                                    if (id < 8 && servoPos8) {
                                        servoPos8[id] = val;
                                        commandProcessed = true;
                                        Serial.printf("BinCmd: servo[%d] = %u\n", id, val);
                                    }
                                }
                            } else if (cmd == 0x02) {  // SET_ALL_SERVOS (8*2)
                                if (len >= 17 && servoPos8) {
                                    for (int i = 0; i < 8; i++) {
                                        servoPos8[i] = _rxBinBuf[9 + i*2] | (_rxBinBuf[10 + i*2] << 8);
                                        //servoPos8[i] = (_rxBinBuf[9 + i*2] << 8) | _rxBinBuf[10 + i*2];
                                    }
                                    commandProcessed = true;
                                    Serial.println("BinCmd: all servos updated");
                                }
                            } else if (cmd == 0x03) {  // RESET
                                if (servoPos8) {
                                    for (int i = 0; i < 8; i++) {
                                        servoPos8[i] = 90; // 中立角度（0～180度）
                                        if (servoOff8) servoOff8[i] = 0;
                                    }
                                    commandProcessed = true;
                                    Serial.println("BinCmd: reset");
                                }
                            } else if (cmd == 0x04) {  // PING
                                // バイナリPONG応答: SYNC, VER, TYPE=0x02, SEQ, LEN=1, CMD=0x04, CRC, ETX
                                uint8_t pong_frame[16];
                                uint16_t pong_seq = seq; // 受信SEQをそのまま返す
                                uint8_t pong_cmd = 0x04;
                                // ヘッダ: SYNC(2), VER(1), TYPE(1), SEQ(2), LEN(2)
                                pong_frame[0] = CommProtocol::SYNC0;
                                pong_frame[1] = CommProtocol::SYNC1;
                                pong_frame[2] = CommProtocol::VERSION;
                                pong_frame[3] = 0x02; // TYPE_COMMAND
                                pong_frame[4] = (uint8_t)(pong_seq & 0xFF);
                                pong_frame[5] = (uint8_t)((pong_seq >> 8) & 0xFF);
                                pong_frame[6] = 1; // LEN=1 (CMDのみ)
                                pong_frame[7] = 0;
                                pong_frame[8] = pong_cmd;
                                // CRC計算（VER～CMDまで）
                                uint16_t pong_crc = CommProtocol::crc16_ccitt(&pong_frame[2], 7);
                                pong_frame[9] = (uint8_t)(pong_crc & 0xFF);
                                pong_frame[10] = (uint8_t)((pong_crc >> 8) & 0xFF);
                                pong_frame[11] = 0x7E; // ETX
                                Serial2.write(pong_frame, 12);
                                commandProcessed = true;
                                Serial.println("BinCmd: ping (binary pong sent)");

                                //pong_frame[0] = CommProtocol::SYNC0;
                                // LcdManager::getInstance().drawText(0, 200, "pong_frame SYNC0 set"); // LCDに表示                         
                            }
                        }
                    } else {
                        Serial.printf("BinCmd: CRC/ETX fail (crc=%04X vs %04X, etx=%02X)\n", calc_crc, frame_crc, etx);
                    }

                    _rxBinIdx = 0;  // 次のフレーム待ち
                }
            }
        }
    }
    
    return commandProcessed;
}
