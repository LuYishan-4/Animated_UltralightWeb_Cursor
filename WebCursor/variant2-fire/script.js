let mouseDown = false;

const cursorEl = document.getElementById("cursor");

// ---- auto-hide logic ----
// 1) hides after 10s with no movement
// 2) hides when "game screen" mode is signalled (see setGameMode below)

const IDLE_TIMEOUT_MS = 10000;
let idleTimer = null;
let gameMode = false;

function hideCursor() {
    cursorEl.classList.add("hidden");
}

function showCursor() {
    if (!gameMode) {
        cursorEl.classList.remove("hidden");
    }
}

function resetIdleTimer() {
    if (idleTimer) clearTimeout(idleTimer);
    showCursor();
    idleTimer = setTimeout(hideCursor, IDLE_TIMEOUT_MS);
}

// call this from outside when a game screen / fullscreen content starts or ends
// e.g. window.setGameMode(true) to hide, window.setGameMode(false) to allow showing again
window.setGameMode = function (isGame) {
    gameMode = !!isGame;
    if (gameMode) {
        hideCursor();
        if (idleTimer) clearTimeout(idleTimer);
    } else {
        resetIdleTimer();
    }
};

// also react to page visibility (tab hidden) and native fullscreen changes
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

// ---- click / particle effects ----

function clickEffect() {
    const ring = document.createElement("div");
    ring.className = "ring";
    document.body.appendChild(ring);

    setTimeout(() => ring.remove(), 600);
}

function particle() {
    const p = document.createElement("div");
    p.className = "particle";

    let x = (Math.random() - 0.5) * 80;
    let y = (Math.random() - 0.5) * 80;

    p.style.left = "64px";
    p.style.top = "64px";

    p.style.setProperty("--x", x + "px");
    p.style.setProperty("--y", y + "px");

    document.body.appendChild(p);

    setTimeout(() => p.remove(), 600);
}

window.moveCursor = function (x, y, pressed) {
    // any movement/press counts as activity -> reset idle timer & show cursor
    resetIdleTimer();

    if (pressed && !mouseDown) {
        clickEffect();
        for (let i = 0; i < 8; i++) {
            particle();
        }
    }

    mouseDown = pressed;

    if (pressed) {
        particle();
    }
};

// start the idle timer immediately
resetIdleTimer();
