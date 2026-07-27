// Shared state used across the other modules.

let mouseDown = false;
let lastX: number | null = null;
let lastY: number | null = null;

const cursorEl = document.getElementById("cursor") as HTMLDivElement;

let gameMode = false;
let idleTimer: ReturnType<typeof setTimeout> | null = null;
