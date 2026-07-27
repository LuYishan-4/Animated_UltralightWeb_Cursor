// ---- click sound effect ----
// put your own sound file at assets/click.mp3 (any filename/format works,
// just update CLICK_SOUND_SRC below to match). The Audio object is created
// fresh inside playClickSound() (never at module top-level) and fully
// wrapped in try/catch: some lightweight WebKit builds (e.g. Ultralight
// without audio/codec support compiled in) throw when Audio() is
// constructed or played. Keeping it lazy + try/catch means an audio
// failure never breaks anything else on the page.

const CLICK_SOUND_SRC = "assets/click.mp3";
let clickAudioSupported = true;

export function playClickSound(): void {
    if (!clickAudioSupported) return;
    try {
        const s = new Audio(CLICK_SOUND_SRC);
        const p = s.play();
        if (p && typeof p.catch === "function") {
            p.catch(() => {
                // no sound file yet, or autoplay blocked -- effects still run fine
            });
        }
    } catch (e) {
        // this environment doesn't support Audio playback -- stop trying,
        // but never let it affect anything else
        clickAudioSupported = false;
    }
}
