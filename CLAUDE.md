# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP-IDF diagnostic tool for ESP32-S3 hardware validation. It uses pure ESP-IDF (not Arduino) to enable FreeRTOS runtime statistics including per-core CPU usage.

**Target Board:** ESP32-S3-DevKitC-1 (N8R2 variant: 8MB Flash, 2MB PSRAM)

## Development Environment

**IDE:** JetBrains CLion with PlatformIO plugin

## Claude Code Tool Path Conventions (CRITICAL)

Most users of this repository are on **Windows with Git Bash**. There are **two different path conventions** you must use depending on context:

### Tool Calls (Read, Edit, Write, Glob, Grep, etc.)

When calling Claude Code tools that accept file paths, you MUST use **Windows-style paths**:

- ✅ Use backslashes: `c:\Projects\REDACTED\app\src\main.ts`
- ✅ Use the full absolute path with drive letter
- ❌ Do NOT use forward slashes: `c:/Projects/REDACTED/app/src/main.ts`
- ❌ Do NOT use Unix-style paths: `/c/Projects/REDACTED/app/src/main.ts`

### Bash Commands

When running commands via the Bash tool, you MUST use **Unix-style paths** (Git Bash convention):

- ✅ Use forward slashes: `/c/Projects/REDACTED/app/src/main.ts`
- ❌ Do NOT use backslashes in bash commands

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

## ESP32-S3 Notes (YD-ESP32-S3 / DevKitC-1)

- Different GPIO numbering than original ESP32
- GPIO 33-34 do not exist on ESP32-S3; GPIO 35-37 reserved for PSRAM on N8R2
- GPIO 38-42, 47 are available for general use
- PSRAM enabled via sdkconfig
- No DAC pins - use I2S or external DAC for audio
- Native USB support available (GPIO 19-20 reserved)
