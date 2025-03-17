# Arduino GSM Security Alarm System

## Overview
This project is an Arduino-based security alarm system that detects motion using PIR sensors and sends alerts via SMS and phone calls. The system operates with a GSM module (SIM800L) and has been tested with **Hamrah Aval (MCI)** in Iran, but it can potentially work with other operators in different countries.

## Features
- Motion detection using **two PIR sensors**
- Sends **SMS alerts** upon motion detection
- Activates a **relay** to trigger external security devices (e.g., siren, lights)
- Makes **phone calls** to predefined numbers
- Detects **power failure** and sends alerts
- Sends an **armed status message** when the system is turned on

## Hardware Requirements
- **Arduino** (Uno, Mega, or compatible board)
- **SIM800L GSM module**
- **PIR motion sensors** (x2)
- **Relay module** (to activate external devices)
- **Buzzer** (for sound alerts)
- **Power failure detection circuit**

## Circuit Diagram
```
PIR Sensor 1 (Front Door)  -> Arduino Pin 11
PIR Sensor 2 (Darkzed Room) -> Arduino Pin 12
GSM Module RX  -> Arduino Pin 3
GSM Module TX  -> Arduino Pin 2
Relay Module   -> Arduino Pin 8
Buzzer         -> Arduino Pin 9
Power Detect   -> Arduino Pin 13
```

## Installation & Setup
1. **Connect the hardware** as per the circuit diagram.
2. **Insert a SIM card** into the GSM module (preferably Hamrah Aval in Iran).
3. **Upload the provided Arduino code** to your board.
4. **Power the system** and wait for initialization.

## Configuration
Modify the following section in the code to update **phone numbers** for alerts:
```cpp
const String PHONE_NUMBERS[] = {"+989120000000", "+989030000000", "+1954000000"};
```

## How It Works
1. When motion is detected:
   - The system **sends an SMS** to predefined numbers.
   - The **relay is activated** (for sirens or lights).
   - The system **calls the predefined numbers**.
2. If power failure occurs:
   - The system **sends a power failure SMS**.
3. When power is restored:
   - The system **sends a power restored SMS**.
4. On startup:
   - The system sends an **armed notification SMS**.

## Tested On
✅ **Hamrah Aval (MCI) - Iran**

⚠️ The system *may* work with other operators, but additional configurations might be required.

## Future Enhancements
- Add **remote control** via SMS commands
- Implement **battery backup monitoring**
- Integrate **WiFi & MQTT support** for smart home automation

## License
This project is open-source under the **GNU GENERAL PUBLIC LICENSE**. Feel free to modify and contribute!
