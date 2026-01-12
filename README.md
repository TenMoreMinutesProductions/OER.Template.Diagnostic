# OER.Tool.Diagnostics

ESP-IDF diagnostic tool for ESP32-S3 hardware validation.

## Purpose

A standalone diagnostic tool to verify ESP32-S3 hardware health before deploying prop firmware. Uses pure ESP-IDF (not Arduino) to enable full FreeRTOS runtime statistics including per-core CPU usage.

## Why ESP-IDF?

Arduino-ESP32 ships with pre-compiled FreeRTOS libraries that have runtime stats disabled. ESP-IDF allows us to enable these features via sdkconfig.

| Feature | Arduino-ESP32 | ESP-IDF |
|---------|---------------|---------|
| FreeRTOS trace facility | Pre-compiled OFF | Configurable |
| CPU usage per core | Not available | Full support |
| Task runtime stats | Not available | Full support |
| Memory diagnostics | Basic | Detailed |

## Features

### Memory Diagnostics
- Heap usage (internal SRAM)
- PSRAM usage (external)
- Largest free block (fragmentation detection)
- Memory high-water marks

### CPU Diagnostics
- Per-core CPU usage (Core 0 / Core 1)
- Idle time tracking
- Task-level CPU consumption

### Task Diagnostics
- Active FreeRTOS tasks
- Task state (running/blocked/ready)
- Core affinity
- Stack high-water mark (overflow detection)
- Task priority

## Hardware

**Target Board:** ESP32-S3-DevKitC-1 (N8R2 variant)
- 8MB Flash
- 2MB PSRAM
- Dual-core Xtensa LX7

## Build Commands

```bash
pio run                  # Build
pio run --target upload  # Upload to ESP32-S3
pio run --target clean   # Clean build
pio device monitor       # Serial monitor (115200 baud)
```

## Expected Output

```
==============================================================
           ESP32-S3 HARDWARE DIAGNOSTIC
==============================================================
  Chip: esp32s3 (rev 0)
  Cores: 2
  Flash: 8 MB
  PSRAM: 2 MB
==============================================================

==============================================================
  Uptime: 00:01:35
==============================================================

-- MEMORY ----------------------------------------------------
 Heap:   45320 / 327680 bytes (13.8% used)
 PSRAM:   8192 / 2097152 bytes ( 0.4% used)
 Largest free block:  278432 bytes
 Min free ever:       298240 bytes
--------------------------------------------------------------

-- CPU -------------------------------------------------------
 Core 0:   2.3% used
 Core 1:   0.8% used
--------------------------------------------------------------

-- TASKS -----------------------------------------------------
 Name             Core  State  Prio  Stack
 ---------------  ----  -----  ----  -----
 diagnostics        1   RUN     5    6144
 main               0   BLK     1    2048
 IDLE0              0   RDY     0     504
 IDLE1              1   RDY     0     504
 esp_timer          0   BLK    22    3584
 ipc0               0   BLK    24    1024
 ipc1               1   BLK    24    1024
--------------------------------------------------------------
```

## Usage

1. Connect ESP32-S3 via USB
2. Build and flash the diagnostic firmware
3. Open serial monitor and observe output
4. When done testing, flash your actual prop firmware

## Project Structure

```
OER.Tool.Diagnostics/
    CMakeLists.txt
    platformio.ini
    sdkconfig.defaults
    partitions.csv
    src/
        CMakeLists.txt
        main.c
```
