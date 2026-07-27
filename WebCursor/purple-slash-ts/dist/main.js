import "./types.js";
import { state } from "./state.js";
import { handleMove } from "./trail.js";
import { clickEffect } from "./slash.js";
import { resetIdleTimer } from "./idle.js";
// ---- entry point ----
// x, y are the host's real desktop mouse coordinates (see
// UltralightHtmlEffect::move in the C++ side) -- used here only to derive
// movement direction, never as an absolute position on this page.
// pressed indicates whether the mouse button is currently held down.
window.moveCursor = function (x, y, pressed) {
    handleMove(x, y);
    if (pressed && !state.mouseDown) {
        clickEffect();
    }
    state.mouseDown = pressed;
};
resetIdleTimer();
// ---- native browser fallback ----
// lets you test this page directly in a normal browser tab without needing
// a host to call window.moveCursor at all. Harmless to leave in for the
// real deployment too -- it just won't receive real events there.
document.addEventListener("mousemove", (e) => {
    handleMove(e.clientX, e.clientY);
});
document.addEventListener("mousedown", () => {
    if (!state.mouseDown) {
        clickEffect();
    }
    state.mouseDown = true;
});
document.addEventListener("mouseup", () => {
    state.mouseDown = false;
});
