// Copyright (c) 2026 Raleyph. All rights reserved.

#include "logging/LogCapture.hpp"
#include "logging/LogTest.hpp"
#include "wifi/wifi.hpp"
#include "web/WebServer.hpp"

extern "C" void app_main()
{
    ESP_ERROR_CHECK(logging::LogCapture::init());
    ESP_ERROR_CHECK(wifi::initialize(nullptr, nullptr));

    static WebServer web_server{};

    ESP_ERROR_CHECK(web_server.start());

    xTaskCreate(
        log_test_task,
        "log_test",
        3072,
        nullptr,
        3,
        nullptr
    );
}
