// Copyright (c) 2026 Raleyph. All rights reserved.

document.addEventListener("DOMContentLoaded", () => {
    const tabsHeader = document.getElementById("tabsHeader");

    if (!tabsHeader) {
        return;
    }

    const buttons = tabsHeader.querySelectorAll(".tab-button");
    const panes = document.querySelectorAll(".tab-pane");

    function activateTab(targetTabId, updateHash = true) {
        const targetBtn = tabsHeader.querySelector(`[data-tab="${targetTabId}"]`);
        const targetPane = document.getElementById(targetTabId);

        if (!targetBtn || !targetPane) {
            return;
        }

        buttons.forEach(btn => btn.classList.remove("active"));
        targetBtn.classList.add("active");

        panes.forEach(pane => pane.classList.remove("active"));
        targetPane.classList.add("active");

        if (updateHash) {
            history.pushState(null, null, `#${targetTabId}`)
        }
    }

    tabsHeader.addEventListener("click", (e) => {
        const clickedBtn = e.target.closest(".tab-button");

        if (!clickedBtn || clickedBtn.classList.contains("active"))
            return;

        activateTab(clickedBtn.dataset.tab);
    });

    const initialHash = window.location.hash.slice(1);

    if (initialHash) {
        activateTab(initialHash, false);
    } else {
        const defaultActive = tabsHeader.querySelector(".tab-button.active");

        if (defaultActive) {
            activateTab(defaultActive.dataset.tab, false);
        }
    }

    window.addEventListener("popstate", () => {
        const hash = window.location.hash.slice(1);

        if (hash) {
            activateTab(hash, false);
        }
    });
});
