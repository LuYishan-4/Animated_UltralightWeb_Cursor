// Ported from the KWin BasparkEffect (C++/OpenGL) animation model:
// a fading multi-stroke trail while dragging, an expanding "boom" ring
// with three rotating dash segments on click, and triangular sparks that
// fly outward with velocity/friction/rotation and fade out. Rendered here
// with <canvas> + globalCompositeOperation "lighter" to approximate the
// original's additive GL_SRC_ALPHA/GL_ONE blending.

const canvas = document.getElementById("fx");
const ctx = canvas ? canvas.getContext("2d") : null;
if (!canvas || !ctx) {
    // Don't let this kill the rest of the script (that's exactly what
    // caused "click does nothing at all" bugs before) -- just warn loudly
    // so it's obvious in devtools/console what to check: make sure
    // index.html actually has <canvas id="fx"> (not an old <div id="cursor">
    // left over from a previous variant).
    console.error("[baspark] #fx canvas not found -- check that index.html matches this script.js");
}

const COLOR = { r: 170, g: 90, b: 255 }; // purple, matches the "拔刀" theme
const SCALE = 1;
const MAX_TRAIL = 40;

// ---- click sound (lazy + fully isolated, see previous notes: never
// construct Audio at top-level script scope -- some WebKit builds throw) ----
const CLICK_SOUND_SRC = "assets/click.mp3";
let clickAudioSupported = true;
function playClickSound() {
    if (!clickAudioSupported) return;
    try {
        const s = new Audio(CLICK_SOUND_SRC);
        const p = s.play();
        if (p && typeof p.catch === "function") p.catch(() => {});
    } catch (e) {
        clickAudioSupported = false;
    }
}

// ---- auto-hide / game-mode (see earlier variants for rationale) ----
const IDLE_TIMEOUT_MS = 10000;
let idleTimer = null;
let gameMode = false;

function hideCanvas() {
    if (canvas) canvas.classList.add("hidden");
}
function showCanvas() {
    if (canvas && !gameMode) canvas.classList.remove("hidden");
}
function resetIdleTimer() {
    if (idleTimer) clearTimeout(idleTimer);
    showCanvas();
    idleTimer = setTimeout(hideCanvas, IDLE_TIMEOUT_MS);
}
window.setGameMode = function (isGame) {
    gameMode = !!isGame;
    if (gameMode) {
        hideCanvas();
        if (idleTimer) clearTimeout(idleTimer);
    } else {
        resetIdleTimer();
    }
};
document.addEventListener("visibilitychange", () => {
    if (document.hidden) hideCanvas();
    else if (!gameMode) resetIdleTimer();
});
document.addEventListener("fullscreenchange", () => {
    window.setGameMode(!!document.fullscreenElement);
});

// ---- virtual cursor position ----
// NOTE: the overlay window itself is a small fixed-size view that the host
// app moves to the real mouse position at the OS level, so raw x/y passed
// into moveCursor() are absolute desktop coordinates, not local coordinates
// inside this tiny 128x128 view. To still get a real, evolving (x,y) for
// the trail/spark simulation (like the original desktop-wide effect has),
// we keep a virtual point that starts at the center and gets nudged by the
// real movement's direction + speed, clamped to stay inside the canvas.
let vx = 64, vy = 64;
let lastRawX = null, lastRawY = null;
let lastMoveVX = vx, lastMoveVY = vy; // for distance-based trail spacing, like m_lastMousePos

const MOVE_SCALE = 0.35; // how strongly real movement nudges the virtual point
const PAD = 10;

function nudgeVirtualPosition(rawX, rawY) {
    if (lastRawX !== null) {
        const dx = (rawX - lastRawX) * MOVE_SCALE;
        const dy = (rawY - lastRawY) * MOVE_SCALE;
        vx = Math.max(PAD, Math.min(128 - PAD, vx + dx));
        vy = Math.max(PAD, Math.min(128 - PAD, vy + dy));
    }
    lastRawX = rawX;
    lastRawY = rawY;
}

