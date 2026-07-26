let mouseDown = false;
let hideTimeout = null; 

// 設定滑鼠超過 2000 毫秒（2 秒）沒動就觸發消失機制
const HIDE_DELAY = 2000; 
const cursorEl = document.getElementById("cursor");

/**
 * 點擊時的擴散圓圈特效
 * @param {number} x - 目前游標的 X 座標
 * @param {number} y - 目前游標的 Y 座標
 */
function clickEffect(x, y) {
    const ring = document.createElement("div");
    ring.className = "ring";
    ring.style.left = x + "px";
    ring.style.top = y + "px";
    document.body.appendChild(ring);

    setTimeout(() => ring.remove(), 600);
}

/**
 * 點擊與按住時的粒子碎屑特效
 * @param {number} x - 目前游標的 X 座標
 * @param {number} y - 目前游標的 Y 座標
 */
function particle(x, y) {
    const p = document.createElement("div");
    p.className = "particle";
    
    // 計算隨機噴發的方向與距離
    let targetX = (Math.random() - 0.5) * 80;
    let targetY = (Math.random() - 0.5) * 80;

    // 將粒子精準定位在游標當前位置
    p.style.left = x + "px";
    p.style.top = y + "px";

    p.style.setProperty("--x", targetX + "px");
    p.style.setProperty("--y", targetY + "px");

    document.body.appendChild(p);

    setTimeout(() => p.remove(), 600);
}


window.moveCursor = function(x, y, pressed) {
    

    if (x < 0 || y < 0) {
        cursorEl.style.display = "none";
        clearTimeout(hideTimeout);
        return;
    }
    

    cursorEl.style.display = "block";
    document.body.style.cursor = "auto";

    cursorEl.style.left = x + "px";
    cursorEl.style.top = y + "px";

    clearTimeout(hideTimeout);
    hideTimeout = setTimeout(() => {
        cursorEl.style.display = "none"; 
        document.body.style.cursor = "none"; 
    }, HIDE_DELAY);

    if (pressed && !mouseDown) {
        clickEffect(x, y);
        for (let i = 0; i < 8; i++) {
            particle(x, y);
        }
    }

    mouseDown = pressed;

    if (pressed) {
        particle(x, y);
    }
}
