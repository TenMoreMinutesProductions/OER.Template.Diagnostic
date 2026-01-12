#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_chip_info.h"
#include "esp_psram.h"
#include "esp_flash.h"

#define DIAGNOSTIC_INTERVAL_MS 5000

// ============================================================
//                    CHIP INFO
// ============================================================

static void print_chip_info(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    uint32_t flash_size;
    esp_flash_get_size(NULL, &flash_size);

    printf("\n");
    printf("==============================================================\n");
    printf("           ESP32-S3 HARDWARE DIAGNOSTIC                       \n");
    printf("==============================================================\n");
    printf("  Chip: %s (rev %d)                                           \n",
           CONFIG_IDF_TARGET, chip_info.revision);
    printf("  Cores: %d                                                   \n",
           chip_info.cores);
    printf("  Flash: %lu MB                                               \n",
           flash_size / (1024 * 1024));
    printf("  PSRAM: %d MB                                                \n",
           esp_psram_get_size() / (1024 * 1024));
    printf("==============================================================\n");
    printf("\n");
}

// ============================================================
//                    MEMORY STATS
// ============================================================

static void print_memory_stats(void) {
    // Internal heap
    size_t heap_free = esp_get_free_heap_size();
    size_t heap_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    size_t heap_used = heap_total - heap_free;
    float heap_pct = (heap_used * 100.0f) / heap_total;

    // PSRAM
    size_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t psram_total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t psram_used = psram_total - psram_free;
    float psram_pct = (psram_total > 0) ? (psram_used * 100.0f) / psram_total : 0;

    // Fragmentation indicator
    size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    // Minimum free ever (high-water mark)
    size_t min_free = esp_get_minimum_free_heap_size();

    printf("-- MEMORY ----------------------------------------------------\n");
    printf(" Heap:  %7zu / %7zu bytes (%5.1f%% used)\n",
           heap_used, heap_total, heap_pct);
    printf(" PSRAM: %7zu / %7zu bytes (%5.1f%% used)\n",
           psram_used, psram_total, psram_pct);
    printf(" Largest free block: %7zu bytes\n",
           largest_block);
    printf(" Min free ever:      %7zu bytes\n",
           min_free);
    printf("--------------------------------------------------------------\n");
}

// ============================================================
//                    CPU STATS
// ============================================================

static void print_cpu_stats(void) {
    static uint32_t last_idle0 = 0, last_idle1 = 0;
    static uint32_t last_total = 0;

    TaskHandle_t idle0 = xTaskGetIdleTaskHandleForCore(0);
    TaskHandle_t idle1 = xTaskGetIdleTaskHandleForCore(1);

    if (idle0 == NULL || idle1 == NULL) {
        printf("-- CPU -------------------------------------------------------\n");
        printf(" CPU stats unavailable\n");
        printf("--------------------------------------------------------------\n");
        return;
    }

    // Get task states
    UBaseType_t task_count = uxTaskGetNumberOfTasks();
    TaskStatus_t *task_array = pvPortMalloc(task_count * sizeof(TaskStatus_t));

    if (task_array == NULL) {
        printf("-- CPU -------------------------------------------------------\n");
        printf(" Memory allocation failed\n");
        printf("--------------------------------------------------------------\n");
        return;
    }

    uint32_t total_runtime;
    task_count = uxTaskGetSystemState(task_array, task_count, &total_runtime);

    // Find idle task runtimes
    uint32_t idle0_runtime = 0, idle1_runtime = 0;
    for (UBaseType_t i = 0; i < task_count; i++) {
        if (task_array[i].xHandle == idle0) {
            idle0_runtime = task_array[i].ulRunTimeCounter;
        } else if (task_array[i].xHandle == idle1) {
            idle1_runtime = task_array[i].ulRunTimeCounter;
        }
    }

    printf("-- CPU -------------------------------------------------------\n");

    if (last_total > 0 && total_runtime > last_total) {
        uint32_t delta_total = total_runtime - last_total;
        uint32_t delta_idle0 = idle0_runtime - last_idle0;
        uint32_t delta_idle1 = idle1_runtime - last_idle1;

        // Each core gets half total time
        uint32_t core_time = delta_total / 2;

        float cpu0 = 100.0f - (delta_idle0 * 100.0f / core_time);
        float cpu1 = 100.0f - (delta_idle1 * 100.0f / core_time);

        // Clamp to valid range
        if (cpu0 < 0) cpu0 = 0;
        if (cpu0 > 100) cpu0 = 100;
        if (cpu1 < 0) cpu1 = 0;
        if (cpu1 > 100) cpu1 = 100;

        printf(" Core 0: %5.1f%% used\n", cpu0);
        printf(" Core 1: %5.1f%% used\n", cpu1);
    } else {
        printf(" Collecting baseline...\n");
    }

    printf("--------------------------------------------------------------\n");

    last_idle0 = idle0_runtime;
    last_idle1 = idle1_runtime;
    last_total = total_runtime;

    vPortFree(task_array);
}

