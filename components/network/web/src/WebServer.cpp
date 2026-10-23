// Copyright (c) 2026 Raleyph. All rights reserved.

#include "web/WebServer.hpp"

#include <cstdio>

#include "esp_log.h"

#include "diagnostics/DiagnosticsMetrics.hpp"
#include "diagnostics/Diagnostics.hpp"
#include "diagnostics/DiagnosticsSerializer.hpp"

#include "logging/LogCapture.hpp"

//////////////////////////////////////////////////////////////////////////
// WebServer

static constexpr auto TAG = "WebServer";

extern "C" {

    // HTML
    extern const unsigned char html_index_start[] asm("_binary_index_html_start");
    extern const unsigned char html_index_end[] asm("_binary_index_html_end");

    // Styles
    extern const uint8_t css_style_start[] asm("_binary_style_css_start");
    extern const uint8_t css_style_end[] asm("_binary_style_css_end");

    extern const uint8_t css_ui_start[] asm("_binary_ui_css_start");
    extern const uint8_t css_ui_end[] asm("_binary_ui_css_end");

    extern const uint8_t css_tabs_start[] asm("_binary_tabs_css_start");
    extern const uint8_t css_tabs_end[] asm("_binary_tabs_css_end");

    // JS
    extern const uint8_t js_tabs_start[] asm("_binary_tabs_js_start");
    extern const uint8_t js_tabs_end[] asm("_binary_tabs_js_end");

    extern const uint8_t js_fetch_start[] asm("_binary_fetch_js_start");
    extern const uint8_t js_fetch_end[] asm("_binary_fetch_js_end");

    extern const uint8_t js_update_start[] asm("_binary_update_js_start");
    extern const uint8_t js_update_end[] asm("_binary_update_js_end");

    extern const uint8_t js_dom_start[] asm("_binary_dom_js_start");
    extern const uint8_t js_dom_end[] asm("_binary_dom_js_end");

    extern const uint8_t js_formatters_start[] asm("_binary_formatters_js_start");
    extern const uint8_t js_formatters_end[] asm("_binary_formatters_js_end");

    extern const uint8_t js_chart_start[] asm("_binary_chart_js_start");
    extern const uint8_t js_chart_end[] asm("_binary_chart_js_end");

    extern const uint8_t js_logs_start[] asm("_binary_logs_js_start");
    extern const uint8_t js_logs_end[] asm("_binary_logs_js_end");

}

static const StaticFile files[] = {
    {"/", "text/html", html_index_start, html_index_end},

    {"/styles/style.css", "text/css", css_style_start, css_style_end},
    {"/styles/ui.css", "text/css", css_ui_start, css_ui_end},
    {"/styles/tabs.css", "text/css", css_tabs_start, css_tabs_end},

    {"/scripts/tabs.js", "application/javascript", js_tabs_start, js_tabs_end},
    {"/scripts/fetch.js", "application/javascript", js_fetch_start, js_fetch_end},
    {"/scripts/update.js", "application/javascript", js_update_start, js_update_end},
    {"/scripts/dom.js", "application/javascript", js_dom_start, js_dom_end},
    {"/scripts/formatters.js", "application/javascript", js_formatters_start, js_formatters_end},
    {"/scripts/chart.js", "application/javascript", js_chart_start, js_chart_end},
    {"/scripts/logs.js", "application/javascript", js_logs_start, js_logs_end},
};

//////////////////////////////////////////////////////////////////////////
// Public API

WebServer::WebServer() = default;

WebServer::~WebServer() {
    stop();
}

esp_err_t WebServer::start() {
    if (server_ != nullptr) {
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 8;
    config.uri_match_fn = httpd_uri_match_wildcard;

    esp_err_t err = httpd_start(&server_, &config);

    if (err != ESP_OK) {
        return err;
    }

    httpd_uri_t index_uri = {};
    index_uri.uri = "/";
    index_uri.method = HTTP_GET;
    index_uri.handler = handle_index;
    index_uri.user_ctx = this;

    httpd_uri_t system_info_uri = {};
    system_info_uri.uri = "/api/system";
    system_info_uri.method = HTTP_GET;
    system_info_uri.handler = handle_system_info;
    system_info_uri.user_ctx = this;

    httpd_uri_t ws_uri = {};
    ws_uri.uri = "/ws";
    ws_uri.method = HTTP_GET;
    ws_uri.handler = handle_websocket;
    ws_uri.user_ctx = this;
    ws_uri.is_websocket = true;

    httpd_uri_t static_uri = {};
    static_uri.uri = "/*";
    static_uri.method = HTTP_GET;
    static_uri.handler = handle_static;
    static_uri.user_ctx = this;

    const httpd_uri_t* uris[] = {
        &index_uri,
        &system_info_uri,
        &ws_uri,
        &static_uri
    };

    for (const auto* uri : uris) {
        err = httpd_register_uri_handler(server_, uri);

        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register URI: %s", uri->uri);
            stop();
            return err;
        }
    }

    if (xTaskCreate(metrics_task, "ws_metrics", 4096, this, 5, &metrics_task_handle_) != pdPASS) {
        stop();
        return ESP_ERR_NO_MEM;
    }

    if (xTaskCreate(log_task, "ws_logs", 4096, this, 5, &logs_task_handle_) != pdPASS) {
        stop();
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "HTTP server started");

    return ESP_OK;
}

