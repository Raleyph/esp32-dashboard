// Copyright (c) 2026 Raleyph. All rights reserved.

export function getPlateType(value) {
    const success = "success";
    const info = "info";
    const pending = "pending";
    const neutral = "neutral";
    const error = "error";

    if (value === true) return success;
    if (value === false) return neutral;

    if (value === "Online") return success;
    if (value === "Offline") return error;

    const otaStates = {
        "VALID": success,
        "NEW": info,
        "PENDING VERIFY": pending,
        "UNDEFINED": neutral,
        "UNKNOWN": neutral,
        "INVALID": error,
        "ABORTED": error,
    };

    if (value in otaStates)
        return otaStates[value];

    return neutral;
}

export function setTextValue(id, value, setPlate = false) {
    if (value == null || value === "") return;

    let text = value;

    if (value === true) text = "Enabled";
    else if (value === false) text = "Disabled";

    const element = document.getElementById(id);

    if (!element) {
        return;
    }

    element.textContent = text;

    if (setPlate) {
        element.className = `plate ${getPlateType(value)}`;
    }
}
