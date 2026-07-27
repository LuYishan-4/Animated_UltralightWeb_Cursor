import { state } from "./state.js";
import { resetIdleTimer } from "./idle.js";

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

export function spawnTrail(offsetX: number, offsetY: number): void {
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
export function handleMove(x: number, y: number): void {
    resetIdleTimer();

    if (state.lastX !== null && state.lastY !== null) {
        const dx = x - state.lastX;
        const dy = y - state.lastY;
        const mag = Math.sqrt(dx * dx + dy * dy);

        if (mag > TRAIL_MOVE_THRESHOLD) {
            const nx = (dx / mag) * TRAIL_MAX_OFFSET;
            const ny = (dy / mag) * TRAIL_MAX_OFFSET;
            spawnTrail(nx, ny);
        }
    }
    state.lastX = x;
    state.lastY = y;
}
