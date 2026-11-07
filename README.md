# scotch-egg shield module

scotch-egg (Seeed XIAO nRF52840、4x12 のオーソ配列、LED なし) の ZMK のシールドと、
キー配列 [windmill](https://github.com/cognitom/windmill) を ZMK で動かすモジュール。

windmill の QMK 実装 (`firmware/windmill.c`) を ZMK の behavior へ移したもので、
かな入力、最下段のタップ・ホールド、こ + み の半角スペース、接続先ごとの OS と配列の設定などは
windmill と同じに動く。

## キー配列

配置は windmill の technik のキーマップ (`firmware/technik/keymaps/default/keymap.c`) と同じ。
図は windmill の README を見てください。

- [キー配列](https://github.com/cognitom/windmill#キー配列) (英字入力時、かな入力時、記号とファンクションキー)
  - [英字入力時の図](https://github.com/cognitom/windmill/blob/main/docs/images/layout-main.png)
  - [かな入力時の図](https://github.com/cognitom/windmill/blob/main/docs/images/layout-kana.png)
  - [設定レイヤーの図](https://github.com/cognitom/windmill/blob/main/docs/images/layout-conf.png)
- [対象 OS の切り替え](https://github.com/cognitom/windmill#対象osの切り替え)

windmill と違うところ (LED とダークモードが無い、設定レイヤーで USB と BLE を切り替える、
設定の保存が 60 秒遅れる、など) は [docs/check.md の差分](docs/check.md#windmill-の-readme-との差分)
にまとめてある。キーマップの本体は [`config/scotch_egg.keymap`](config/scotch_egg.keymap)。

起動したときは英数から始まる。左下の Ctrl をタップするたびに英数とかなが入れ替わる。

## 書き込み方

ファームウェアは GitHub Actions の Build ワークフローが作る。

1. リポジトリの [Actions](https://github.com/cognitom/scotch-egg-shield-module/actions/workflows/build.yml) で、書き込みたいコミットの Build の実行を開く
2. 下の Artifacts から `firmware` をダウンロードして展開する。中の
   `scotch_egg-seeeduino_xiao_ble-zmk.uf2` を書き込む
3. scotch-egg を USB でパソコンへ繋ぎ、XIAO をブートローダにする。初めてのときは XIAO の
   リセットボタンをすばやく2回押す。windmill を書き込んだあとなら、左右の <kbd>Fn</kbd> を両方押しながら
   <kbd>Enter</kbd> でもよい
4. USB ドライブが現れるので、`.uf2` をそこへコピーする。コピーが終わると XIAO が再起動して、
   ドライブは消える

手元でビルドするときは、[CLAUDE.md](CLAUDE.md#ビルドとテスト) のとおりコンテナを起動して
`bash scripts/build.sh` を回す。`output/scotch_egg-seeeduino_xiao_ble-zmk.uf2` ができる。

## 設定レイヤーの使い方

左右の <kbd>Fn</kbd> を両方押している間だけ、設定レイヤーになる。<kbd>Fn</kbd> は英数では最下段の
左から4つ目と右から4つ目、かなでは「そ」「ね」 (長押し)。片方だけなら Fn レイヤーのまま。

```
 USB   BLE0  BLE1  BLE2  BLE3  ・ ・ ・ ・ ・ BT消去 Boot
 ・    Win   Andr  ・    ・    ・ ・ ・ ・ ・ ・     Studio
 ・    JIS   US    ・    ・    ・ ・ ・ ・ ・ ・     ・
 ・    ・    ・    Fn    ・    ・ ・ ・ Fn ・ ・     ・
```

| キー (英数の刻印) | 効果 |
|--|--|
| <kbd>Esc</kbd> | USB へ送る |
| <kbd>Q</kbd> / <kbd>W</kbd> / <kbd>E</kbd> / <kbd>R</kbd> | BLE プロファイル 0 / 1 / 2 / 3 へ送る。まだペアリングしていなければ、ホストから「scotch-egg」とペアリングする |
| <kbd>A</kbd> / <kbd>S</kbd> | いまの接続先の OS を Windows / Android にする |
| <kbd>Z</kbd> / <kbd>X</kbd> | いまの接続先の配列を JIS / US にする |
| <kbd>P</kbd> | いま選んでいる BLE プロファイルのペアリングを消す |
| <kbd>Enter</kbd> | ブートローダへ入る (書き込み用) |
| <kbd>'</kbd> | ZMK Studio のロックを解除する |

- OS と配列は、接続先 (USB と BLE プロファイル 0〜3 の5つ) ごとに覚える。既定は Windows + JIS。
  「いまの接続先」は実際にキーを送っている先で、選んだ BLE プロファイルがまだ繋がっていなければ USB
- 配列はホストの配列 (日本語キーボード / 英語キーボード) に合わせる。食い違うと記号が別の文字になる
- 設定は最後に変えてから 60 秒後に保存され、電源を切っても残る。60 秒たつ前に電源を切ると、変えた設定は残らない
- Android では、かな / 英数の切り替えに Android アプリ「Windmill 言語切り替え」が要る
  ([windmill の導入方法](https://github.com/cognitom/windmill/blob/main/docs/install.md#android))。
  どの OS でも、IME の入力方式は「かな入力」にしておく

## 実機での確認

[docs/check.md](docs/check.md) に、windmill のテストのケースを実機で打って確かめる手順をまとめてある。
Windows / Android、JIS / US、USB / BLE の組み合わせと、設定が再起動後も残ることの確かめ方を含む。

## 開発

構成、ビルドとテストの回し方、守ってほしいことは [CLAUDE.md](CLAUDE.md) にある。
