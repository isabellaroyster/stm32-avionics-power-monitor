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
# Telemetry format
# ========================================

"""
Expected STM32 telemetry:

TIME_MS=12345,BATTERY_MV=8400,STATUS=OK,
ACCEL_X_MG=-10,ACCEL_Y_MG=-90,ACCEL_Z_MG=1018,
GYRO_X_MDPS=35,GYRO_Y_MDPS=70,GYRO_Z_MDPS=-367
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
    r"GYRO_Z_MDPS=(-?\d+)$"
)


# ========================================
# Create logs folder
# ========================================

SCRIPT_DIRECTORY = Path(__file__).resolve().parent
LOG_DIRECTORY = SCRIPT_DIRECTORY / "logs"

LOG_DIRECTORY.mkdir(exist_ok=True)


# ========================================
# Create a new CSV file
# ========================================

timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

csv_path = LOG_DIRECTORY / f"avionics_log_{timestamp}.csv"


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
# Open CSV file
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
        "gyro_z_mdps"
    ])

    csv_file.flush()


    # ========================================
    # Continuously read STM32 telemetry
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

            # Show everything received from STM32
            print(line)


            # ========================================
            # Check for valid telemetry packet
            # ========================================

            match = TELEMETRY_PATTERN.match(line)

            if match is None:
                # Startup messages such as:
                # "Avionics monitor started"
                # "LSM6DSOX FOUND"
                # are intentionally ignored by the CSV logger.
                continue


            # ========================================
            # Extract telemetry values
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


            # ========================================
            # Record computer timestamp
            # ========================================

            computer_time = datetime.now().isoformat(
                timespec="milliseconds"
            )


            # ========================================
            # Write one complete telemetry row
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
                gyro_z_mdps
            ])

            # Immediately save data to disk
            csv_file.flush()


    # ========================================
    # Stop safely with Ctrl+C
    # ========================================

    except KeyboardInterrupt:

        print()
        print("Logging stopped.")
        print(f"CSV saved to: {csv_path}")


    finally:

        serial_connection.close()

        print("Serial connection closed.")