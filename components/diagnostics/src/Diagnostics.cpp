// Copyright (c) 2026 Raleyph. All rights reserved.

#include "diagnostics/Diagnostics.hpp"

#include "esp_err.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_heap_caps.h"
#include "esp_image_format.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_flash.h"
#include "esp_flash_encrypt.h"
#include "esp_chip_info.h"
#include "esp_mac.h"
#include "esp_secure_boot.h"
#include "esp_efuse.h"
#include "esp_efuse_table.h"
#include "sdkconfig.h"

namespace diagnostics {

namespace {

const char* get_ota_state(const esp_partition_t* partition)
{
    if (!partition) {
        return "NOT_FOUND";
    }

    esp_ota_img_states_t state{};

    const esp_err_t err = esp_ota_get_state_partition(partition, &state);

    if (err == ESP_ERR_NOT_FOUND) {
        return "UNKNOWN";
    }

    if (err != ESP_OK) {
        return "ERROR";
    }

    switch (state) {
        case ESP_OTA_IMG_NEW:            return "NEW";
        case ESP_OTA_IMG_PENDING_VERIFY: return "PENDING VERIFY";
        case ESP_OTA_IMG_VALID:          return "VALID";
        case ESP_OTA_IMG_INVALID:        return "INVALID";
        case ESP_OTA_IMG_ABORTED:        return "ABORTED";
        case ESP_OTA_IMG_UNDEFINED:      return "UNDEFINED";
        default:                         return "UNKNOWN";
    }
}

std::optional<std::uint32_t> get_firmware_size(const esp_partition_t* partition)
{
    if (!partition) {
        return std::nullopt;
    }

    esp_partition_pos_t position{};

    position.offset = partition->address;
    position.size = partition->size;

    esp_image_metadata_t metadata{};

    if (esp_image_verify(ESP_IMAGE_VERIFY_SILENT, &position, &metadata) != ESP_OK) {
        return std::nullopt;
    }

    return metadata.image_len;
}

void read_partition_info(const esp_partition_t* partition, PartitionInfo &partition_info) {
    if (partition == nullptr) {
        ESP_LOGW("MetricsService", "OTA partition not found");
        return;
    }

    partition_info.label = partition->label;

    esp_app_desc_t desc{};

    if (esp_ota_get_partition_description(partition, &desc) == ESP_OK) {
        partition_info.firmware_version = "v" + std::string(desc.version);
    }

    partition_info.ota_state = get_ota_state(partition);
    partition_info.size = partition->size;
    partition_info.firmware_size = get_firmware_size(partition);
}

bool is_rollback_enabled()
{
#ifdef CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE
    return true;
#else
    return false;
#endif
}

bool is_jtag_disabled()
{
#if CONFIG_IDF_TARGET_ESP32
    return esp_efuse_read_field_bit(ESP_EFUSE_JTAG_DISABLE);
#elif CONFIG_IDF_TARGET_ESP32C6
    const bool pad_disabled = esp_efuse_read_field_bit(ESP_EFUSE_DIS_PAD_JTAG);
    const bool usb_disabled = esp_efuse_read_field_bit(ESP_EFUSE_DIS_USB_JTAG);
    return pad_disabled && usb_disabled;
#else
    return false;
#endif
}

bool is_nvs_encrypted() {
#ifdef CONFIG_NVS_ENCRYPTION
    return true;
#else
    return false;
#endif
}

const char* get_reset_reason()
{
    switch (esp_reset_reason()) {
        case ESP_RST_UNKNOWN:    return "UNKNOWN";
        case ESP_RST_POWERON:    return "POWERON";
        case ESP_RST_EXT:        return "EXTERNAL";
        case ESP_RST_SW:         return "SOFTWARE";
        case ESP_RST_PANIC:      return "PANIC";
        case ESP_RST_INT_WDT:    return "INT_WDT";
        case ESP_RST_TASK_WDT:   return "TASK_WDT";
        case ESP_RST_WDT:        return "WDT";
        case ESP_RST_DEEPSLEEP:  return "DEEPSLEEP";
        case ESP_RST_BROWNOUT:   return "BROWNOUT";
        case ESP_RST_SDIO:       return "SDIO";
        case ESP_RST_USB:        return "USB";
        case ESP_RST_JTAG:       return "JTAG";
        case ESP_RST_EFUSE:      return "EFUSE";
        case ESP_RST_PWR_GLITCH: return "POWER_GLITCH";
        case ESP_RST_CPU_LOCKUP: return "CPU_LOCKUP";
        default:                 return "UNKNOWN";
    }
}

}

SystemMetrics get_dynamic_info() {
    SystemMetrics metrics{};

    // Summary
    metrics.uptime_seconds = esp_timer_get_time() / 1'000'000;

    wifi_ap_record_t ap{};

    if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) {
        metrics.wifi_rssi = ap.rssi;
    }

    // RAM
    const uint32_t total_heap = heap_caps_get_total_size(MALLOC_CAP_8BIT);
    const uint32_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);

    metrics.total_heap = total_heap;
    metrics.free_heap = free_heap;
    metrics.used_heap = total_heap - free_heap;
    metrics.minimum_free_heap = esp_get_minimum_free_heap_size();
    metrics.largest_free_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    return metrics;
}

SystemInfo get_system_info() {
    SystemInfo info{};

    // OTA Partitions
    const esp_partition_t* ota0_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP,
        ESP_PARTITION_SUBTYPE_APP_OTA_0,
        nullptr
    );

    const esp_partition_t* ota1_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP,
        ESP_PARTITION_SUBTYPE_APP_OTA_1,
        nullptr
    );

    PartitionInfo ota_0{};
    read_partition_info(ota0_partition, ota_0);
    info.ota_0 = ota_0;

    PartitionInfo ota_1{};
    read_partition_info(ota1_partition, ota_1);
    info.ota_1 = ota_1;

    // Partition Roles
    const esp_partition_t* running = esp_ota_get_running_partition();
    const esp_partition_t* boot = esp_ota_get_boot_partition();
    const esp_partition_t* next = esp_ota_get_next_update_partition(nullptr);

    info.running_partition = running->label;
    info.boot_partition = boot->label;
    info.next_ota_partition = next->label;

    info.is_rollback_enabled = is_rollback_enabled();

    // Flash Info
    std::uint32_t flash_size = 0;
    ESP_ERROR_CHECK(esp_flash_get_physical_size(nullptr, &flash_size));
    info.total_flash_size = flash_size;

    // Security
    info.is_flash_encrypted = esp_flash_encryption_enabled();
    info.is_secure_boot_enabled = esp_secure_boot_enabled();
    info.is_jtag_disabled = is_jtag_disabled();
    info.is_nvs_encrypted = is_nvs_encrypted();

    // IP Address
    esp_netif_ip_info_t ip_info{};
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        esp_ip4addr_ntoa(&ip_info.ip, info.ip.data(), info.ip.size());
    }

    // MAC Address
    std::uint8_t mac[6]{};

    if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK) {
        snprintf(
            info.mac_address.data(),
            info.mac_address.size(),
            "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2],
            mac[3], mac[4], mac[5]
        );
    }

    // Device Info
    info.firmware_version = "v" + std::string(esp_app_get_description()->version);
    info.idf_version = esp_get_idf_version();

    esp_chip_info_t chip{};
    esp_chip_info(&chip);

    info.cpu_cores = chip.cores;
    info.chip_name = CONFIG_IDF_TARGET;

    info.reset_reason = get_reset_reason();

    return info;
}

}
