"""
Plot the newest simulated battery telemetry CSV log.

Expected CSV columns:
    computer_time,time_ms,battery_mv,status
"""

from __future__ import annotations

import csv
import math
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

    elapsed_seconds: list[float] = []
    battery_volts: list[float] = []
    low_battery_times: list[float] = []
    low_battery_volts: list[float] = []

    first_time_ms: int | None = None
    previous_battery_mv: int | None = None

    with newest_csv.open("r", newline="", encoding="utf-8") as csv_file:
        reader = csv.DictReader(csv_file)

        for row in reader:
            time_ms = int(row["time_ms"])
            battery_mv = int(row["battery_mv"])
            status = row["status"]

            if first_time_ms is None:
                first_time_ms = time_ms

            elapsed_s = (time_ms - first_time_ms) / 1000.0

            # Insert a gap when the simulated battery resets from 6.0 V to 8.4 V.
            # This prevents the graph from drawing a misleading vertical line.
            if (
                previous_battery_mv is not None
                and battery_mv > previous_battery_mv
            ):
                elapsed_seconds.append(math.nan)
                battery_volts.append(math.nan)

            elapsed_seconds.append(elapsed_s)
            battery_volts.append(battery_mv / 1000.0)

            if status == "LOW_BATTERY":
                low_battery_times.append(elapsed_s)
                low_battery_volts.append(battery_mv / 1000.0)

            previous_battery_mv = battery_mv

    if not battery_volts:
        raise ValueError(f"The CSV contains no battery telemetry: {newest_csv}")

    print(f"Plotting: {newest_csv}")

    plt.figure()
    plt.plot(
        elapsed_seconds,
        battery_volts,
        marker="o",
        label="Battery voltage",
    )
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
    plt.xlabel("Elapsed Test Time (seconds)")
    plt.ylabel("Battery Voltage (V)")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
