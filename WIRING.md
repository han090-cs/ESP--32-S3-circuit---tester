# Wiring Guide

## Voltage measurement

```text
                 ESP32-S3
                 +------+
TEST_V+ --22k---| GPIO4|---- ADC
                +------+
                   |
                  4.7k
                   |
                  GND

GPIO4 ---- 100nF ---- GND
```

Connect the DC source:

```text
SOURCE + -> TEST_V+
SOURCE - -> GND
```

Recommended maximum: **12 V DC**.

## Continuity

```text
ESP32 GPIO5 -- 1k -- CONT+
                         |
                        DUT
                         |
                        GND

CONT+ ---------------- GPIO6
```

GPIO6 is configured with an internal pull-up.

- Open DUT -> GPIO6 HIGH
- Conductive DUT to GND -> GPIO6 LOW

Only test **unpowered** components/wires.

## Recommended finished-product protection

For a handheld version:

```text
TEST_V+
   |
 [FUSE]
   |
[REVERSE POLARITY]
   |
[OPTIONAL TVS/CLAMP]
   |
[22k / 4.7k DIVIDER]
   |
 GPIO4
```

The exact protection design should be validated against the maximum expected input voltage and fault conditions.
