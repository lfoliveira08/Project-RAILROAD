# ESP32 Firmware — Modbus TCP Slave Devices

Arduino sketches for the ESP32 field devices used in Project RAILROAD. Each ESP32 runs as an independent Modbus TCP server (port 502) polled by the OpenPLC master.

| Sketch | Device |
|--------|--------|
| `CrossingGate_ESP32/` | Crossing gate servo + IR obstacle sensor + RGB warning light |
| `PowerTrack/` | Track power relay (NO), E-stop, potentiometer |
| `switchtest/` | Switch-track servo actuation |
| `Radar/` | Proximity/speed sensing (servo + potentiometer demo) |

## ⚙️ WiFi credentials

Credentials have been replaced with placeholders — **set your own before flashing**:

```cpp
#define WLAN_SSID "YOUR_WIFI_SSID"
#define WLAN_PASS "YOUR_WIFI_PASSWORD"
// or, in sketches using variables:
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

Never commit real WiFi credentials. Keep them local only.

## 🌐 Network settings

The static IP, gateway and subnet values in the sketches are **placeholders** from the reserved documentation range (`192.0.2.x`). Set them to match your own isolated lab network before flashing. Do not publish your real addresses.

## Libraries
Arduino core for ESP32, plus `WiFi`, a Modbus TCP library (e.g. `ModbusIP`/`modbus-esp8266`), and `ESP32Servo` where servos are used.
