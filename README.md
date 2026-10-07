# ESP32-S3 Circuit Tester — No TFT

A compact **ESP32-S3 based low-voltage circuit tester** that works without a TFT display.

The tester provides:

- DC voltage / battery voltage measurement
- Approximate voltage-class detection
- Continuity test for **unpowered** wires/components
- Serial Monitor output
- Local Wi-Fi web dashboard
- Simple, documented wiring
- No external cloud service or API
- No credentials other than the ESP32 local AP password

> **This project is a low-voltage electronics tester, not a multimeter and not a mains tester.**

## 1. What this project does

### Voltage / battery test

Connect a DC source to the protected voltage input:

```text
DUT +  ---- TEST_V+
DUT -  ---- GND
```

The ESP32 reads the voltage through a resistor divider.

Recommended operating range:

```text
0.0 V to 12.0 V DC
```

The circuit has measurement headroom above 12 V, but **12 V is the recommended maximum for normal use**.

Typical sources:

- AA / AAA cells
- 1.5 V batteries
- 3 V supplies
- 3.7 V Li-ion cells
- 5 V USB rails
- 9 V batteries
- 12 V batteries/supplies

Voltage-only battery testing does **not** measure real battery capacity. A battery's percentage/health cannot be determined accurately from one open-circuit voltage reading.

### Continuity test

Connect an **unpowered** wire or component between:

```text
CONT+ ---- DUT ---- GND
```

The ESP32 applies a small test signal and reports:

```text
CONTINUITY
OPEN
```

**Never connect a powered battery or external voltage to the continuity port.**

## 2. Hardware

### Required

- ESP32-S3 development board
- 22 kΩ resistor
- 4.7 kΩ resistor
- 100 nF capacitor
- 1 kΩ resistor
- Breadboard / perfboard
- Test leads
- USB cable / 5 V USB supply

### Recommended protection

For a real handheld tester, add:

- Input fuse or resettable polyfuse
- Reverse-polarity protection
- TVS or suitable clamp protection
- Clearly separated voltage and continuity terminals
- Enclosure
- Insulated test leads

Do not treat the breadboard prototype as a protected commercial multimeter.

## 3. Pinout

| Function | ESP32-S3 GPIO |
|---|---:|
| Voltage ADC | GPIO 4 |
| Continuity drive | GPIO 5 |
| Continuity sense | GPIO 6 |
| Ground | GND |

### Voltage divider

```text
TEST_V+
   |
  22kΩ
   |
   +-------- GPIO4 (ADC)
   |
  4.7kΩ
   |
  GND

GPIO4 ---- 100nF ---- GND
```

For a 12 V input:

```text
Vadc = 12 × 4.7 / (22 + 4.7)
     ≈ 2.10 V
```

This keeps the ADC node well below 3.3 V.

## 4. Continuity wiring

Basic prototype wiring:

```text
GPIO5 ---- 1kΩ ---- CONT+
                       |
                      DUT
                       |
                      GND

CONT+ ---------------- GPIO6
```

GPIO6 uses an internal pull-up.

When the DUT provides a low-resistance path to GND:

```text
GPIO6 = LOW
=> CONTINUITY
```

When the circuit is open:

```text
GPIO6 = HIGH
=> OPEN
```

### Important

The continuity input must be treated as a **separate unpowered test port**.

Do not use it on a live circuit.

## 5. Software

### Arduino IDE

Install:

1. Arduino IDE
2. ESP32 board package
3. Select your ESP32-S3 board
4. Install the normal ESP32 Arduino core
5. Open:

```text
src/ESP32_S3_Circuit_Tester.ino
```

No TFT library is required.

No ESP32Servo library is required.

### Recommended board setting

For a typical ESP32-S3 Dev Module:

```text
Board: ESP32S3 Dev Module
USB CDC On Boot: Enabled
```

Other board options can remain at their normal/default values unless your specific board requires different settings.

## 6. Upload

1. Connect ESP32-S3 by USB.
2. Select the correct serial port.
3. Upload the sketch.
4. Open Serial Monitor.
5. Set baud rate to:

```text
115200
```

You should see something similar to:

```text
================================
 ESP32-S3 CIRCUIT TESTER
================================
AP started: YES
SSID: ESP32S3-Circuit-Tester
Password: change-me-123
Web UI: http://192.168.4.1
```

## 7. Phone / laptop web interface

After the ESP32 starts its access point:

1. Connect your phone or laptop to:

```text
ESP32S3-Circuit-Tester
```

2. Password:

```text
change-me-123
```

3. Open:

```text
http://192.168.4.1
```

The page provides:

- Voltage measurement
- Voltage classification
- Continuity test
- Safety reminders

The web interface is served directly by the ESP32-S3.

No internet connection is required.

## 8. Serial Monitor

The Serial Monitor prints measurements such as:

```text
[VOLTAGE] 4.982 V | LOW VOLTAGE
```

and:

```text
[CONTINUITY] CONTINUITY
```

or:

```text
[CONTINUITY] OPEN
```

## 9. Safety

### Never do these

**DO NOT:**

- Connect mains voltage
- Test 110/120/220/230/240 VAC
- Connect AC directly to GPIO
- Connect a powered battery to the continuity port
- Connect an unknown high-voltage source
- Short a battery through a GPIO
- Assume a voltage reading is a battery-capacity measurement

### Battery polarity

The voltage input is polarity-sensitive.

```text
Battery + -> TEST_V+
Battery - -> GND
```

For a finished handheld tester, use reverse-polarity protection before the ADC divider.

## 10. Accuracy

The ESP32 ADC is not laboratory-grade.

Accuracy depends on:

- ESP32-S3 ADC characteristics
- resistor tolerance
- ADC attenuation
- board noise
- USB/power noise
- temperature
- calibration

For better accuracy:

- use 0.1% resistors
- add the 100 nF ADC capacitor
- average multiple readings
- calibrate against a trusted multimeter
- use a proper external ADC if higher accuracy is required

## 11. Future upgrades

This project is intentionally kept simple and TFT-free.

Possible next hardware revision:

- resistor measurement
- diode test
- LED test
- transistor test
- selectable resistance ranges
- current measurement
- battery load test
- reverse-polarity protection
- fuse/protection stage
- buzzer
- physical mode buttons
- 2.4" ILI9341 TFT version
- rechargeable battery inside the tester

### Battery load testing

A real battery load tester is different from a voltage tester.

A proper load tester needs:

```text
Battery
   |
MOSFET / controlled load
   |
Current sensor
   |
GND
```

The ESP32 can then measure voltage, current and discharge time.

Do not implement battery load testing by connecting a resistor directly to a GPIO.

## 12. Repository structure

```text
ESP32-S3-Circuit-Tester/
├── src/
│   └── ESP32_S3_Circuit_Tester.ino
├── docs/
│   ├── WIRING.md
│   └── SAFETY.md
├── images/
│   └── README.md
├── .gitignore
├── LICENSE
└── README.md
```

## 13. License

MIT License.

See `LICENSE`.

## 14. Project status

**Version:** 1.0.0  
**Display:** No TFT  
**Controller:** ESP32-S3  
**Connection:** Local Wi-Fi + Serial  
**Voltage test:** 0–12 V recommended  
**Continuity:** Unpowered DUT only

This repository is intended as a practical electronics prototype and learning project.
