export function formatSecondsToDDHHMMSS(totalSeconds) {
    const hours = Math.floor((totalSeconds % (3600 * 24)) / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = Math.floor(totalSeconds % 60);

    const h = String(hours).padStart(2, "0");
    const m = String(minutes).padStart(2, "0");
    const s = String(seconds).padStart(2, "0");

    return `${h}:${m}:${s}`;
}

export function bytesToKibStr(bytes) {
    return (bytes / 1024).toFixed(1);
}

export function bytesToMibStr(bytes) {
    return (bytes / 1024 / 1024).toFixed(2);
}

export function bytesToMbStr(bytes) {
    return (bytes / 1024 / 1024).toFixed(0);
}

export function getBarCapacity(occupied, total, unit) {
    return `${occupied} / ${total} ${unit}`;
}

export function getPercentage(value, maxValue) {
    return (value / maxValue * 100).toFixed(0);
}

export function getPercentageString(value, maxValue, subject) {
    if (value == null || maxValue == null) {
        return "No available data";
    }

    return `${getPercentage(value, maxValue)}% of ${subject}`;
}
