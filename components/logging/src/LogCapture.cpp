// Copyright (c) 2026 Raleyph. All rights reserved.

#include "logging/LogCapture.hpp"

#include "logging/LogMessage.hpp"

#include <cstdio>
#include <cstdarg>

namespace logging {

QueueHandle_t LogCapture::queue_ = nullptr;
vprintf_like_t LogCapture::originalVprintf_ = nullptr;

esp_err_t LogCapture::init() {
    if (queue_ != nullptr) {
        return ESP_OK;
    }

    queue_ = xQueueCreate(32, sizeof(LogMessage));

    if (queue_ == nullptr) {
        return ESP_ERR_NO_MEM;
    }

    originalVprintf_ = esp_log_set_vprintf(vprintfHandler);

    return ESP_OK;
}

int LogCapture::vprintfHandler(const char* format, va_list args) {
    va_list copy;
    va_copy(copy, args);

    const int result = originalVprintf_(format, args);

    if (queue_ != nullptr) {
        LogMessage message = {};

        vsnprintf(message.text, sizeof(message.text), format, copy);

        xQueueSend(queue_, &message, 0);
    }

    va_end(copy);

    return result;
}

QueueHandle_t LogCapture::getQueue() {
    return queue_;
}

}
