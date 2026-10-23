// Copyright (c) 2026 Raleyph. All rights reserved.

#include "diagnostics/DiagnosticsSerializer.hpp"

#include <cstring>

#include "utils/JsonUtils.hpp"
#include "logging/LogMessage.hpp"

namespace diagnostics::serialization {

//////////////////////////////////////////////////////////////////////////
// Static System Info

void add_partition(cJSON* parent, const char* key, const PartitionInfo& partition) {
    using namespace JsonUtils;

    cJSON* partition_json = cJSON_AddObjectToObject(parent, key);

    if (!partition_json) {
        return;
    }

    add_string(partition_json, "label", partition.label);

    cJSON_AddNumberToObject(partition_json, "size", partition.size);

    if (partition.firmware_size.has_value()) {
        cJSON_AddNumberToObject(partition_json, "firmwareSize", *partition.firmware_size);
    } else {
        cJSON_AddNullToObject(partition_json, "firmwareSize");
    }

    add_string(partition_json, "firmwareVersion", partition.firmware_version.c_str());
    add_string(partition_json, "state", partition.ota_state);
}

char* serialize_system_info(const SystemInfo& info) {
    using namespace JsonUtils;

    cJSON* root = cJSON_CreateObject();

    if (!root) {
        return nullptr;
    }

    cJSON_AddStringToObject(root, "type", "system");

    cJSON* data = cJSON_AddObjectToObject(root, "data");

    if (!data) {
        cJSON_Delete(root);
        return nullptr;
    }

    cJSON* partitions = cJSON_AddObjectToObject(data, "partitions");

    // Partitions
    add_partition(partitions, "ota_0", info.ota_0);
    add_partition(partitions, "ota_1", info.ota_1);

    // Partition roles
    add_string(data, "runningPartition", info.running_partition);
    add_string(data, "bootPartition", info.boot_partition);
    add_string(data, "nextOtaPartition", info.next_ota_partition);

    add_boolean(data, "isRollbackEnabled", info.is_rollback_enabled);

    // Flash
    add_number(data, "totalFlashSize", info.total_flash_size);

    // Security
    add_boolean(data, "isFlashEncrypted", info.is_flash_encrypted);
    add_boolean(data, "isSecureBootEnabled", info.is_secure_boot_enabled);
    add_boolean(data, "isJtagDisabled", info.is_jtag_disabled);
    add_boolean(data, "isNvsEncrypted", info.is_nvs_encrypted);

    // Device information
    add_string(data, "ip", info.ip.data());
    add_string(data, "mac", info.mac_address.data());

    add_string(data, "firmwareVersion", info.firmware_version.c_str());
    add_string(data, "idfVersion", info.idf_version);

    add_string(data, "chipName", info.chip_name);
    add_number(data, "cpuCores", info.cpu_cores);

    add_string(data, "resetReason", info.reset_reason);

    // JSON Ultimate Response
    char* json = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);

    return json;
}

//////////////////////////////////////////////////////////////////////////
// Dynamic Metrics

char * serialize_dynamic_metrics(const SystemMetrics& metrics) {
    using namespace JsonUtils;

    cJSON* root = cJSON_CreateObject();

    if (!root) {
        return nullptr;
    }

    cJSON_AddStringToObject(root, "type", "metrics");

    cJSON* data = cJSON_AddObjectToObject(root, "data");

    // Summary
    add_number(data, "uptime", metrics.uptime_seconds);
    add_number(data, "wifiRssi", *metrics.wifi_rssi);
    add_number(data, "chipTemperature", *metrics.chip_temperature_celsius);

    // Heap Usage
    add_number(data, "totalHeap", metrics.total_heap);
    add_number(data, "freeHeap", metrics.free_heap);
    add_number(data, "usedHeap", metrics.used_heap);
    add_number(data, "minimumFreeHeap", metrics.minimum_free_heap);
    add_number(data, "largestFreeBlock", metrics.largest_free_block);

    // JSON Ultimate Response
    char* json = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);

    return json;
}

//////////////////////////////////////////////////////////////////////////
// Output Log

char * serialize_log(const logging::LogMessage& message) {
    using namespace JsonUtils;

    char level = '?';
    unsigned long uptime = 0;
    char issuer[64] = {};
    int offset = 0;

    const int parsed = sscanf(
        message.text,
        " %c (%lu) %63[^:]: %n",
        &level,
        &uptime,
        issuer,
        &offset
    );

    auto levelName = "unknown";

    switch (level) {
        case 'E': levelName = "error"; break;
        case 'W': levelName = "warn"; break;
        case 'I': levelName = "info"; break;
        case 'D': levelName = "debug"; break;
        case 'V': levelName = "verbose"; break;
        default: break;
    }

    const char* content = message.text;

    if (parsed == 3 && offset > 0) {
        content = message.text + offset;
    } else {
        levelName = "unknown";
        issuer[0] = '\0';
    }

    char cleanMessage[256];
    snprintf(cleanMessage, sizeof(cleanMessage), "%s", content);

    size_t len = std::strlen(cleanMessage);

    while (len > 0 && (cleanMessage[len - 1] == '\n' || cleanMessage[len - 1] == '\r')) {
        cleanMessage[--len] = '\0';
    }

    cJSON* root = cJSON_CreateObject();

    if (!root) {
        return nullptr;
    }

    cJSON_AddStringToObject(root, "type", "logs");

    cJSON* data = cJSON_AddObjectToObject(root, "data");

    add_number(data, "uptime", uptime);
    add_string(data, "level", levelName);
    add_string(data, "issuer", issuer);
    add_string(data, "message", cleanMessage);

    // JSON Ultimate Response
    char* json = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);

    return json;
}

}
