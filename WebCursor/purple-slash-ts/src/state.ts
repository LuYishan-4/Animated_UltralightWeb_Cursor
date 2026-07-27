// Shared mutable state, as a single object so other modules can mutate
// its properties directly (reassigning an imported primitive binding
// isn't allowed across ES modules, but mutating a shared object is).

export const state = {
    mouseDown: false,
    lastX: null as number | null,
    lastY: null as number | null,
    gameMode: false,
    idleTimer: null as ReturnType<typeof setTimeout> | null,
};

export const cursorEl = document.getElementById("cursor") as HTMLDivElement;
