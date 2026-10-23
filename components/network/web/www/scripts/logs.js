// Copyright (c) 2026 Raleyph. All rights reserved.

import { formatSecondsToDDHHMMSS } from "./formatters.js"

export class LogManager {
    constructor(bufferSize=200, fileUuid="esp32") {
        this.bufferSize = bufferSize;
        this.fileUuid = fileUuid;
        this.stringBuffer = [];

        this.pauseBuffer = [];
        this.isPaused = false;

        this.container = document.getElementById("logs");
        this.statusIndicator = document.getElementById("logs-status");
        this.noLogsLabel = document.getElementById("no-logs-label");

        this.pauseButton = document.getElementById("pause-logs");
        this.downladButton = document.getElementById("download-logs");
        this.clearButton = document.getElementById("clear-logs");

        this.initEvents();
        this.setIndicatorValue();
    }

    initEvents() {
        if (this.pauseButton) {
            this.pauseButton.addEventListener("click", () => {
                this.togglePause();
            });
        }

        if (this.downladButton) {
            this.downladButton.addEventListener("click", () => {
                this.download();
            });
        }

        if (this.clearButton) {
            this.clearButton.addEventListener("click", () => {
                this.clear();
            });
        }
    }

    resolveLogLevelText(level) {
        const levels = {
            "info":  "I",
            "warn":  "W",
            "error": "E"
        };

        return levels[level.toLowerCase()];
    }

    resolveLogLevelColor(level) {
        const levels = {
            "info":  "success",
            "warn":  "pending",
            "error": "error"
        };

        return levels[level.toLowerCase()];
    }

    setIndicatorValue() {
        const indicatorClass = this.isPaused ? "pending" : "success";
        const indicatorText = this.isPaused ? "Paused" : "Live";

        this.statusIndicator.className = `plate ${indicatorClass}`;
        this.statusIndicator.textContent = indicatorText;
    }

    toggleNoLogsLabel(enabled) {
        this.noLogsLabel.style = enabled ? "display: block" : "display: none";
    }

    togglePauseButton() {
        this.pauseButton.textContent = this.isPaused ? "Resume" : "Pause";

        const stylesReplacement = this.isPaused ? ["neutral", "green"] : ["green", "neutral"];
        this.pauseButton.classList.replace(...stylesReplacement);
    }

    addRow(...row) {
        if (this.stringBuffer.length === 0 && !this.isPaused) {
            this.toggleNoLogsLabel(false);
        }

        if (this.isPaused) {
            this.pauseBuffer.push(row);

            if (this.pauseBuffer.length > this.bufferSize) {
                this.pauseBuffer.shift();
            }

            return;
        }

        if (this.stringBuffer.length > this.bufferSize) {
            this.stringBuffer.shift();
            this.container.firstElementChild.remove();
        }

        const logRow = document.createElement("p");
        logRow.classList.add("log-row", `${this.resolveLogLevelColor(row[1])}`);

        row.forEach((value, index) => {
            const logElement = document.createElement("span");

            let text = value;

            switch (index) {
                case 0:
                    text = formatSecondsToDDHHMMSS(value / 1_000);
                    break;

                case 1:
                    text = this.resolveLogLevelText(value);
                    break;
            }

            logElement.textContent = text;

            logRow.appendChild(logElement);
        });

        this.container.appendChild(logRow);

        this.stringBuffer.push(row.join(" "));

        this.container.scrollTop = this.container.scrollHeight;
    }

    togglePause() {
        this.isPaused = !this.isPaused;

        if (!this.isPaused && this.pauseBuffer.length > 0) {
            for (const bufferedRow of this.pauseBuffer) {
                this.addRow(...bufferedRow);
            }

            this.pauseBuffer = [];
        }

        this.setIndicatorValue();
        this.togglePauseButton();
    }

    download() {
        const fileContent = this.stringBuffer.join("\n");
        const blob = new Blob([fileContent], { type: "text/plain;charset=utf-8" });
        const url = URL.createObjectURL(blob);

        const date = new Date();
        const link = document.createElement("a");

        link.href = url;
        link.download = `${this.fileUuid} ${date.toISOString()}.log`;

        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);

        URL.revokeObjectURL(url);
    }

    clear() {
        this.stringBuffer.length = 0;
        this.container.innerHTML = "";
        this.toggleNoLogsLabel(true);
    }
}
