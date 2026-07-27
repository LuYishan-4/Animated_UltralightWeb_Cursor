// ---- click sound effect ----
// put your own sound file at assets/click.mp3 (any filename/format works,
// just update CLICK_SOUND_SRC below to match). The Audio object is created
// fresh inside playClickSound() (never at top-level script scope) and
// fully wrapped in try/catch: some lightweight WebKit builds (e.g.
// Ultralight without audio/codec support compiled in) throw when Audio()
// is constructed or played. If that happened at top-level scope it would
// silently abort the ENTIRE script before window.moveCursor got defined.
// This keeps any audio failure fully contained.

const CLICK_SOUND_SRC = "assets/click.mp3";
let clickAudioSupported = true;

function playClickSound(): void {
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
