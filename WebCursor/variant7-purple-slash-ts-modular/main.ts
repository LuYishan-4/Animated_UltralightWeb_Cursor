// ---- entry point ----
// x, y are the host's real desktop mouse coordinates (see
// UltralightHtmlEffect::move in the C++ side) -- used here only to derive
// movement direction, never as an absolute position on this page.
// pressed indicates whether the mouse button is currently held down.
window.moveCursor = function (x: number, y: number, pressed: boolean): void {
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
document.addEventListener("mousemove", (e: MouseEvent) => {
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
