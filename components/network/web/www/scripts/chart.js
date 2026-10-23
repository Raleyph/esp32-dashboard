// Copyright (c) 2026 Raleyph. All rights reserved.

export class MemoryChart {
    constructor(canvas, options = {}) {
        if (!(canvas instanceof HTMLCanvasElement)) {
            throw new Error("Valid canvas element is required");
        }

        this.canvas = canvas;
        this.ctx = canvas.getContext("2d");

        if (!this.ctx) {
            throw new Error("Cannot initialize 2D context");
        }

        this.duration = options.duration ?? 60_000;
        this.maxPoints = options.maxPoints ?? 60;
        this.minY = options.minY ?? 0;
        this.maxY = options.maxY ?? 512;
        this.unit = options.unit ?? "KiB";

        this.lineColors = options.lineColors ?? [
            "#0d9488",
            "#64748b",
            "#d97706"
        ];

        this.colors = {
            grid: "#333333",
            text: "#888888"
        };

        this.padding = {
            top: 20,
            right: 10,
            bottom: 50,
            left: 50
        };

        this.data = [];

        this.canvasWidth = 0;
        this.canvasHeight = 0;

        this.resizeObserver = new ResizeObserver(() => {
            this.resize();
        });

        this.resizeObserver.observe(this.canvas.parentElement);

        this.timer = setInterval(() => {
            this.draw();
        }, 1000);

        this.resize();
    }

    resizeYAxis(maxValue) {
        if (!Number.isFinite(maxValue) || maxValue <= this.minY) {
            return;
        }

        this.maxY = maxValue;
        this.draw();
    }

    resize() {
        const rect = this.canvas.getBoundingClientRect();

        if (rect.width <= 0 || rect.height <= 0) {
            return;
        }

        const dpr = window.devicePixelRatio || 1;

        this.canvasWidth = rect.width;
        this.canvasHeight = rect.height;

        const pixelWidth = Math.round(rect.width * dpr);
        const pixelHeight = Math.round(rect.height * dpr);

        if (this.canvas.width !== pixelWidth ||
            this.canvas.height !== pixelHeight) {
            this.canvas.width = pixelWidth;
            this.canvas.height = pixelHeight;
        }

        this.ctx.setTransform(dpr, 0, 0, dpr, 0, 0);

        this.draw();
    }

    addDataPoint(...values) {
        const point = {
            time: Date.now(),
            values: values.map(value => Number.isFinite(value) ? value : null)
        };

        this.data.push(point);

        this.pruneData(point.time);
        this.draw();
    }

    pruneData(now) {
        const cutoff = now - this.duration;

        while (this.data.length > 0 && this.data[0].time < cutoff) {
            this.data.shift();
        }

        while (this.data.length > this.maxPoints) {
            this.data.shift();
        }
    }

    getXPixel(timestamp, now) {
        const plotWidth = this.canvasWidth - this.padding.left - this.padding.right;
        const startTime = now - this.duration;
        return this.padding.left + ((timestamp - startTime) / this.duration) * plotWidth;
    }

    getYPixel(value) {
        const plotHeight = this.canvasHeight - this.padding.top - this.padding.bottom;
        return this.padding.top + (1 - (value - this.minY) / (this.maxY - this.minY)) * plotHeight;
    }

    formatTimestamp(timestamp) {
        const date = new Date(timestamp);

        return date.toLocaleTimeString("en-GB", {
            hour: "2-digit",
            minute: "2-digit",
            second: "2-digit",
            hour12: false
        });
    }

