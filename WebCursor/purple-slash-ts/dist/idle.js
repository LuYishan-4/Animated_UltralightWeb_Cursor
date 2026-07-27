import { state, cursorEl } from "./state.js";
// ---- auto-hide logic ----
// 1) hides after 10s with no movement
// 2) hides when "game screen" mode is signalled (see setGameMode below)
const IDLE_TIMEOUT_MS = 10000;
export function hideCursor() {
    cursorEl.classList.add("hidden");
}
export function showCursor() {
    if (!state.gameMode) {
        cursorEl.classList.remove("hidden");
    }
}
export function resetIdleTimer() {
    if (state.idleTimer)
        clearTimeout(state.idleTimer);
    showCursor();
    state.idleTimer = setTimeout(hideCursor, IDLE_TIMEOUT_MS);
}
// call from outside when a game screen / fullscreen content starts or ends
window.setGameMode = function (isGame) {
    state.gameMode = !!isGame;
    if (state.gameMode) {
        hideCursor();
        if (state.idleTimer)
            clearTimeout(state.idleTimer);
    }
    else {
        resetIdleTimer();
    }
};
document.addEventListener("visibilitychange", () => {
    if (document.hidden) {
        hideCursor();
    }
    else if (!state.gameMode) {
        resetIdleTimer();
    }
});
document.addEventListener("fullscreenchange", () => {
    if (document.fullscreenElement) {
        window.setGameMode(true);
    }
    else {
        window.setGameMode(false);
    }
});
