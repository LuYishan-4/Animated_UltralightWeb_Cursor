// Gun HUD cursor effect.
//
// IMPORTANT ARCHITECTURE NOTE: earlier variants in this project ran inside
// a small 128x128 Ultralight view that the host app repositioned to follow
// the real mouse at the OS level -- in that setup x/y passed into
// moveCursor() were absolute desktop coordinates that did NOT line up with
// this page's own local coordinates. The edge-avoidance rotation requested
// here ("don't let gun.gif touch the 1920x1080 desktop boundary") only
// makes sense if this page instead covers the FULL 1920x1080 desktop, so
// that x/y map 1:1 onto it. Make sure the Ultralight view is created/resized
// to 1920x1080 for this variant (see UltralightHtmlEffect::resize in your
// C++), otherwise the math below won't line up with reality.

const DESKTOP_W = 1920;
const DESKTOP_H = 1080;

// ~96 CSS px per inch is the standard browser assumption, so 1cm ~= 37.795px
const PX_PER_CM = 37.795;
const GUN_RADIUS_CM = 4;
const GUN_RADIUS_PX = Math.round(PX_PER_CM * GUN_RADIUS_CM); // ~151px

// gun.gif is 768x384 natively; scaled down to a reasonable HUD size
const GUN_NATIVE_W = 768;
const GUN_NATIVE_H = 384;
const GUN_DISPLAY_SCALE = 0.32;
const GUN_W = GUN_NATIVE_W * GUN_DISPLAY_SCALE;
const GUN_H = GUN_NATIVE_H * GUN_DISPLAY_SCALE;

// measured from the actual gun.gif: 10 frames x 100ms = 1000ms per loop
const GUN_FIRE_DURATION_MS = 1000;

const MAX_AMMO = 7;
const RELOAD_MS = 2000;
const AMMO_ICON_SIZE = 26; // displayed size (ammo.png is 512x512 natively)
const AMMO_OFFSET_X = 46; // stack anchor, relative to crosshair
const AMMO_OFFSET_Y = 46;
const AMMO_SPACING = 22; // vertical gap between stacked rounds

const SHOT_SOUND_SRC = "assets/shot.mp3";
const RELOAD_SOUND_SRC = "assets/reload.mp3";

const hud = document.getElementById("hud");
const crosshairEl = document.getElementById("crosshair");
const crosshairRingEl = document.getElementById("crosshair-ring");
const gunPivot = document.getElementById("gunPivot");
const gunImg = document.getElementById("gunImg");

// position the gun image within its pivot: anchored (hand/grip side) at
// radius GUN_RADIUS_PX to the right of the pivot (angle 0 = gun's natural
// drawn orientation, pointing right), vertically centered
gunImg.style.left = GUN_RADIUS_PX + "px";
gunImg.style.top = -GUN_H / 2 + "px";
gunImg.style.width = GUN_W + "px";
gunImg.style.height = GUN_H + "px";

// ---- state ----
let mouseDown = false;
let currentX = DESKTOP_W / 2;
let currentY = DESKTOP_H / 2;
let ammoCount = MAX_AMMO;
let reloading = false;
let fireRevertTimer = null;
const ammoIcons = [];

// pre-create the ammo icon elements once, toggle visibility as ammoCount changes
for (let i = 0; i < MAX_AMMO; i++) {
    const img = document.createElement("img");
    img.className = "ammo-icon";
    img.src = "assets/ammo.png";
    img.style.width = AMMO_ICON_SIZE + "px";
    img.style.height = AMMO_ICON_SIZE + "px";
    hud.appendChild(img);
    ammoIcons.push(img);
}

// ---- click / gunshot sound (lazy + fully isolated -- see earlier notes:
// never construct Audio at top-level script scope, some WebKit builds
// throw and that silently kills the whole script before window.moveCursor
// gets defined) ----
let audioSupported = true;
function playSound(src) {
    if (!audioSupported) return;
    try {
        const s = new Audio(src);
        const p = s.play();
        if (p && typeof p.catch === "function") p.catch(() => {});
    } catch (e) {
        audioSupported = false;
    }
}

// ---- auto-hide / game-mode ----
const IDLE_TIMEOUT_MS = 10000;
let idleTimer = null;
let gameMode = false;

