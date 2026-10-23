// Copyright (c) 2026 Raleyph. All rights reserved.

document.addEventListener('DOMContentLoaded', () => {
    const tabsHeader = document.getElementById('tabsHeader');

    tabsHeader.addEventListener('click', (e) => {
        const clickedBtn = e.target.closest('.tab-button');

        if (!clickedBtn || clickedBtn.classList.contains('active'))
            return;

        const targetTabId = clickedBtn.dataset.tab;

        tabsHeader.querySelectorAll('.tab-button').forEach(
            btn => btn.classList.remove('active')
        );

        clickedBtn.classList.add('active');

        document.querySelectorAll('.tab-pane').forEach(
            tab => tab.classList.remove('active')
        );

        document.getElementById(targetTabId)?.classList.add('active');
    });
});