void WebServer::stop() {
    if (server_ != nullptr) {
        return;
    }

    httpd_stop(server_);
    server_ = nullptr;

    ESP_LOGI(TAG, "HTTP server stopped");
}

//////////////////////////////////////////////////////////////////////////
// API - Handlers

esp_err_t WebServer::handle_index(httpd_req_t *req) {
    const auto size = html_index_end - html_index_start;

    httpd_resp_set_type(req, "text/html; charset=utf-8");

    return httpd_resp_send(
        req,
        reinterpret_cast<const char *>(html_index_start),
        size
    );
}

esp_err_t WebServer::handle_system_info(httpd_req_t *req) {
    const diagnostics::SystemInfo info = diagnostics::get_system_info();
    const char* json = diagnostics::serialization::serialize_system_info(info);

    if (!json) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_type(req, "application/json");

    const esp_err_t result = httpd_resp_sendstr(req, json);

    return result;
}

esp_err_t WebServer::handle_websocket(httpd_req_t* req) {
    if (req->method == HTTP_GET) {
        return ESP_OK;
    }

    return ESP_OK;
}

esp_err_t WebServer::handle_static(httpd_req_t* req) {
    for (const auto& file : files) {
        if (strcmp(req->uri, file.uri) != 0) {
            continue;
        }

        httpd_resp_set_type(req, file.contentType);

        return httpd_resp_send(
            req,
            reinterpret_cast<const char*>(file.start),
            file.end - file.start
        );
    }

    return httpd_resp_send_err(
        req,
        HTTPD_404_NOT_FOUND,
        "File not found"
    );
}

//////////////////////////////////////////////////////////////////////////
// API - WebSockets

void WebServer::ws_broadcast_callback(void *arg) {
    const auto* ctx = static_cast<WsBroadcastContext*>(arg);

    ctx->self->send_ws_broadcast(ctx->json);

    cJSON_free(ctx->json);
    delete ctx;
}

void WebServer::queue_ws_broadcast(char *json) const {
    if (json == nullptr) {
        return;
    }

    if (server_ == nullptr) {
        cJSON_free(json);
        return;
    }

    auto* ctx = new (std::nothrow) WsBroadcastContext{
        .self = this,
        .json = json
    };

    if (ctx == nullptr) {
        cJSON_free(json);
        return;
    }

    if (httpd_queue_work(server_, ws_broadcast_callback, ctx) != ESP_OK) {
        cJSON_free(json);
        delete ctx;
    }
}

void WebServer::send_ws_broadcast(const char* json) const {
    if (server_ == nullptr || json == nullptr) {
        return;
    }

    size_t client_count = CONFIG_LWIP_MAX_SOCKETS;
    int clients[CONFIG_LWIP_MAX_SOCKETS];

    if (httpd_get_client_list(server_, &client_count, clients) != ESP_OK) {
        return;
    }

    if (client_count == 0) {
        return;
    }

    httpd_ws_frame_t frame{};
    frame.type = HTTPD_WS_TYPE_TEXT;
    frame.payload = reinterpret_cast<uint8_t*>(const_cast<char*>(json));
    frame.len = strlen(json);

    for (size_t i = 0; i < client_count; ++i) {
        const int fd = clients[i];

        if (httpd_ws_get_fd_info(server_, fd) != HTTPD_WS_CLIENT_WEBSOCKET) {
            continue;
        }

        const esp_err_t err = httpd_ws_send_frame_async(server_, fd, &frame);

        if (err != ESP_OK) {
            ESP_LOGW(TAG, "WebSocket send failed (fd=%d): %s", fd, esp_err_to_name(err));
            httpd_sess_trigger_close(server_, fd);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Metrics (WebSockets)

void WebServer::metrics_task(void* arg) {
    const auto* self = static_cast<WebServer*>(arg);

    while (self->server_ != nullptr) {
        self->broadcast_metrics();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(nullptr);
}

void WebServer::broadcast_metrics() const {
    const diagnostics::SystemMetrics metrics = diagnostics::get_dynamic_info();
    char* json = diagnostics::serialization::serialize_dynamic_metrics(metrics);
    queue_ws_broadcast(json);
}

//////////////////////////////////////////////////////////////////////////
// Logs (WebSockets)

void WebServer::log_task(void* arg) {
    const auto* self = static_cast<WebServer*>(arg);

    logging::LogMessage message{};

    while (self->server_ != nullptr) {
        if (xQueueReceive(logging::LogCapture::getQueue(), &message, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (self->server_ == nullptr) {
                break;
            }

            self->broadcast_log(message);
        }
    }

    vTaskDelete(nullptr);
}

void WebServer::broadcast_log(const logging::LogMessage& message) const {
    char* json = diagnostics::serialization::serialize_log(message);
    queue_ws_broadcast(json);
}
