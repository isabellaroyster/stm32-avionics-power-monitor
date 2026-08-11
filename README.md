# STM32 Avionics Power Monitor

An STM32-based avionics monitoring system that measures battery voltage, current, pressure, temperature, acceleration, and rotation while transmitting telemetry for Python-based testing and analysis.

## Project Status

This project is currently in development.

## System Requirements

1. The system shall collect telemetry at 10 samples per second.
2. The system shall measure an external voltage between 0 and 9 volts.
3. The system shall report temperature in degrees Celsius.
4. The system shall report pressure in pascals.
5. The system shall report acceleration on three axes.
6. The system shall report angular velocity on three axes.
7. The system shall measure load voltage and current.
8. The system shall transmit telemetry through UART.
9. The system shall identify when a sensor cannot be reached.
10. The system shall produce a low-voltage warning below 6 volts.
11. The computer shall save received telemetry as a CSV file.
12. The Python program shall generate graphs of voltage, current, temperature, pressure, and acceleration.
13. The complete system shall operate for at least 30 minutes without crashing.
14. Each requirement shall have a documented pass/fail verification result.

## Planned Hardware

- STM32 Nucleo development board
- Pressure and temperature sensor
- Accelerometer and gyroscope
- Current sensor
- Battery-voltage sensing circuit
- Breadboard and jumper wires

## Planned Software

- Embedded C
- STM32CubeIDE
- Python
- Git and GitHub
- Altium Designer

## Current Milestone

Integrated an LSM6DSOX 6-axis IMU with the STM32 NUCLEO-G071RB over I2C.

The firmware currently:
- Detects the LSM6DSOX over I2C
- Verifies the WHO_AM_I register (`0x6C`)
- Reads 3-axis accelerometer data
- Converts acceleration to milli-g (mg)
- Reads 3-axis gyroscope data
- Converts angular velocity to milli-degrees per second (mdps)
- Sends live sensor and battery telemetry over UART

I also soldered the IMU header pins, wired the sensor to the STM32, configured the device registers in embedded C, and verified both orientation and rotation measurements using real motion tests.
