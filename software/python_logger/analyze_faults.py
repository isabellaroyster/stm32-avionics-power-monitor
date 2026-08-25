import csv
from pathlib import Path
from collections import defaultdict


# ========================================
# Project folders
# ========================================

SCRIPT_DIRECTORY = Path(__file__).resolve().parent
LOG_DIRECTORY = SCRIPT_DIRECTORY / "logs"


# ========================================
# Find newest telemetry log
# ========================================

log_files = list(
    LOG_DIRECTORY.glob("avionics_log_*.csv")
)

if not log_files:
    print("No avionics logs found.")
    raise SystemExit


latest_log = max(
    log_files,
    key=lambda file: file.stat().st_mtime
)

print(f"Using log: {latest_log.name}")
print()


# ========================================
# Read telemetry
# ========================================

times = []
statuses = []

with open(latest_log, "r", newline="") as csv_file:

    reader = csv.DictReader(csv_file)

    for row in reader:

        times.append(
            int(row["time_ms"]) / 1000.0
        )

        statuses.append(
            row["status"]
        )


if not times:
    print("No telemetry data found.")
    raise SystemExit


# ========================================
# Normalize time to start at zero
# ========================================

start_time = times[0]

times = [
    time_value - start_time
    for time_value in times
]


# ========================================
# Analyze fault events
# ========================================

fault_event_count = defaultdict(int)
fault_total_duration = defaultdict(float)

successful_recoveries = 0

current_status = statuses[0]
event_start_time = times[0]


for index in range(1, len(statuses)):

    new_status = statuses[index]

    if new_status != current_status:

        event_end_time = times[index]

        duration = (
            event_end_time -
            event_start_time
        )

        # Record completed fault
        if current_status != "OK":

            fault_event_count[current_status] += 1

            fault_total_duration[current_status] += (
                duration
            )

        # Detect recovery:
        # FAULT -> OK
        if (
            current_status != "OK"
            and new_status == "OK"
        ):
            successful_recoveries += 1

        current_status = new_status
        event_start_time = times[index]


# ========================================
# Handle final event
# ========================================

final_duration = (
    times[-1] -
    event_start_time
)

if current_status != "OK":

    fault_event_count[current_status] += 1

    fault_total_duration[current_status] += (
        final_duration
    )


# ========================================
# Overall test information
# ========================================

total_test_time = times[-1]

final_status = statuses[-1]


# ========================================
# Print report
# ========================================

print("========================================")
print(" STM32 AVIONICS RELIABILITY TEST REPORT")
print("========================================")
print()

print(
    f"Total test time: "
    f"{total_test_time:.1f} seconds"
)

print()


# ========================================
# Print individual fault results
# ========================================

if fault_event_count:

    for fault_status in fault_event_count:

        print(fault_status)

        print(
            f"  Events: "
            f"{fault_event_count[fault_status]}"
        )

        print(
            f"  Total duration: "
            f"{fault_total_duration[fault_status]:.1f} s"
        )

        print()

else:

    print("No faults detected.")
    print()


# ========================================
# Recovery information
# ========================================

print(
    f"Successful recoveries: "
    f"{successful_recoveries}"
)

print(
    f"Final system state: "
    f"{final_status}"
)

print()


# ========================================
# Simple pass/fail result
# ========================================

if (
    successful_recoveries > 0
    and final_status == "OK"
):
    print(
        "RESULT: PASS - "
        "System detected faults and returned "
        "to normal operation."
    )

elif final_status != "OK":

    print(
        "RESULT: ATTENTION - "
        "System finished test in a fault state."
    )

else:

    print(
        "RESULT: PASS - "
        "System finished in normal operation."
    )