import { playClickSound } from "./audio.js";

// ---- click effect: purple sword-draw ("拔刀") slash + spark flash + sound ----

const SLASH_BASE_ANGLE = -40; // degrees, roughly bottom-left to top-right
const SLASH_ANGLE_JITTER = 18; // random +/- variation so it doesn't feel robotic

export function clickEffect(): void {
    playClickSound();

    const angle = SLASH_BASE_ANGLE + (Math.random() - 0.5) * 2 * SLASH_ANGLE_JITTER;

    const slash = document.createElement("div");
    slash.className = "slash";
    slash.style.setProperty("--angle", angle + "deg");
    document.body.appendChild(slash);
    setTimeout(() => slash.remove(), 400);

    const flash = document.createElement("div");
    flash.className = "slash-flash";
    document.body.appendChild(flash);
    setTimeout(() => flash.remove(), 260);
}
