// ---- auto-hide logic ----
// 1) hides after 10s with no movement
// 2) hides when "game screen" mode is signalled (see setGameMode below)

const IDLE_TIMEOUT_MS = 10000;

function hideCursor(): void {
    cursorEl.classList.add("hidden");
}

function showCursor(): void {
    if (!gameMode) {
        cursorEl.classList.remove("hidden");
    }
}

function resetIdleTimer(): void {
    if (idleTimer) clearTimeout(idleTimer);
    showCursor();
    idleTimer = setTimeout(hideCursor, IDLE_TIMEOUT_MS);
}

// call from outside when a game screen / fullscreen content starts or ends
window.setGameMode = function (isGame: boolean): void {
    gameMode = !!isGame;
    if (gameMode) {
        hideCursor();
        if (idleTimer) clearTimeout(idleTimer);
    } else {
        resetIdleTimer();
    }
};

document.addEventListener("visibilitychange", () => {
    if (document.hidden) {
        hideCursor();
    } else if (!gameMode) {
        resetIdleTimer();
    }
});

document.addEventListener("fullscreenchange", () => {
    if (document.fullscreenElement) {
        window.setGameMode(true);
    } else {
        window.setGameMode(false);
    }
});
