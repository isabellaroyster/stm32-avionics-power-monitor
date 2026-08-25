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

log_files = list(
    LOG_DIRECTORY.glob("avionics_log_*.csv")
)

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
# Telemetry lists
# ========================================

time_seconds = []
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
        statuses.append(row["status"])


# ========================================
# Make sure telemetry exists
# ========================================

if not time_seconds:
    print("The CSV contains no telemetry data.")
    raise SystemExit


# ========================================
# Convert status names into graph levels
# ========================================

STATUS_LEVELS = {
    "OK": 0,
    "LOW_BATTERY": 1,
    "BMP_FAULT": 2,
    "IMU_FAULT": 3,
    "IMU_BMP_FAULT": 4,
    "LOW_BATT_BMP_FAULT": 5,
    "LOW_BATT_IMU_FAULT": 6,
    "LOW_BATT_IMU_BMP_FAULT": 7
}


status_values = []

for status in statuses:

    status_values.append(
        STATUS_LEVELS.get(status, 8)
    )


# ========================================
# Create system health graph
# ========================================

plt.figure(figsize=(12, 6))

plt.step(
    time_seconds,
    status_values,
    where="post",
    linewidth=2
)

plt.scatter(
    time_seconds,
    status_values,
    s=25
)


# ========================================
# Y-axis labels
# ========================================

plt.yticks(
    list(STATUS_LEVELS.values()),
    list(STATUS_LEVELS.keys())
)


# ========================================
# Graph labels
# ========================================

plt.title(
    "STM32 Avionics System Health and Fault Recovery"
)

plt.xlabel(
    "Elapsed Time (seconds)"
)

plt.ylabel(
    "System Status"
)

plt.grid(True)
plt.tight_layout()


# ========================================
# Save graph
# ========================================

health_plot_path = (
    PLOT_DIRECTORY /
    "system_health_fault_recovery.png"
)

plt.savefig(
    health_plot_path,
    dpi=300
)


# ========================================
# Print test summary
# ========================================

print()
print("System health graph created successfully.")
print()

print("Statuses detected during test:")

for status in sorted(set(statuses)):
    print(f"  {status}")

print()
print(
    f"Graph saved to: {health_plot_path}"
)


# ========================================
# Display graph
# ========================================

plt.show()