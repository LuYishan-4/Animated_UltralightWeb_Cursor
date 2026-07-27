"use strict";
// Global type augmentation shared by every other .ts file in this project.
// No import/export anywhere in this project on purpose -- these are all
// plain global scripts (like the original single-file version), just split
// into separate files for readability. tsc concatenates them in order
// (see tsconfig.json "files" list) into one script.js.
// Shared state used across the other modules.
let mouseDown = false;
let lastX = null;
let lastY = null;
const cursorEl = document.getElementById("cursor");
let gameMode = false;
let idleTimer = null;
// ---- auto-hide logic ----
// 1) hides after 10s with no movement
// 2) hides when "game screen" mode is signalled (see setGameMode below)
const IDLE_TIMEOUT_MS = 10000;
function hideCursor() {
    cursorEl.classList.add("hidden");
}
function showCursor() {
    if (!gameMode) {
        cursorEl.classList.remove("hidden");
    }
}
function resetIdleTimer() {
    if (idleTimer)
        clearTimeout(idleTimer);
    showCursor();
    idleTimer = setTimeout(hideCursor, IDLE_TIMEOUT_MS);
}
// call from outside when a game screen / fullscreen content starts or ends
window.setGameMode = function (isGame) {
    gameMode = !!isGame;
    if (gameMode) {
        hideCursor();
        if (idleTimer)
            clearTimeout(idleTimer);
    }
    else {
        resetIdleTimer();
    }
};
document.addEventListener("visibilitychange", () => {
    if (document.hidden) {
        hideCursor();
    }
    else if (!gameMode) {
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
// ---- click sound effect ----
// put your own sound file at assets/click.mp3 (any filename/format works,
// just update CLICK_SOUND_SRC below to match). The Audio object is created
// fresh inside playClickSound() (never at top-level script scope) and
// fully wrapped in try/catch: some lightweight WebKit builds (e.g.
// Ultralight without audio/codec support compiled in) throw when Audio()
// is constructed or played. If that happened at top-level scope it would
// silently abort the ENTIRE script before window.moveCursor got defined.
// This keeps any audio failure fully contained.
const CLICK_SOUND_SRC = "assets/click.mp3";
let clickAudioSupported = true;
function playClickSound() {
    if (!clickAudioSupported)
        return;
    try {
        const s = new Audio(CLICK_SOUND_SRC);
        const p = s.play();
        if (p && typeof p.catch === "function") {
            p.catch(() => {
                // no sound file yet, or autoplay blocked -- effects still run fine
            });
        }
    }
    catch (e) {
        // this environment doesn't support Audio playback -- stop trying,
        // but never let it affect anything else
        clickAudioSupported = false;
    }
}
// ---- movement trail effect ----
// NOTE: the overlay window itself is a small fixed-size view (e.g. 128x128)
// that the host app moves to the real mouse position at the OS level.
// So x/y passed into moveCursor() are absolute desktop coordinates, NOT
// local coordinates inside this tiny view. Everything here is drawn
// relative to the fixed center point (64,64). We only use x/y to figure
// out the *direction* the mouse is moving, for the trail.
const CENTER_X = 64;
const CENTER_Y = 64;
const TRAIL_MOVE_THRESHOLD = 1; // ignore sub-pixel jitter
const TRAIL_MAX_OFFSET = 26; // how far the trail streak reaches from center
function spawnTrail(offsetX, offsetY) {
    const t = document.createElement("div");
    t.className = "trail";
    t.style.left = CENTER_X + "px";
    t.style.top = CENTER_Y + "px";
    t.style.setProperty("--x", offsetX + "px");
    t.style.setProperty("--y", offsetY + "px");
    document.body.appendChild(t);
    setTimeout(() => t.remove(), 500);
}
// core movement handler, shared by both the external API (window.moveCursor)
// and the native browser fallback in main.ts
function handleMove(x, y) {
    resetIdleTimer();
    if (lastX !== null && lastY !== null) {
        const dx = x - lastX;
        const dy = y - lastY;
        const mag = Math.sqrt(dx * dx + dy * dy);
        if (mag > TRAIL_MOVE_THRESHOLD) {
            const nx = (dx / mag) * TRAIL_MAX_OFFSET;
            const ny = (dy / mag) * TRAIL_MAX_OFFSET;
            spawnTrail(nx, ny);
        }
    }
    lastX = x;
    lastY = y;
}
// ---- click effect: purple sword-draw ("拔刀") slash + spark flash + sound ----
const SLASH_BASE_ANGLE = -40; // degrees, roughly bottom-left to top-right
const SLASH_ANGLE_JITTER = 18; // random +/- variation so it doesn't feel robotic
function clickEffect() {
    playClickSound();
    const angle = SLASH_BASE_ANGLE + (Math.random() - 0.5) * 2 * SLASH_ANGLE_JITTER;
    const slash = document.createElement("div");
    slash.className = "slash";
    slash.style.setProperty("--angle", angle + "deg");
    document.body.appendChild(slash);
    setTimeout(() => slash.remove(), 400);
    const flash = document.createElement("div");
    flash.className = "slash-flash";
    document.body.appendChild(flash);
    setTimeout(() => flash.remove(), 260);
}
// ---- entry point ----
// x, y are the host's real desktop mouse coordinates (see
// UltralightHtmlEffect::move in the C++ side) -- used here only to derive
// movement direction, never as an absolute position on this page.
// pressed indicates whether the mouse button is currently held down.
window.moveCursor = function (x, y, pressed) {
    handleMove(x, y);
    if (pressed && !mouseDown) {
        clickEffect();
    }
    mouseDown = pressed;
};
resetIdleTimer();
// ---- native browser fallback ----
// lets you test this page directly in a normal browser tab (double-click
// the html file) without needing the Ultralight host to call
// window.moveCursor at all. Harmless to leave in for the real deployment
// too -- it just won't receive real events there.
document.addEventListener("mousemove", (e) => {
    handleMove(e.clientX, e.clientY);
});
document.addEventListener("mousedown", () => {
    if (!mouseDown) {
        clickEffect();
    }
    mouseDown = true;
});
document.addEventListener("mouseup", () => {
    mouseDown = false;
});
