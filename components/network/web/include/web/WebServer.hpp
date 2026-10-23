// Copyright (c) 2026 Raleyph. All rights reserved.

#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

#include "logging/LogMessage.hpp"

class DiagnosticsService;

struct SystemInfo;
struct PartitionInfo;

struct StaticFile {
    const char* uri;
    const char* contentType;
    const uint8_t* start;
    const uint8_t* end;
};

class WebServer {
public:
    explicit WebServer();

    ~WebServer();

    WebServer(const WebServer&) = delete;
    WebServer& operator=(const WebServer&) = delete;

    esp_err_t start();
    void stop();

private:
    struct WsBroadcastContext {
        const WebServer* self;
        char* json;
    };

    static esp_err_t handle_index(httpd_req_t* req);
    static esp_err_t handle_system_info(httpd_req_t* req);
    static esp_err_t handle_websocket(httpd_req_t *req);
    static esp_err_t handle_static(httpd_req_t* req);

    static void ws_broadcast_callback(void* arg);

    void queue_ws_broadcast(char* json) const;
    void send_ws_broadcast(const char* json) const;

    static void metrics_task(void* arg);
    void broadcast_metrics() const;

    static void log_task(void* arg);
    void broadcast_log(const logging::LogMessage& message) const;

    httpd_handle_t server_ = nullptr;

    TaskHandle_t metrics_task_handle_ = nullptr;
    TaskHandle_t logs_task_handle_ = nullptr;
};
