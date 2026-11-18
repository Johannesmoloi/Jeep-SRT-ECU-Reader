# Wiring - Jeep SRT ECU Reader

## Parts Needed
- ESP32 Dev Board
- ELM327 Bluetooth / USB Adapter
- USB Cable
- Laptop / Phone with PlatformIO

## Connection

| ESP32 | ELM327 / OBD2 |
| --- | --- |
| 3.3V / 5V | VCC |
| GND | GND |
| GPIO 16 (RX2) | TX |
| GPIO 17 (TX2) | RX |

## Steps
1. Plug ELM327 into Jeep OBD2 port (under steering wheel)
2. Turn Ignition ON - do NOT start engine for code reading
3. Power ESP32 via USB
4. Open Serial Monitor at 115200 baud
5. Press 1 to test connection
6. Press 2 to read fault codes
7. Press 4 for live SRT data

## Notes
- If no connection: check Ignition ON, ELM327 LED blinking
- If garbage data: check baud 38400 for ELM327, 115200 for Serial Monitor
- For Arduino Uno: change Serial2 to Serial in elm327.c
