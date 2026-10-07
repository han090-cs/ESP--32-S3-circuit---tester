# Safety

This is a low-voltage electronics prototype.

## Absolute limits

- Do not connect mains voltage.
- Do not connect 120 VAC or 230/240 VAC.
- Do not connect unknown high-voltage sources.
- Do not use the continuity port on a powered circuit.
- Do not short batteries through ESP32 GPIO pins.

## Battery testing

Voltage measurement is suitable for checking approximate DC voltage.

It does not provide:

- exact state of charge
- internal resistance
- capacity in Ah
- battery health

Those require additional measurement circuitry and controlled test conditions.

## Before connecting a new source

1. Estimate the expected voltage.
2. Confirm it is DC.
3. Confirm it is within the 12 V recommended range.
4. Confirm the polarity.
5. Connect the negative lead to GND.
6. Connect the positive lead to TEST_V+.

For unknown sources, use a real multimeter first.

## Prototype vs finished tool

A breadboard prototype is not equivalent to a commercial multimeter.

For a finished handheld tester, add:

- fuse / PTC
- reverse-polarity protection
- input clamps
- appropriate PCB creepage/clearance
- insulated connectors
- enclosure
- properly rated probes
