// Copyright (c) 2026 Raleyph. All rights reserved.

#pragma once

#include <cstdint>
#include <optional>
#include <array>
#include <string>

namespace diagnostics {

struct SystemMetrics {

    // Summary
    std::uint64_t uptime_seconds = 0;
    std::optional<std::int32_t> wifi_rssi = std::nullopt;
    std::optional<float> chip_temperature_celsius = std::nullopt;

    // RAM
    std::uint32_t total_heap = 0;
    std::uint32_t free_heap = 0;
    std::uint32_t used_heap = 0;
    std::uint32_t minimum_free_heap = 0;
    std::uint32_t largest_free_block = 0;
};

struct PartitionInfo {
    const char* label = nullptr;
    const char* ota_state = nullptr;
    std::string firmware_version;

    std::uint32_t size = 0;
    std::optional<std::uint32_t> firmware_size = std::nullopt;
};

struct SystemInfo {

    // Flash & Partitions
    PartitionInfo ota_0;
    PartitionInfo ota_1;

    const char* running_partition = nullptr;
    const char* boot_partition = nullptr;
    const char* next_ota_partition = nullptr;

    bool is_rollback_enabled = false;

    std::uint32_t total_flash_size = 0;

    // Security
    bool is_flash_encrypted = false;
    bool is_secure_boot_enabled = false;
    bool is_jtag_disabled = false;
    bool is_nvs_encrypted = false;

    // Device Information
    std::array<char, 16> ip{};
    std::array<char, 18> mac_address{};

    std::string firmware_version;
    const char* idf_version = nullptr;

    const char* chip_name = nullptr;
    std::uint32_t cpu_cores = 0;

    const char* reset_reason = nullptr;
};

}
