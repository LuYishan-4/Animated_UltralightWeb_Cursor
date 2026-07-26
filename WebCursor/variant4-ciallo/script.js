let mouseDown = false;
let lastX = null;
let lastY = null;

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

// call from outside when a game screen / fullscreen content starts or ends
window.setGameMode = function (isGame) {
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

// ---- movement trail effect ----
// NOTE: the overlay window itself is a small fixed-size view (e.g. 128x128)
// that the host app moves to the real mouse position at the OS level.
// So x/y passed into moveCursor() are absolute desktop coordinates, NOT
// local coordinates inside this tiny view -- they should never be used to
// place anything on this page directly (that's what caused the
// stuck-in-a-corner bugs before). Everything here is drawn relative to the
// fixed center point (64,64), exactly like variant 1-3. We only use the
// x/y to figure out the *direction* the mouse is moving, for the trail.

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

// ---- click effects: ring burst + scattering ciallo.svg icons ----
// always drawn from the fixed center point, same principle as variant 1-3

function clickEffect() {
    const ring = document.createElement("div");
    ring.className = "ring";
    document.body.appendChild(ring);
    setTimeout(() => ring.remove(), 600);
}

function iconParticle() {
    const p = document.createElement("img");
    p.src = "assets/ciallo.svg";
    p.className = "icon-particle";

    // scatter direction/distance and rotation are randomized per icon
    let dx = (Math.random() - 0.5) * 100;
    let dy = (Math.random() - 0.5) * 100;
    let rot = (Math.random() - 0.5) * 360;

    p.style.setProperty("--x", dx + "px");
    p.style.setProperty("--y", dy + "px");
    p.style.setProperty("--r", rot + "deg");

    document.body.appendChild(p);
    setTimeout(() => p.remove(), 700);
}

// x, y are the host's real desktop mouse coordinates (see
// UltralightHtmlEffect::move in the C++ side) -- used here only to derive
// movement direction, never as an absolute position on this page.
// pressed indicates whether the mouse button is currently held down.
window.moveCursor = function (x, y, pressed) {
    resetIdleTimer();

    if (typeof x === "number" && typeof y === "number" && lastX !== null) {
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

    // click just started -> burst of ring + many scattered icons
    if (pressed && !mouseDown) {
        clickEffect();
        for (let i = 0; i < 10; i++) {
            iconParticle();
        }
    }

    mouseDown = pressed;

    if (pressed) {
        iconParticle();
    }
};

resetIdleTimer();