    drawGrid(now) {
        const ctx = this.ctx;

        const plotWidth = this.canvasWidth - this.padding.left - this.padding.right;
        const plotHeight = this.canvasHeight - this.padding.top - this.padding.bottom;

        ctx.strokeStyle = this.colors.grid;
        ctx.fillStyle = this.colors.text;
        ctx.font = "11px sans-serif";
        ctx.lineWidth = 1;

        // Y-axis
        const ySteps = 4;

        for (let i = 0; i <= ySteps; i++) {
            const fraction = i / ySteps;

            const value = this.maxY - (this.maxY - this.minY) * fraction;
            const y = this.padding.top + plotHeight * fraction;

            ctx.beginPath();
            ctx.moveTo(this.padding.left, y);
            ctx.lineTo(this.padding.left + plotWidth, y);
            ctx.stroke();

            ctx.textAlign = "right";
            ctx.textBaseline = "middle";

            ctx.fillText(
                `${Math.round(value).toString()} ${this.unit}`,
                this.padding.left - 10,
                y
            );
        }

        // X-axis
        const xSteps = this.canvasWidth < 480 ? 12 : 8;
        const startTime = now - this.duration;

        for (let i = 0; i <= xSteps; i++) {
            const fraction = i / xSteps;
            const x = this.padding.left + plotWidth * fraction;

            ctx.beginPath();
            ctx.moveTo(x, this.padding.top);
            ctx.lineTo(x, this.padding.top + plotHeight);
            ctx.stroke();

            if (i % 6 !== 0) {
                continue;
            }

            ctx.fillStyle = this.colors.text;
            ctx.textBaseline = "top";

            ctx.textAlign =
                i === 0 ? "left" :
                    i === xSteps ? "right" : "center";

            const timestamp = startTime + this.duration * fraction;

            ctx.fillText(
                this.formatTimestamp(timestamp),
                x,
                this.padding.top + plotHeight + 12
            );
        }
    }

    drawLines(now) {
        const ctx = this.ctx;

        const plotWidth = this.canvasWidth - this.padding.left - this.padding.right;
        const plotHeight = this.canvasHeight - this.padding.top - this.padding.bottom;

        ctx.save();

        ctx.beginPath();
        ctx.rect(
            this.padding.left,
            this.padding.top,
            plotWidth,
            plotHeight
        );

        ctx.clip();

        for (let lineIndex = 0; lineIndex < this.lineColors.length; lineIndex++) {
            ctx.beginPath();
            ctx.strokeStyle = this.lineColors[lineIndex];
            ctx.lineWidth = 2;
            ctx.lineJoin = "round";
            ctx.lineCap = "round";

            let started = false;
            let previousTime = null;
            let lastPoint = null;

            for (const entry of this.data) {
                const value = entry.values[lineIndex];

                if (!Number.isFinite(value)) {
                    started = false;
                    previousTime = null;
                    continue;
                }

                const x = this.getXPixel(entry.time, now);
                const y = this.getYPixel(value);

                if (previousTime !== null && entry.time - previousTime > 5000) {
                    started = false;
                }

                if (!started) {
                    ctx.moveTo(x, y);
                    started = true;
                } else {
                    ctx.lineTo(x, y);
                }

                previousTime = entry.time;
                lastPoint = { x, y };
            }

            ctx.stroke();

            if (
                lastPoint &&
                lastPoint.x >= this.padding.left &&
                lastPoint.x <= this.padding.left + plotWidth
            ) {

                ctx.beginPath();
                ctx.fillStyle = this.lineColors[lineIndex];

                ctx.arc(
                    lastPoint.x,
                    lastPoint.y,
                    3,
                    0,
                    Math.PI * 2
                );

                ctx.fill();
            }
        }

        ctx.restore();
    }

    draw() {
        if (this.canvasWidth <= 0 || this.canvasHeight <= 0) {
            return;
        }

        const now = Date.now();

        this.pruneData(now);

        this.ctx.clearRect(
            0,
            0,
            this.canvasWidth,
            this.canvasHeight
        );

        this.drawGrid(now);

        if (this.data.length > 0) {
            this.drawLines(now);
        }
    }

    clear() {
        this.data = [];
        this.draw();
    }

    destroy() {
        clearInterval(this.timer);
        this.resizeObserver.disconnect();
    }
}
