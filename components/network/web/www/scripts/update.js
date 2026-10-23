// Copyright (c) 2026 Raleyph. All rights reserved.

import * as dom from "./dom.js";
import * as formatters from "./formatters.js";

const uptimeDom = document.getElementById("uptime");
const systemStatusDom = document.getElementById("system-status");

const wifiRssiDom = document.getElementById("signal-dbm");
const wifiRssiStatusDom = document.getElementById("rssi-status");

const chipTemperatureDom = document.getElementById("chip-temperature");
const chipTemperatureStatusDom = document.getElementById("chip-temperature-status");

const ramPercentageDom = document.getElementById("ram-percentage");
const ramBarValue = document.getElementById("ram-bar-value");
const ramCapacity = document.getElementById("ram-capacity");

const ramFreeHeapDom = document.getElementById("ram-free-heap");
const ramMinFreeHeapDom = document.getElementById("ram-min-free-heap");
const ramLargestFreeBlockDom = document.getElementById("ram-largest-free-block");

const chipTemperatureRanges = [
    { max: 65,  text: "Good",     style: "success" },
    { max: 80,  text: "Elevated", style: "pending" },
    { max: 100, text: "High",     style: "error" }
];

const wifiRssiRanges = [
    { max: 50, text: "Excellent", style: "success" },
    { max: 60, text: "Good",      style: "success" },
    { max: 70, text: "Fair",      style: "info" },
    { max: 80, text: "Weak",      style: "pending" },
    { max: 90, text: "Poor",      style: "error" },
];

function updatePartitionBars(data) {
    const partitions = data.partitions;

    // OTA0

    dom.setTextValue("ota0-label", partitions.ota_0.label);
    dom.setTextValue("ota0-state", partitions.ota_0.state, true);
    dom.setTextValue("ota0-firmware-version", partitions.ota_0.firmwareVersion);

    document.getElementById("ota0-percentage").textContent = formatters.getPercentageString(
        partitions.ota_0.firmwareSize,
        partitions.ota_0.size,
        "partition capacity"
    );

    document.getElementById("ota0-bar-value").style.width = `${formatters.getPercentage(
        partitions.ota_0.firmwareSize,
        partitions.ota_0.size
    )}%`;

    document.getElementById("ota0-capacity").textContent = formatters.getBarCapacity(
        formatters.bytesToMibStr(partitions.ota_0.firmwareSize),
        formatters.bytesToMibStr(partitions.ota_0.size),
        "MiB"
    );

    // OTA1

    dom.setTextValue("ota1-label", partitions.ota_1.label);
    dom.setTextValue("ota1-state", partitions.ota_1.state, true);
    dom.setTextValue("ota1-firmware-version", partitions.ota_1.firmwareVersion);

    document.getElementById("ota1-percentage").textContent = formatters.getPercentageString(
        partitions.ota_1.firmwareSize,
        partitions.ota_1.size,
        "partition capacity"
    );

    document.getElementById("ota1-bar-value").style.width = `${formatters.getPercentage(
        partitions.ota_1.firmwareSize,
        partitions.ota_1.size
    )}%`;

    document.getElementById("ota1-capacity").textContent = formatters.getBarCapacity(
        formatters.bytesToMibStr(partitions.ota_1.firmwareSize),
        formatters.bytesToMibStr(partitions.ota_1.size),
        "MiB"
    );
}

function updateStatus(element, text, style) {
    element.textContent = text;
    element.className = `subtitle-small ${style}`;
}

function getSystemStatus(uptime) {
    if (uptime === null) {
        return { text: "No data", style: "neutral" };
    }

    return { text: "System active", style: "success" };
}

function getStatus(value, ranges, defaultCritical = { text: "Critical", style: "error" }) {
    if (value === null || value === undefined || Number.isNaN(value)) {
        return { text: "No data", style: "neutral" };
    }

    const found = ranges.find(range => value <= range.max);

    return found ? { text: found.text, style: found.style } : defaultCritical;
}

export function setConnectionStatus(status) {
    dom.setTextValue("connection-status", status ? "Online" : "Offline", true);
}

export function updateStaticMetrics(data) {
    // Header
    dom.setTextValue("firmware-version", data.firmwareVersion);
    dom.setTextValue("device-ip", data.ip);

    // Partitions
    updatePartitionBars(data);

    // Flash
    document.getElementById("running-partition").textContent = data.runningPartition;
    document.getElementById("boot-partition").textContent = data.bootPartition;
    document.getElementById("next-ota-partition").textContent = data.nextOtaPartition;
    document.getElementById("total-flash-size").textContent = `${formatters.bytesToMbStr(data.totalFlashSize)} MB`;
    dom.setTextValue("rollback-enabled", data.isRollbackEnabled, true);

    // Security
    dom.setTextValue("flash-encryption", data.isFlashEncrypted, true);
    dom.setTextValue("secure-boot", data.isSecureBootEnabled, true);
    dom.setTextValue("jtag-disabled", !data.isJtagDisabled, true);
    dom.setTextValue("nvs-encryption", data.isNvsEncrypted, true);

    // Hardware
    document.getElementById("chip-name").textContent = data.chipName.toLocaleUpperCase();
    document.getElementById("cpu-cores").textContent = data.cpuCores;
    document.getElementById("idf-version").textContent = data.idfVersion;
    document.getElementById("mac-address").textContent = data.mac;
    document.getElementById("reset-reason").textContent = data.resetReason;
}

export function updateDynamicMetrics(data) {
    uptimeDom.textContent = formatters.formatSecondsToDDHHMMSS(data.uptime);
    const systemStatus = getSystemStatus(data.uptime);
    updateStatus(systemStatusDom, systemStatus.text, systemStatus.style);

    wifiRssiDom.textContent = `${data.wifiRssi} dBm`;
    const wifiRssiStatus = getStatus(data.wifiRssi * -1, wifiRssiRanges)
    updateStatus(wifiRssiStatusDom, wifiRssiStatus.text, wifiRssiStatus.style);

    chipTemperatureDom.textContent = data.chipTemperature ? `${data.chipTemperature} °C` : "-";
    const chipTemperatureStatus = getStatus(data.chipTemperature, chipTemperatureRanges);
    updateStatus(chipTemperatureStatusDom, chipTemperatureStatus.text, chipTemperatureStatus.style);

    ramPercentageDom.textContent = formatters.getPercentageString(
        data.usedHeap,
        data.totalHeap,
        "available heap"
    );

    ramBarValue.style.width = `${formatters.getPercentage(
        data.usedHeap,
        data.totalHeap
    )}%`;

    ramCapacity.textContent = formatters.getBarCapacity(
        formatters.bytesToKibStr(data.usedHeap),
        formatters.bytesToKibStr(data.totalHeap),
        "KiB"
    );

    ramFreeHeapDom.textContent = `${formatters.bytesToKibStr(data.freeHeap)} KiB`;
    ramMinFreeHeapDom.textContent = `${formatters.bytesToKibStr(data.minimumFreeHeap)} KiB`;
    ramLargestFreeBlockDom.textContent = `${formatters.bytesToKibStr(data.largestFreeBlock)} KiB`;
}
