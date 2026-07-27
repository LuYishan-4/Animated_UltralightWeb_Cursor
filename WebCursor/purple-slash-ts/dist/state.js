// Shared mutable state, as a single object so other modules can mutate
// its properties directly (reassigning an imported primitive binding
// isn't allowed across ES modules, but mutating a shared object is).
export const state = {
    mouseDown: false,
    lastX: null,
    lastY: null,
    gameMode: false,
    idleTimer: null,
};
export const cursorEl = document.getElementById("cursor");
