import csv
from pathlib import Path

import matplotlib.pyplot as plt


# ========================================
# Find project folders
# ========================================

SCRIPT_DIRECTORY = Path(__file__).resolve().parent
LOG_DIRECTORY = SCRIPT_DIRECTORY / "logs"
PLOT_DIRECTORY = SCRIPT_DIRECTORY / "plots"

# Create plots folder automatically if it does not exist
PLOT_DIRECTORY.mkdir(exist_ok=True)


# ========================================
# Find newest avionics CSV log
# ========================================

log_files = list(LOG_DIRECTORY.glob("avionics_log_*.csv"))

if not log_files:
    print("No avionics log files were found.")
    print("Run serial_logger.py first to create a log.")
    raise SystemExit

latest_log = max(
    log_files,
    key=lambda file: file.stat().st_mtime
)

print(f"Using log file: {latest_log.name}")


# ========================================
# Lists for telemetry data
# ========================================

time_seconds = []

accel_x_g = []
accel_y_g = []
accel_z_g = []

gyro_x_dps = []
gyro_y_dps = []
gyro_z_dps = []


# ========================================
# Read CSV data
# ========================================

with open(latest_log, "r", newline="") as csv_file:

    reader = csv.DictReader(csv_file)

    first_time_ms = None

    for row in reader:

        current_time_ms = int(row["time_ms"])

        # Make graph time start at 0 seconds
        if first_time_ms is None:
            first_time_ms = current_time_ms

        elapsed_seconds = (
            current_time_ms - first_time_ms
        ) / 1000.0

        time_seconds.append(elapsed_seconds)


        # ========================================
        # Convert accelerometer mg -> g
        # ========================================

        accel_x_g.append(
            int(row["accel_x_mg"]) / 1000.0
        )

        accel_y_g.append(
            int(row["accel_y_mg"]) / 1000.0
        )

        accel_z_g.append(
            int(row["accel_z_mg"]) / 1000.0
        )


        # ========================================
        # Convert gyro mdps -> degrees/second
        # ========================================

        gyro_x_dps.append(
            int(row["gyro_x_mdps"]) / 1000.0
        )

        gyro_y_dps.append(
            int(row["gyro_y_mdps"]) / 1000.0
        )

        gyro_z_dps.append(
            int(row["gyro_z_mdps"]) / 1000.0
        )


# ========================================
# Make sure data exists
# ========================================

if not time_seconds:
    print("The CSV file does not contain telemetry data.")
    raise SystemExit


# ========================================
# Accelerometer graph
# ========================================

plt.figure(figsize=(10, 6))

plt.plot(
    time_seconds,
    accel_x_g,
    label="X-axis"
)

plt.plot(
    time_seconds,
    accel_y_g,
    label="Y-axis"
)

plt.plot(
    time_seconds,
    accel_z_g,
    label="Z-axis"
)

# Gravity reference lines
plt.axhline(
    y=1.0,
    linestyle="--",
    label="+1 g"
)

plt.axhline(
    y=-1.0,
    linestyle="--",
    label="-1 g"
)

plt.title("LSM6DSOX 3-Axis Acceleration")

plt.xlabel("Elapsed Time (seconds)")
plt.ylabel("Acceleration (g)")

plt.grid(True)
plt.legend()
plt.tight_layout()


# ========================================
# Save accelerometer graph
# ========================================

accelerometer_plot_path = (
    PLOT_DIRECTORY / "accelerometer_telemetry.png"
)

plt.savefig(
    accelerometer_plot_path,
    dpi=300
)


# ========================================
# Gyroscope graph
# ========================================

plt.figure(figsize=(10, 6))

plt.plot(
    time_seconds,
    gyro_x_dps,
    label="X-axis"
)

plt.plot(
    time_seconds,
    gyro_y_dps,
    label="Y-axis"
)

plt.plot(
    time_seconds,
    gyro_z_dps,
    label="Z-axis"
)

# Zero rotation reference
plt.axhline(
    y=0,
    linestyle="--"
)

plt.title("LSM6DSOX 3-Axis Angular Velocity")

plt.xlabel("Elapsed Time (seconds)")
plt.ylabel("Angular Velocity (degrees/second)")

plt.grid(True)
plt.legend()
plt.tight_layout()


# ========================================
# Save gyroscope graph
# ========================================

gyroscope_plot_path = (
    PLOT_DIRECTORY / "gyroscope_telemetry.png"
)

plt.savefig(
    gyroscope_plot_path,
    dpi=300
)


# ========================================
# Tell user where files were saved
# ========================================

print()
print("Graphs created successfully.")
print()
print(
    f"Accelerometer graph saved to: "
    f"{accelerometer_plot_path}"
)

print(
    f"Gyroscope graph saved to: "
    f"{gyroscope_plot_path}"
)


# ========================================
# Display both graph windows
# ========================================

plt.show()