// ---- state (1:1 with the C++ side) ----
let mouseDown = false;
let trail = [];  // { x, y, life }
let waves = [];  // { x, y, life, maxLife, r, ring: { ang, rs, life, maxLife, segs: [{off,len}] } }
let sparks = []; // { x, y, vx, vy, rot, rs, size, a, f }

function rand() {
    return Math.random();
}

function createBoom(x, y) {
    const ring = {
        ang: rand() * Math.PI * 2,
        rs: 0.08,
        life: 0,
        maxLife: 30,
        segs: [
            { off: -0.25 * Math.PI, len: 1.15 * Math.PI },
            { off: 0.0 * Math.PI, len: 1.15 * Math.PI },
            { off: 0.25 * Math.PI, len: 1.15 * Math.PI },
        ],
    };
    waves.push({ x, y, life: 0, maxLife: 30, r: 0, ring });

    for (let i = 0; i < 8; i++) {
        const a = rand() * Math.PI * 2;
        sparks.push({
            x, y,
            vx: Math.cos(a) * (4.8 + rand() * 2),
            vy: Math.sin(a) * (4.8 + rand() * 2),
            rot: rand() * Math.PI * 2,
            rs: (rand() - 0.5) * 0.28,
            size: (4.0 + rand() * 3.0) * SCALE,
            a: 0.9,
            f: 1.0,
        });
    }
}

function updateAnimation(frames) {
    for (let i = trail.length - 1; i >= 0; i--) {
        trail[i].life -= (mouseDown ? 0.085 : 0.18) * frames;
        if (trail[i].life <= 0) trail.splice(i, 1);
    }

    for (let i = waves.length - 1; i >= 0; i--) {
        const w = waves[i];
        w.life += frames;
        const progress = Math.min(w.life / w.maxLife, 1.0);
        w.r = 26.0 * SCALE * (1.0 - Math.pow(1.0 - progress, 3));

        w.ring.life += frames;
        w.ring.ang -= w.ring.rs * frames;

        if (w.life >= w.maxLife && w.ring.life >= w.ring.maxLife) {
            waves.splice(i, 1);
        }
    }

    for (let i = sparks.length - 1; i >= 0; i--) {
        const s = sparks[i];
        s.x += s.vx * frames;
        s.y += s.vy * frames;
        s.vx *= Math.pow(s.f, frames);
        s.vy *= Math.pow(s.f, frames);
        s.rot += s.rs * frames;
        s.a -= 0.032 * frames;
        if (s.a <= 0) sparks.splice(i, 1);
    }
}

function isActive() {
    return trail.length > 0 || waves.length > 0 || sparks.length > 0;
}

function rgba(alpha) {
    return `rgba(${COLOR.r}, ${COLOR.g}, ${COLOR.b}, ${Math.max(0, alpha)})`;
}

function drawTrail() {
    if (trail.length < 2) return;
    ctx.lineJoin = "round";
    ctx.lineCap = "round";
    for (let i = 0; i < 4; i++) {
        const w = 4.5 + (10.0 - 4.5) * (i / 3.0);
        const alpha = 0.35 * (1.0 - (i / 3.0) * 0.7);
        ctx.strokeStyle = rgba(alpha);
        ctx.lineWidth = w;
        ctx.beginPath();
        trail.forEach((p, idx) => {
            if (idx === 0) ctx.moveTo(p.x, p.y);
            else ctx.lineTo(p.x, p.y);
        });
        ctx.stroke();
    }
}

function drawWave(w) {
    const alpha = Math.max(0, 1.0 - w.life / w.maxLife);
    if (alpha > 0) {
        ctx.beginPath();
        ctx.moveTo(w.x, w.y);
        for (let i = 0; i <= 40; i++) {
            const a = (2.0 * Math.PI * i) / 40.0;
            ctx.lineTo(w.x + w.r * Math.cos(a), w.y + w.r * Math.sin(a));
        }
        ctx.closePath();
        ctx.fillStyle = rgba(alpha * 0.45);
        ctx.fill();
    }

    const rProg = Math.min(w.ring.life / w.ring.maxLife, 1.0);
    for (const seg of w.ring.segs) {
        const len = seg.len * Math.max(0, 1.0 - rProg);
        const start = w.ring.ang + seg.off;
        const r = w.r + 3.0 * SCALE;

        for (let i = 0; i < 3; i++) {
            const alpha = (1.0 - rProg) * (1.0 - (i / 2.0) * 0.5);
            ctx.strokeStyle = `rgba(245, 248, 252, ${Math.max(0, alpha)})`;
            ctx.lineWidth = 3.7 + i * 1.15;
            ctx.beginPath();
            for (let j = 0; j <= 24; j++) {
                const a = start + (len * j) / 24.0;
                const px = w.x + r * Math.cos(a);
                const py = w.y + r * Math.sin(a);
                if (j === 0) ctx.moveTo(px, py);
                else ctx.lineTo(px, py);
            }
            ctx.stroke();
        }
    }
}

