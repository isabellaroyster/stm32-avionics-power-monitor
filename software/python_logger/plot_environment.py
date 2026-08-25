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

temperature_c = []
pressure_hpa = []
altitude_m = []


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

        time_seconds.append(
            elapsed_seconds
        )

        # Temperature is already stored in Celsius
        temperature_c.append(
            float(row["temp_c"])
        )

        # Convert Pascals to hectopascals
        pressure_hpa.append(
            float(row["pressure_pa"]) / 100.0
        )

        # Altitude is already stored in meters
        altitude_m.append(
            float(row["altitude_m"])
        )


# ========================================
# Verify data exists
# ========================================

if not time_seconds:
    print("The CSV does not contain telemetry data.")
    raise SystemExit


# ========================================
# Temperature graph
# ========================================

plt.figure(figsize=(10, 6))

plt.plot(
    time_seconds,
    temperature_c,
    marker="o",
    label="Temperature"
)

plt.title("BMP390 Temperature Telemetry")

plt.xlabel("Elapsed Time (seconds)")
plt.ylabel("Temperature (°C)")

plt.grid(True)
plt.legend()
plt.tight_layout()

temperature_plot_path = (
    PLOT_DIRECTORY /
    "bmp390_temperature.png"
)

plt.savefig(
    temperature_plot_path,
    dpi=300
)


# ========================================
# Pressure graph
# ========================================

plt.figure(figsize=(10, 6))

plt.plot(
    time_seconds,
    pressure_hpa,
    marker="o",
    label="Atmospheric Pressure"
)

plt.title("BMP390 Atmospheric Pressure")

plt.xlabel("Elapsed Time (seconds)")
plt.ylabel("Pressure (hPa)")

plt.grid(True)
plt.legend()
plt.tight_layout()

pressure_plot_path = (
    PLOT_DIRECTORY /
    "bmp390_pressure.png"
)

plt.savefig(
    pressure_plot_path,
    dpi=300
)


# ========================================
# Altitude graph
# ========================================

plt.figure(figsize=(10, 6))

plt.plot(
    time_seconds,
    altitude_m,
    marker="o",
    label="Barometric Altitude"
)

plt.title("BMP390 Barometric Altitude")

plt.xlabel("Elapsed Time (seconds)")
plt.ylabel("Estimated Altitude (m)")

plt.grid(True)
plt.legend()
plt.tight_layout()

altitude_plot_path = (
    PLOT_DIRECTORY /
    "bmp390_altitude.png"
)

plt.savefig(
    altitude_plot_path,
    dpi=300
)


# ========================================
# Finished
# ========================================

print()
print("Environmental graphs created successfully.")
print()

print(
    f"Temperature graph: {temperature_plot_path}"
)

print(
    f"Pressure graph: {pressure_plot_path}"
)

print(
    f"Altitude graph: {altitude_plot_path}"
)


# ========================================
# Display graphs
# ========================================

plt.show()