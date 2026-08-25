import csv
import re
from datetime import datetime
from pathlib import Path

import serial


# ========================================
# Serial port settings
# ========================================

PORT = "COM3"
BAUD_RATE = 115200


# ========================================
# Expected STM32 telemetry format
# ========================================

"""
Example:

TIME_MS=12345,
BATTERY_MV=9720,
STATUS=OK,
ACCEL_X_MG=-10,
ACCEL_Y_MG=-90,
ACCEL_Z_MG=1018,
GYRO_X_MDPS=35,
GYRO_Y_MDPS=70,
GYRO_Z_MDPS=-367,
TEMP_C_X100=2415,
PRESSURE_PA=98112,
ALTITUDE_M_X100=27099
"""

TELEMETRY_PATTERN = re.compile(
    r"^TIME_MS=(\d+),"
    r"BATTERY_MV=(\d+),"
    r"STATUS=([A-Z_]+),"
    r"ACCEL_X_MG=(-?\d+),"
    r"ACCEL_Y_MG=(-?\d+),"
    r"ACCEL_Z_MG=(-?\d+),"
    r"GYRO_X_MDPS=(-?\d+),"
    r"GYRO_Y_MDPS=(-?\d+),"
    r"GYRO_Z_MDPS=(-?\d+),"
    r"TEMP_C_X100=(-?\d+),"
    r"PRESSURE_PA=(\d+),"
    r"ALTITUDE_M_X100=(-?\d+)$"
)


# ========================================
# Project folders
# ========================================

SCRIPT_DIRECTORY = Path(__file__).resolve().parent
LOG_DIRECTORY = SCRIPT_DIRECTORY / "logs"

LOG_DIRECTORY.mkdir(exist_ok=True)


# ========================================
# Create new CSV file
# ========================================

timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

csv_path = (
    LOG_DIRECTORY /
    f"avionics_log_{timestamp}.csv"
)


# ========================================
# Open STM32 serial connection
# ========================================

print("Opening STM32 serial connection...")
print(f"Port: {PORT}")
print(f"Baud rate: {BAUD_RATE}")

serial_connection = serial.Serial(
    PORT,
    BAUD_RATE,
    timeout=1
)

print("Connected.")
print(f"Saving telemetry to: {csv_path}")
print()
print("Press Ctrl+C to stop logging.")
print()


# ========================================
# Open CSV
# ========================================

with open(csv_path, "w", newline="") as csv_file:

    csv_writer = csv.writer(csv_file)

    # CSV column names
    csv_writer.writerow([
        "computer_time",
        "time_ms",
        "battery_mv",
        "status",
        "accel_x_mg",
        "accel_y_mg",
        "accel_z_mg",
        "gyro_x_mdps",
        "gyro_y_mdps",
        "gyro_z_mdps",
        "temp_c",
        "pressure_pa",
        "altitude_m"
    ])

    csv_file.flush()


    # ========================================
    # Continuously read telemetry
    # ========================================

    try:

        while True:

            raw_line = serial_connection.readline()

            line = raw_line.decode(
                "utf-8",
                errors="ignore"
            ).strip()

            if not line:
                continue

            # Show incoming telemetry
            print(line)


            # ========================================
            # Match complete telemetry packet
            # ========================================

            match = TELEMETRY_PATTERN.match(line)

            if match is None:

                # Ignore startup messages such as:
                #
                # Avionics monitor started
                # LSM6DSOX FOUND
                # BMP390 FOUND
                # BMP390 CONFIG OK

                continue


            # ========================================
            # Extract STM32 values
            # ========================================

            time_ms = int(match.group(1))

            battery_mv = int(match.group(2))

            status = match.group(3)

            accel_x_mg = int(match.group(4))
            accel_y_mg = int(match.group(5))
            accel_z_mg = int(match.group(6))

            gyro_x_mdps = int(match.group(7))
            gyro_y_mdps = int(match.group(8))
            gyro_z_mdps = int(match.group(9))

            temp_c_x100 = int(match.group(10))

            pressure_pa = int(match.group(11))

            altitude_m_x100 = int(match.group(12))


            # ========================================
            # Convert environmental data
            # ========================================

            temperature_c = (
                temp_c_x100 / 100.0
            )

            altitude_m = (
                altitude_m_x100 / 100.0
            )


            # ========================================
            # Computer timestamp
            # ========================================

            computer_time = datetime.now().isoformat(
                timespec="milliseconds"
            )


            # ========================================
            # Save one complete telemetry snapshot
            # ========================================

            csv_writer.writerow([
                computer_time,
                time_ms,
                battery_mv,
                status,
                accel_x_mg,
                accel_y_mg,
                accel_z_mg,
                gyro_x_mdps,
                gyro_y_mdps,
                gyro_z_mdps,
                temperature_c,
                pressure_pa,
                altitude_m
            ])

            # Save immediately to disk
            csv_file.flush()


    # ========================================
    # Stop safely
    # ========================================

    except KeyboardInterrupt:

        print()
        print("Logging stopped.")
        print(f"CSV saved to: {csv_path}")


    finally:

        serial_connection.close()

        print("Serial connection closed.")