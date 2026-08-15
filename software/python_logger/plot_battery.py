import csv
from pathlib import Path

import matplotlib.pyplot as plt


# ========================================
# Project folders
# ========================================

SCRIPT_DIRECTORY = Path(__file__).resolve().parent
LOG_DIRECTORY = SCRIPT_DIRECTORY / "logs"
PLOT_DIRECTORY = SCRIPT_DIRECTORY / "plots"

PLOT_DIRECTORY.mkdir(exist_ok=True)


# ========================================
# Find newest avionics log
# ========================================

log_files = list(LOG_DIRECTORY.glob("avionics_log_*.csv"))

if not log_files:
    print("No avionics log files were found.")
    print("Run serial_logger.py first.")
    raise SystemExit

latest_log = max(
    log_files,
    key=lambda file: file.stat().st_mtime
)

print(f"Using log file: {latest_log.name}")


# ========================================
# Telemetry data lists
# ========================================

time_seconds = []
battery_voltage = []
statuses = []


# ========================================
# Read CSV
# ========================================

with open(latest_log, "r", newline="") as csv_file:

    reader = csv.DictReader(csv_file)

    first_time_ms = None

    for row in reader:

        current_time_ms = int(row["time_ms"])

        if first_time_ms is None:
            first_time_ms = current_time_ms

        elapsed_seconds = (
            current_time_ms - first_time_ms
        ) / 1000.0

        time_seconds.append(elapsed_seconds)

        # Convert millivolts to volts
        battery_voltage.append(
            int(row["battery_mv"]) / 1000.0
        )

        statuses.append(row["status"])


# ========================================
# Make sure telemetry exists
# ========================================

if not time_seconds:
    print("The CSV does not contain telemetry data.")
    raise SystemExit


# ========================================
# Find low-battery samples
# ========================================

low_battery_time = []
low_battery_voltage = []

for index, status in enumerate(statuses):

    if status == "LOW_BATTERY":

        low_battery_time.append(
            time_seconds[index]
        )

        low_battery_voltage.append(
            battery_voltage[index]
        )


# ========================================
# Create battery graph
# ========================================

plt.figure(figsize=(10, 6))

plt.plot(
    time_seconds,
    battery_voltage,
    marker="o",
    label="Measured Battery Voltage"
)

# Low-battery threshold
plt.axhline(
    y=7.0,
    linestyle="--",
    label="Low-Battery Threshold (7.0 V)"
)

# Highlight samples where fault is active
if low_battery_time:

    plt.scatter(
        low_battery_time,
        low_battery_voltage,
        marker="x",
        s=80,
        label="LOW_BATTERY Fault"
    )


# ========================================
# Labels
# ========================================

plt.title("STM32 Real Battery Voltage Monitoring")

plt.xlabel("Elapsed Time (seconds)")
plt.ylabel("Battery Voltage (V)")

plt.grid(True)
plt.legend()
plt.tight_layout()


# ========================================
# Save graph
# ========================================

battery_plot_path = (
    PLOT_DIRECTORY / "real_battery_telemetry.png"
)

plt.savefig(
    battery_plot_path,
    dpi=300
)

print()
print("Battery graph created successfully.")
print(f"Saved to: {battery_plot_path}")


# ========================================
# Display graph
# ========================================

plt.show()
