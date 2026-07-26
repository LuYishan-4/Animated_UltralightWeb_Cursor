let mouseDown = false;
let lastX = 64;
let lastY = 64;

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

function spawnTrail(x, y) {
    const t = document.createElement("div");
    t.className = "trail";
    t.style.left = x + "px";
    t.style.top = y + "px";
    document.body.appendChild(t);
    setTimeout(() => t.remove(), 500);
}

// ---- click effects: ring burst + scattering ciallo.svg icons ----

function clickEffect(x, y) {
    const ring = document.createElement("div");
    ring.className = "ring";
    ring.style.left = x + "px";
    ring.style.top = y + "px";
    document.body.appendChild(ring);
    setTimeout(() => ring.remove(), 600);
}

function iconParticle(x, y) {
    const p = document.createElement("img");
    p.src = "assets/ciallo.svg";
    p.className = "icon-particle";

    // scatter direction/distance and rotation are randomized per icon
    let dx = (Math.random() - 0.5) * 100;
    let dy = (Math.random() - 0.5) * 100;
    let rot = (Math.random() - 0.5) * 360;

    p.style.left = x + "px";
    p.style.top = y + "px";
    p.style.setProperty("--x", dx + "px");
    p.style.setProperty("--y", dy + "px");
    p.style.setProperty("--r", rot + "deg");

    document.body.appendChild(p);
    setTimeout(() => p.remove(), 700);
}

// x, y are pixel coordinates within the 128x128 canvas.
// pressed indicates whether the mouse button is currently held down.
window.moveCursor = function (x, y, pressed) {
    resetIdleTimer();

    // move the cursor dot to the real position
    cursorEl.style.left = x + "px";
    cursorEl.style.top = y + "px";

    // leave a trail while moving
    if (x !== lastX || y !== lastY) {
        spawnTrail(x, y);
    }
    lastX = x;
    lastY = y;

    // click just started -> burst of ring + many scattered icons
    if (pressed && !mouseDown) {
        clickEffect(x, y);
        for (let i = 0; i < 10; i++) {
            iconParticle(x, y);
        }
    }

    mouseDown = pressed;

    if (pressed) {
        iconParticle(x, y);
    }
};

resetIdleTimer();