// ============================================================
//                    TASK LIST
// ============================================================

static void print_task_list(void) {
    UBaseType_t task_count = uxTaskGetNumberOfTasks();
    TaskStatus_t *task_array = pvPortMalloc(task_count * sizeof(TaskStatus_t));

    if (task_array == NULL) {
        printf("-- TASKS -----------------------------------------------------\n");
        printf(" Memory allocation failed\n");
        printf("--------------------------------------------------------------\n");
        return;
    }

    uint32_t total_runtime;
    task_count = uxTaskGetSystemState(task_array, task_count, &total_runtime);

    printf("-- TASKS -----------------------------------------------------\n");
    printf(" Name             Core  State  Prio  Stack\n");
    printf(" ---------------  ----  -----  ----  -----\n");

    for (UBaseType_t i = 0; i < task_count; i++) {
        const char *state;
        switch (task_array[i].eCurrentState) {
            case eRunning:   state = "RUN"; break;
            case eReady:     state = "RDY"; break;
            case eBlocked:   state = "BLK"; break;
            case eSuspended: state = "SUS"; break;
            case eDeleted:   state = "DEL"; break;
            default:         state = "???"; break;
        }

        int core = task_array[i].xCoreID;
        const char *core_str = (core == 0) ? "0" : (core == 1) ? "1" : "*";

        printf(" %-15s  %3s   %3s    %2d   %5lu\n",
               task_array[i].pcTaskName,
               core_str,
               state,
               (int)task_array[i].uxCurrentPriority,
               (unsigned long)task_array[i].usStackHighWaterMark);
    }

    printf("--------------------------------------------------------------\n");

    vPortFree(task_array);
}

// ============================================================
//                    UPTIME
// ============================================================

static void print_uptime(void) {
    int64_t uptime_us = esp_timer_get_time();
    uint32_t secs = uptime_us / 1000000;
    uint32_t mins = secs / 60;
    uint32_t hours = mins / 60;
    secs %= 60;
    mins %= 60;

    printf("\n==============================================================\n");
    printf("  Uptime: %02lu:%02lu:%02lu\n", hours, mins, secs);
    printf("==============================================================\n\n");
}

// ============================================================
//                    DIAGNOSTIC TASK
// ============================================================

static void diagnostic_task(void *arg) {
    while (1) {
        print_uptime();
        print_memory_stats();
        printf("\n");
        print_cpu_stats();
        printf("\n");
        print_task_list();

        vTaskDelay(pdMS_TO_TICKS(DIAGNOSTIC_INTERVAL_MS));
    }
}

// ============================================================
//                    MAIN
// ============================================================

void app_main(void) {
    print_chip_info();

    printf("Starting diagnostic output every %d seconds...\n\n",
           DIAGNOSTIC_INTERVAL_MS / 1000);

    // Run diagnostics on Core 1 (like Arduino loop)
    xTaskCreatePinnedToCore(
        diagnostic_task,
        "diagnostics",
        8192,
        NULL,
        5,
        NULL,
        1  // Core 1
    );
}
