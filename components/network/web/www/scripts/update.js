import * as dom from "./dom.js";
import * as formatters from "./formatters.js";

function updateHeader(data) {
    dom.setText("firmware-version", data.firmwareVersion);
    dom.setText("device-ip", data.ip);
}

function updatePartitionBars(data) {
    const partitions = data.partitions;

    // OTA0

    dom.setText("ota0-label", partitions.ota_0.label);
    dom.setText("ota0-state", partitions.ota_0.state, true);
    dom.setText("ota0-firmware-version", partitions.ota_0.firmwareVersion);

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

    dom.setText("ota1-label", partitions.ota_1.label);
    dom.setText("ota1-state", partitions.ota_1.state, true);
    dom.setText("ota1-firmware-version", partitions.ota_1.firmwareVersion);

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

function updateFlashInfo(data) {
    document.getElementById("running-partition").textContent = data.runningPartition;
    document.getElementById("boot-partition").textContent = data.bootPartition;
    document.getElementById("next-ota-partition").textContent = data.nextOtaPartition;

    dom.setText("rollback-enabled", data.isRollbackEnabled, true);

    document.getElementById("total-flash-size").textContent = `${formatters.bytesToMbStr(data.totalFlashSize)} MB`;
}

function updateSecurityInfo(data) {
    dom.setText("flash-encryption", data.isFlashEncrypted, true);
    dom.setText("secure-boot", data.isSecureBootEnabled, true);
    dom.setText("jtag-disabled", !data.isJtagDisabled, true);
    dom.setText("nvs-encryption", data.isNvsEncrypted, true);
}

function updateDeviceInfo(data) {
    document.getElementById("chip-name").textContent = data.chipName.toLocaleUpperCase();
    document.getElementById("cpu-cores").textContent = data.cpuCores;

    document.getElementById("idf-version").textContent = data.idfVersion;
    document.getElementById("mac-address").textContent = data.mac;

    document.getElementById("reset-reason").textContent = data.resetReason;
}

function updateStatusBarStyle(element, style) {
    element.className = `subtitle-small ${style}`;
}

function updateSystemStatus(uptime) {
    const element = document.getElementById("system-status");

    if (!uptime) {
        element.textContent = "No data";
        updateStatusBarStyle(element, "neutral");
        return;
    }

    element.textContent = "System active";
    updateStatusBarStyle(element, "success");
}

function updateNetworkStatus(rssi) {
    const element = document.getElementById("rssi-status");

    if (!rssi) {
        element.textContent = "No data";
        updateStatusBarStyle(element, "neutral");
        return;
    }

    let text;

    if (rssi >= -50) {
        text = "Excellent";
        updateStatusBarStyle(element, "success");
    } else if (rssi >= -60) {
        text = "Good";
        updateStatusBarStyle(element, "success");
    } else if (rssi >= -70) {
        text = "Fair";
        updateStatusBarStyle(element, "info");
    } else if (rssi >= -80) {
        text = "Weak";
        updateStatusBarStyle(element, "pending");
    } else if (rssi >= -90) {
        text = "Poor";
        updateStatusBarStyle(element, "error");
    } else {
        text = "Critical";
        updateStatusBarStyle(element, "error");
    }

    element.textContent = text;
}

function updateTemperatureStatus(temperature) {
    const element = document.getElementById("temperature-status");

    if (!temperature) {
        element.textContent = "No data";
        updateStatusBarStyle(element, "neutral");
        return;
    }

    let text;

    if (temperature <= 65) {
        text = "Good";
        updateStatusBarStyle(element, "success");
    } else if (temperature > 65 && temperature <= 80) {
        text = "Elevated";
        updateStatusBarStyle(element, "pending");
    } else if (temperature > 85 && temperature < 100) {
        text = "High";
        updateStatusBarStyle(element, "error");
    } else {
        text = "Critical";
        updateStatusBarStyle(element, "error");
    }

    element.textContent = text;
}

export function setConnectionStatus(status) {
    dom.setText("connection-status", status ? "Online" : "Offline", true);
}

export function updateStaticMetrics(data) {
    updateHeader(data);
    updatePartitionBars(data);
    updateFlashInfo(data);
    updateSecurityInfo(data);
    updateDeviceInfo(data);
}

export function updateDynamicMetrics(data) {
    document.getElementById("uptime").textContent = formatters.formatSecondsToDDHHMMSS(data.uptime);
    updateSystemStatus(data.uptime);

    document.getElementById("signal-dbm").textContent = `${data.wifiRssi} dBm`;
    updateNetworkStatus(data.wifiRssi);

    document.getElementById("chip-temperature").textContent = data.chipTemperature ? `${data.chipTemperature} °C` : "-";
    updateTemperatureStatus(data.chipTemperature);

    document.getElementById("ram-percentage").textContent = formatters.getPercentageString(
        data.usedHeap,
        data.totalHeap,
        "available heap"
    );

    document.getElementById("ram-bar-value").style.width = `${formatters.getPercentage(
        data.usedHeap,
        data.totalHeap
    )}%`;

    document.getElementById("ram-capacity").textContent = formatters.getBarCapacity(
        formatters.bytesToKibStr(data.usedHeap),
        formatters.bytesToKibStr(data.totalHeap),
        "KiB"
    );

    document.getElementById("ram-free-heap").textContent = `${formatters.bytesToKibStr(data.freeHeap)} KiB`;
    document.getElementById("ram-min-free-heap").textContent = `${formatters.bytesToKibStr(data.minimumFreeHeap)} KiB`;
    document.getElementById("ram-largest-free-block").textContent = `${formatters.bytesToKibStr(data.largestFreeBlock)} KiB`;
}
