// Ambient global type augmentation. This file has no imports/exports of
// its own value, just `export {}` to mark it as a module so `declare
// global` is allowed to merge into the real global `Window` type -- tsc
// picks this up automatically since it's included in the program, no need
// to import it anywhere.
export {};
