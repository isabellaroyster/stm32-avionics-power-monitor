"""
Plot the newest STM32 uptime CSV log.

The script looks inside the local "logs" folder, finds the newest
uptime_log_*.csv file, and plots STM32 uptime in seconds.
"""

from __future__ import annotations

import csv
from pathlib import Path

import matplotlib.pyplot as plt


def main() -> None:
    script_folder = Path(__file__).resolve().parent
    logs_folder = script_folder / "logs"

    csv_files = sorted(
        logs_folder.glob("uptime_log_*.csv"),
        key=lambda path: path.stat().st_mtime,
        reverse=True,
    )

    if not csv_files:
        raise FileNotFoundError(
            "No uptime CSV files were found. Run serial_logger.py first."
        )

    newest_csv = csv_files[0]

    sample_numbers: list[int] = []
    uptime_seconds: list[float] = []

    with newest_csv.open("r", newline="", encoding="utf-8") as csv_file:
        reader = csv.DictReader(csv_file)

        for sample_number, row in enumerate(reader, start=1):
            sample_numbers.append(sample_number)
            uptime_seconds.append(int(row["time_ms"]) / 1000.0)

    if not uptime_seconds:
        raise ValueError(f"The CSV file contains no telemetry rows: {newest_csv}")

    print(f"Plotting: {newest_csv}")

    plt.figure()
    plt.plot(sample_numbers, uptime_seconds, marker="o")
    plt.title("STM32 Uptime Telemetry")
    plt.xlabel("Sample Number")
    plt.ylabel("STM32 Uptime (seconds)")
    plt.grid(True)
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
