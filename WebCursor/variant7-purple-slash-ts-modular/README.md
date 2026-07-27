# 紫色拔刀 cursor effect (TypeScript, 多檔案模組化版)

這版把邏輯拆成多個 `.ts` 檔案(不是單一一大包),各自負責一塊:

| 檔案 | 內容 |
|---|---|
| `types.ts` | `window.moveCursor` / `window.setGameMode` 的型別宣告 |
| `state.ts` | 共用的狀態變數(`mouseDown`、`lastX/lastY`、`cursorEl`...) |
| `idle.ts` | 閒置 10 秒自動隱藏 + `setGameMode` 遊戲畫面隱藏邏輯 |
| `audio.ts` | 點擊音效(延遲建立 + try/catch 保護,見下方說明) |
| `trail.ts` | 移動軌跡效果 + 共用的 `handleMove` |
| `slash.ts` | 紫色拔刀點擊特效(刀光 + 閃光) |
| `main.ts` | 進入點,接上 `window.moveCursor` 跟瀏覽器原生滑鼠事件備援 |

全部都是**純全域腳本**,沒有用 `import`/`export`,所以 `tsconfig.json`
用 `outFile` + `files`(照上面表格的順序)把它們編譯串接成單一一份
`script.js`,`index.html` 實際載入的就是這個。

## 重新編譯

改完任何一個 `.ts` 檔案後:

```bash
npm install -g typescript   # 如果還沒裝過 tsc
tsc -p tsconfig.json
```

這會照 `tsconfig.json` 裡 `files` 的順序重新串接、覆蓋 `script.js`。
如果你新增了檔案,記得也要把檔名加進 `tsconfig.json` 的 `files` 陣列裡
(順序要對:被依賴的檔案要排在前面)。

## 音效

把你的音效檔放進 `assets/click.mp3`(或改 `audio.ts` 裡的
`CLICK_SOUND_SRC`),然後重新編譯一次即可。沒放檔案之前特效照常運作,
只是沒聲音,不會出錯。

## 這版包含的修復

- 點擊/移動效果永遠畫在小視窗的固定中心點(64,64),因為視窗本身已經被
  外部程式移到滑鼠位置了,x/y 只拿來算移動方向。
- 音效物件延後到點擊當下才建立,並包在 try/catch 裡,避免所在環境不支援
  Audio 時整支 script 中斷執行。
- 加了原生瀏覽器 mousemove/mousedown/mouseup 事件當備援,方便直接在瀏覽器
  測試。
