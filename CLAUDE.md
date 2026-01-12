# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP-IDF diagnostic tool for ESP32-S3 hardware validation. It uses pure ESP-IDF (not Arduino) to enable FreeRTOS runtime statistics including per-core CPU usage.

**Target Board:** ESP32-S3-DevKitC-1 (N8R2 variant: 8MB Flash, 2MB PSRAM)

## Development Environment

**IDE:** JetBrains CLion with PlatformIO plugin

## Build Commands

```bash
pio run                    # Build
pio run --target upload    # Upload to ESP32-S3
pio run --target clean     # Clean build
pio device monitor         # Serial monitor (115200 baud)
```

## Project Structure

```
src/
    CMakeLists.txt    -> ESP-IDF component registration
    main.c            -> Diagnostic functions and app_main()
```

## Key Configuration

### sdkconfig.defaults
Enables FreeRTOS runtime stats:
- `CONFIG_FREERTOS_USE_TRACE_FACILITY=y`
- `CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS=y`
- `CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID=y`

### Diagnostic Features
- Memory: Heap, PSRAM, fragmentation, high-water marks
- CPU: Per-core usage via idle task tracking
- Tasks: State, priority, core affinity, stack usage

## ESP-IDF vs Arduino

This project uses ESP-IDF because Arduino-ESP32 has pre-compiled FreeRTOS libraries with runtime stats disabled. ESP-IDF allows full configuration of these features.

## ESP32-S3 Notes

- Different GPIO numbering than original ESP32
- PSRAM enabled via sdkconfig
- No DAC pins - use I2S or external DAC for audio
- Native USB support available
