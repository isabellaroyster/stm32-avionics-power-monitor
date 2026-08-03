"""
STM32 UART CSV Logger

Reads telemetry from the NUCLEO-G071RB over COM3.

Expected telemetry format:
    TIME_MS=12345,BATTERY_MV=8400,STATUS=OK
"""

from __future__ import annotations

import csv
import re
import sys
from datetime import datetime
from pathlib import Path

import serial
from serial import SerialException


PORT = "COM3"
BAUD_RATE = 115200
TIMEOUT_SECONDS = 1

TELEMETRY_PATTERN = re.compile(
    r"^TIME_MS=(\d+),BATTERY_MV=(\d+),STATUS=([A-Z_]+)$"
)


def main() -> int:
    logs_directory = Path(__file__).resolve().parent / "logs"
    logs_directory.mkdir(exist_ok=True)

    filename = datetime.now().strftime(
        "battery_log_%Y-%m-%d_%H-%M-%S.csv"
    )
    csv_path = logs_directory / filename

    print(f"Opening {PORT} at {BAUD_RATE} baud...")
    print("Close PuTTY before running this program.")
    print("Press Ctrl+C to stop logging.\n")

    try:
        with serial.Serial(
            PORT,
            BAUD_RATE,
            timeout=TIMEOUT_SECONDS
        ) as connection:
            with csv_path.open(
                "w",
                newline="",
                encoding="utf-8"
            ) as csv_file:
                writer = csv.writer(csv_file)
                writer.writerow(
                    ["computer_time", "time_ms", "battery_mv", "status"]
                )

                print(
                    f"Connected. Saving data to:\n{csv_path}\n"
                )

                while True:
                    raw_line = connection.readline()

                    if not raw_line:
                        continue

                    line = raw_line.decode(
                        "utf-8",
                        errors="replace"
                    ).strip()

                    if not line:
                        continue

                    print(line)

                    match = TELEMETRY_PATTERN.fullmatch(line)

                    if match is None:
                        continue

                    time_ms = int(match.group(1))
                    battery_mv = int(match.group(2))
                    status = match.group(3)

                    computer_time = datetime.now().isoformat(
                        timespec="milliseconds"
                    )

                    writer.writerow(
                        [computer_time, time_ms, battery_mv, status]
                    )
                    csv_file.flush()

    except KeyboardInterrupt:
        print(
            f"\nLogging stopped. CSV saved to:\n{csv_path}"
        )
        return 0

    except SerialException as error:
        print("\nCould not open or use the serial port.")
        print(f"Reason: {error}")
        print("\nCheck that:")
        print(
            f"1. The Nucleo is connected and appears as {PORT}."
        )
        print("2. PuTTY and other serial programs are closed.")
        print("3. The COM port number is correct.")
        return 1


if __name__ == "__main__":
    sys.exit(main())