function drawSpark(s) {
    const cR = Math.cos(s.rot), sR = Math.sin(s.rot);
    const rot = (px, py) => [s.x + (px * cR - py * sR), s.y + (px * sR + py * cR)];
    const p0 = rot(0, -s.size);
    const p1 = rot(s.size * 0.6, s.size * 0.6);
    const p2 = rot(-s.size * 0.6, s.size * 0.6);

    ctx.fillStyle = `rgba(255, 255, 255, ${Math.max(0, s.a)})`;
    ctx.beginPath();
    ctx.moveTo(p0[0], p0[1]);
    ctx.lineTo(p1[0], p1[1]);
    ctx.lineTo(p2[0], p2[1]);
    ctx.closePath();
    ctx.fill();
}

function render() {
    if (!ctx) return;
    ctx.clearRect(0, 0, 128, 128);
    ctx.globalCompositeOperation = "lighter";

    drawTrail();
    for (const w of waves) drawWave(w);
    for (const s of sparks) drawSpark(s);

    ctx.globalCompositeOperation = "source-over";
}

// ---- main loop ----
let lastFrameTime = null;
function loop(now) {
    requestAnimationFrame(loop);

    if (lastFrameTime === null) lastFrameTime = now;
    const elapsed = now - lastFrameTime;
    lastFrameTime = now;

    const frames = Math.min(elapsed / 16.666, 3.0);
    updateAnimation(frames);
    render();
}
requestAnimationFrame(loop);

// ---- input handling ----
// x, y are the host's real desktop mouse coordinates (see
// UltralightHtmlEffect::move in the C++ side); handleMove nudges a virtual
// local point that trail/spark/wave effects are actually drawn around.
function handleMove(rawX, rawY) {
    resetIdleTimer();
    nudgeVirtualPosition(rawX, rawY);

    if (mouseDown) {
        const d = Math.hypot(vx - lastMoveVX, vy - lastMoveVY);
        if (d > 2.0) {
            trail.push({ x: vx, y: vy, life: 1.0 });
            if (trail.length > MAX_TRAIL) trail.shift();

            if (rand() < 0.3) {
                const a = rand() * Math.PI * 2;
                sparks.push({
                    x: vx + Math.cos(a) * 10.0 * SCALE,
                    y: vy + Math.sin(a) * 10.0 * SCALE,
                    vx: Math.cos(a) * 1.3,
                    vy: Math.sin(a) * 1.3,
                    rot: rand() * Math.PI * 2,
                    rs: 0.16,
                    size: 9.0 * SCALE,
                    a: 0.95,
                    f: 0.7,
                });
            }
            lastMoveVX = vx;
            lastMoveVY = vy;
        }
    }
}

function handlePressStart() {
    playClickSound();
    lastMoveVX = vx;
    lastMoveVY = vy;
    createBoom(vx, vy);
}

window.moveCursor = function (x, y, pressed) {
    handleMove(x, y);

    if (pressed && !mouseDown) {
        handlePressStart();
    }
    mouseDown = pressed;
};

resetIdleTimer();

// ---- native browser fallback (for local testing without the host) ----
document.addEventListener("mousemove", (e) => {
    handleMove(e.clientX, e.clientY);
});
document.addEventListener("mousedown", () => {
    if (!mouseDown) handlePressStart();
    mouseDown = true;
});
document.addEventListener("mouseup", () => {
    mouseDown = false;
});