function hideHud() {
    hud.classList.add("hidden");
}
function showHud() {
    if (!gameMode) hud.classList.remove("hidden");
}
function resetIdleTimer() {
    if (idleTimer) clearTimeout(idleTimer);
    showHud();
    idleTimer = setTimeout(hideHud, IDLE_TIMEOUT_MS);
}
window.setGameMode = function (isGame) {
    gameMode = !!isGame;
    if (gameMode) {
        hideHud();
        if (idleTimer) clearTimeout(idleTimer);
    } else {
        resetIdleTimer();
    }
};
document.addEventListener("visibilitychange", () => {
    if (document.hidden) hideHud();
    else if (!gameMode) resetIdleTimer();
});
document.addEventListener("fullscreenchange", () => {
    window.setGameMode(!!document.fullscreenElement);
});

// ---- edge-avoidance rotation ----
// pushes the gun's aim angle away from whichever desktop edge(s) the
// cursor is currently close to, so the (radius + gun length) reach never
// crosses the 1920x1080 boundary. Combines contributions from all four
// edges so corners blend smoothly (e.g. top-left corner -> gun points
// down-right). Falls back to angle 0 (pointing right) away from any edge.
const REACH = GUN_RADIUS_PX + GUN_W;

function computeSafeAngleDeg(x, y) {
    let vx = 0, vy = 0;

    if (x < REACH) vx += (REACH - x) / REACH;
    if (x > DESKTOP_W - REACH) vx -= (x - (DESKTOP_W - REACH)) / REACH;
    if (y < REACH) vy += (REACH - y) / REACH;
    if (y > DESKTOP_H - REACH) vy -= (y - (DESKTOP_H - REACH)) / REACH;

    if (vx === 0 && vy === 0) return 0;
    return (Math.atan2(vy, vx) * 180) / Math.PI;
}

// ---- HUD positioning ----
function updateHudPosition(x, y) {
    crosshairEl.style.transform = `translate(${x}px, ${y}px)`;
    crosshairRingEl.style.transform = `translate(${x}px, ${y}px)`;

    const angleDeg = computeSafeAngleDeg(x, y);
    gunPivot.style.transform = `translate(${x}px, ${y}px) rotate(${angleDeg}deg)`;

    renderAmmo(x, y);
}

function renderAmmo(x, y) {
    for (let i = 0; i < MAX_AMMO; i++) {
        const icon = ammoIcons[i];
        if (i < ammoCount) {
            icon.style.display = "block";
            icon.style.left = x + AMMO_OFFSET_X + "px";
            icon.style.top = y + AMMO_OFFSET_Y - i * AMMO_SPACING + "px";
        } else {
            icon.style.display = "none";
        }
    }
}

// ---- firing ----
function playGunAnimation() {
    if (fireRevertTimer) clearTimeout(fireRevertTimer);
    // cache-bust so the gif always restarts from frame 1 even if clicked
    // again before the previous animation finished
    gunImg.src = "assets/gun.gif?t=" + Date.now();
    fireRevertTimer = setTimeout(() => {
        gunImg.src = "assets/gun_frame1.png";
    }, GUN_FIRE_DURATION_MS);
}

function fire() {
    if (reloading || ammoCount <= 0) return;

    ammoCount -= 1;
    playSound(SHOT_SOUND_SRC);
    playGunAnimation();
    renderAmmo(currentX, currentY);

    if (ammoCount === 0) {
        reloading = true;
        playSound(RELOAD_SOUND_SRC);
        setTimeout(() => {
            ammoCount = MAX_AMMO;
            reloading = false;
            renderAmmo(currentX, currentY);
        }, RELOAD_MS);
    }
}

// ---- input handling ----
// x, y are real desktop pixel coordinates (see UltralightHtmlEffect::move
// in the C++ side) -- this page must cover the full 1920x1080 desktop for
// them to line up 1:1 with positions on this page (see note at top).
function handleMove(x, y) {
    resetIdleTimer();
    currentX = x;
    currentY = y;
    updateHudPosition(x, y);
}

window.moveCursor = function (x, y, pressed) {
    handleMove(x, y);

    if (pressed && !mouseDown) {
        fire();
    }

    mouseDown = pressed;
};

updateHudPosition(currentX, currentY);
resetIdleTimer();

// ---- native browser fallback (for local testing without the host) ----
document.addEventListener("mousemove", (e) => {
    handleMove(e.clientX, e.clientY);
});
document.addEventListener("mousedown", () => {
    if (!mouseDown) fire();
    mouseDown = true;
});
document.addEventListener("mouseup", () => {
    mouseDown = false;
});
