# purple-slash-cursor-effect

一般常見的 TypeScript 專案結構,`src/` 放原始碼、`tsc` 編譯到 `dist/`,
每個 `.ts` 對應編出一個 `.js`(標準 ES module 寫法,彼此用 `import`/`export`),
沒有附 index.html / style.css,你自己接到你的環境裡就好。

```
src/
  types.ts   -- window.moveCursor / window.setGameMode 的型別宣告
  state.ts   -- 共用狀態 (state 物件、cursorEl)
  idle.ts    -- 閒置 10 秒自動隱藏 + setGameMode 遊戲畫面隱藏
  audio.ts   -- 點擊音效 (延遲建立 + try/catch 保護)
  trail.ts   -- 移動軌跡效果 + 共用的 handleMove
  slash.ts   -- 紫色拔刀點擊特效 (刀光 + 閃光)
  main.ts    -- 進入點,接上 window.moveCursor 跟瀏覽器原生滑鼠事件備援
dist/        -- tsc 編譯出來的東西,每個檔案一對一
```

## 安裝 / 編譯

```bash
npm install
npm run build     # 編譯一次
npm run watch      # 邊改邊自動重新編譯
```

## 怎麼載入

`dist/main.js` 是進入點,是標準 ES module,用 `<script type="module">` 載入即可:

```html
<script type="module" src="dist/main.js"></script>
```

頁面裡需要有 `<div id="cursor"></div>`,以及對應的 CSS(`.hidden` /
`.trail` / `.slash` / `.slash-flash` 這幾個 class,可以照之前給你的
style.css 搬過來,或自己刻)。

## 音效

需要一個 `assets/click.mp3`(或改 `src/audio.ts` 裡的 `CLICK_SOUND_SRC`
指到你要的路徑/檔名),改完記得 `npm run build` 重新編譯。沒放檔案之前
特效照常運作,只是沒聲音,不會出錯。
