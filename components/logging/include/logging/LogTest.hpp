// Copyright (c) 2026 Raleyph. All rights reserved.

#pragma once

#include <cstdint>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void log_test_task(void* arg) {
    std::uint32_t counter = 0;

    while (true) {
        ++counter;

        switch (counter % 4) {
            case 0:
                ESP_LOGE("LogTest", "Test error #%lu",
                         static_cast<unsigned long>(counter));
                break;

            case 1:
                ESP_LOGW("LogTest", "Test warning #%lu",
                         static_cast<unsigned long>(counter));
                break;

            case 2:
                ESP_LOGI("LogTest", "Test info #%lu",
                         static_cast<unsigned long>(counter));
                break;

            case 3:
                ESP_LOGD("LogTest", "Test debug #%lu",
                         static_cast<unsigned long>(counter));
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
