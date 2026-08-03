"""
Plot the newest simulated battery telemetry CSV log.

Expected CSV columns:
    computer_time,time_ms,battery_mv,status
"""

from __future__ import annotations

import csv
from pathlib import Path

import matplotlib.pyplot as plt


LOW_BATTERY_THRESHOLD_MV = 7000


def main() -> None:
    script_folder = Path(__file__).resolve().parent
    logs_folder = script_folder / "logs"

    csv_files = sorted(
        logs_folder.glob("battery_log_*.csv"),
        key=lambda path: path.stat().st_mtime,
        reverse=True,
    )

    if not csv_files:
        raise FileNotFoundError(
            "No battery CSV files were found. Run serial_logger.py first."
        )

    newest_csv = csv_files[0]

    time_seconds: list[float] = []
    battery_volts: list[float] = []
    low_battery_times: list[float] = []
    low_battery_volts: list[float] = []

    with newest_csv.open("r", newline="", encoding="utf-8") as csv_file:
        reader = csv.DictReader(csv_file)

        for row in reader:
            time_s = int(row["time_ms"]) / 1000.0
            battery_v = int(row["battery_mv"]) / 1000.0
            status = row["status"]

            time_seconds.append(time_s)
            battery_volts.append(battery_v)

            if status == "LOW_BATTERY":
                low_battery_times.append(time_s)
                low_battery_volts.append(battery_v)

    if not battery_volts:
        raise ValueError(f"The CSV contains no battery telemetry: {newest_csv}")

    print(f"Plotting: {newest_csv}")

    plt.figure()
    plt.plot(time_seconds, battery_volts, marker="o", label="Battery voltage")
    plt.axhline(
        LOW_BATTERY_THRESHOLD_MV / 1000.0,
        linestyle="--",
        label="Low-battery threshold",
    )

    if low_battery_times:
        plt.scatter(
            low_battery_times,
            low_battery_volts,
            marker="x",
            s=80,
            label="LOW_BATTERY",
        )

    plt.title("Simulated Battery Telemetry")
    plt.xlabel("STM32 Uptime (seconds)")
    plt.ylabel("Battery Voltage (V)")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
