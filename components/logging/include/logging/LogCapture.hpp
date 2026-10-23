// Copyright (c) 2026 Raleyph. All rights reserved.

#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace logging {

class LogCapture {
public:
    static esp_err_t init();
    static QueueHandle_t getQueue();

private:
    static int vprintfHandler(const char* format, va_list args);

    static QueueHandle_t queue_;
    static vprintf_like_t originalVprintf_;
};

}
