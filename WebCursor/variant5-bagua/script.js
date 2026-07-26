let mouseDown = false;

const cursorEl = document.getElementById("cursor");




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
    if (idleTimer) {
        clearTimeout(idleTimer);
    }

    showCursor();

    idleTimer = setTimeout(hideCursor, IDLE_TIMEOUT_MS);
}

window.setGameMode = function (isGame) {
    gameMode = !!isGame;

    if (gameMode) {
        hideCursor();

        if (idleTimer) {
            clearTimeout(idleTimer);
        }
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
    window.setGameMode(!!document.fullscreenElement);
});


function clickEffect() {
    playClickSound();

    const burst = document.createElement("div");
    burst.className = "bagua-burst";

    const img = document.createElement("img");
    img.src = "assets/bagua.svg";

    burst.appendChild(img);

    document.body.appendChild(burst);

    setTimeout(() => burst.remove(), 700);
}



window.moveCursor = function (x, y, pressed) {


    resetIdleTimer();

    if (gameMode) {
        mouseDown = pressed;
        return;
    }


    if (pressed && !mouseDown) {
        clickEffect();
    }


    if (pressed) {

    }

    mouseDown = pressed;
};

resetIdleTimer();