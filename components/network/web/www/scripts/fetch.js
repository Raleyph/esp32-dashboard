// Copyright (c) 2026 Raleyph. All rights reserved.

import * as update from "./update.js";

import { MemoryChart } from "./chart.js";
import { LogManager } from "./logs.js";

let socket;
let reconnectTimer;

const domChart = document.getElementById("heap-chart");

const heapChart = new MemoryChart(domChart);
const logManager = new LogManager();

async function fetchStaticMetrics() {
    try {
        const response = await fetch("/api/system", {
            cache: "no-store"
        });

        if (!response.ok) {
            return;
        }

        const json = await response.json();

        update.updateStaticMetrics(json.data);
    } catch (error) {
        console.error("Failed to fetch system info:", error);
    }
}

function connectWebSocket() {
    function bytesToKib(bytes) {
        return bytes / 1024;
    }

    if (socket) {
        socket.onclose = null;
        socket.close();
    }

    const protocol = location.protocol === "https:" ? "wss:" : "ws:";

    socket = new WebSocket(`${protocol}//${location.host}/ws`);

    socket.onopen = () => {
        console.log("Successfully connected to ESP32.");

        update.setConnectionStatus(true);

        clearTimeout(reconnectTimer);
    };

    socket.onmessage = (event) => {
        const message = JSON.parse(event.data);
        const data = message.data;

        switch (message.type) {
            case "metrics":
                update.updateDynamicMetrics(data);

                const totalHeapKiB = bytesToKib(data.totalHeap)

                if (heapChart.maxY !== totalHeapKiB) {
                    heapChart.resizeYAxis(totalHeapKiB);
                }

                heapChart.addDataPoint(
                    bytesToKib(data.freeHeap),
                    bytesToKib(data.minimumFreeHeap),
                    bytesToKib(data.largestFreeBlock)
                );

                break;

            case "logs":
                logManager.addRow(data.uptime, data.level, data.issuer, data.message);
                break;
        }
    };

    socket.onclose = () => {
        console.error("Connection has been lost. Try to reconnect in 2 seconds...")

        update.setConnectionStatus(false);

        clearTimeout(reconnectTimer);

        reconnectTimer = setTimeout(connectWebSocket, 2000);
    };

    socket.onerror = (error) => {
        console.error("WebSocket error:", error)

        update.setConnectionStatus(false);

        socket.close();
    }
}

fetchStaticMetrics().then();
connectWebSocket();
