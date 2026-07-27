// Global type augmentation shared by every other .ts file in this project.
// No import/export anywhere in this project on purpose -- these are all
// plain global scripts (like the original single-file version), just split
// into separate files for readability. tsc concatenates them in order
// (see tsconfig.json "files" list) into one script.js.

interface Window {
    moveCursor: (x: number, y: number, pressed: boolean) => void;
    setGameMode: (isGame: boolean) => void;
}
