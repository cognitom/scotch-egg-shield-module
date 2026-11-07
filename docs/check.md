# 実機確認の手順書

scotch-egg (Seeed XIAO nRF52840) に書き込んだ windmill を、実機で打って確かめるための一覧。
windmill の `tests/` の各ケースを「打つキー」と「画面に出るべきもの」の組で並べてある。

自動テスト (`scripts/test.sh`) はホストへ出るキーの列までを固定している。この手順書で見るのは、
その先の IME と OS が受け取った結果と、native_posix では動かせないもの (BLE、設定の保存、
実際のキーの打ち方) のほう。

## windmill の README との差分

windmill の [README](https://github.com/cognitom/windmill#readme) と挙動が違うところ。
確認のときは、ここに書いたとおりになっていれば正しい。

| 項目 | windmill (QMK) | scotch-egg (ZMK) |
|--|--|--|
| LED とダークモード | <kbd>Fn</kbd>+<kbd>Enter</kbd> でダークモード | LED が無いので移していない。<kbd>Fn</kbd>+<kbd>Enter</kbd> は何もしない |
| 設定レイヤーの最上段 (Esc Q W E R の位置) | Geonix Rev.2.5 だけ USB / BT1 / BT2 / BT3 / 2.4G | USB / BLE 0 / BLE 1 / BLE 2 / BLE 3。押すと送り先とプロファイルを一緒に切り替える。windmill の 2.4G の枠は BLE 3 に当てた |
| 設定レイヤーの P の位置 | なし | 選んでいる BLE プロファイルのペアリングを消す (`&bt BT_CLR`) |
| 設定レイヤーの ' の位置 | なし | ZMK Studio のロック解除 (`&studio_unlock`) |
| 接続先ごとの設定 | USB / BT1〜3 / 2.4G の5つ | USB / BLE 0〜3 の5つ。BLE のプロファイルは4つに絞ってある |
| 設定の保存 | 押した時点で EEPROM に書く | 最後に変えてから 60 秒後に Zephyr settings へ書く (ZMK の `CONFIG_ZMK_SETTINGS_SAVE_DEBOUNCE` の既定)。**60 秒たつ前に電源を切ると、変えた設定は残らない** |
| ファームウェアの入手と書き込み | リリースページの `.hex` / `.bin` | Build ワークフローの成果物の `.uf2` を XIAO のブートローダへコピーする ([README](../README.md#書き込み方)) |

README には書かれていないが、windmill (QMK) と違うところ。

- 左下の Ctrl (<kbd>MY_LCTL</kbd>) をタップした直後に押し直して押し続けると、ZMK では Ctrl になる。
  windmill では QMK の mod-tap の作りでタップの繰り返しになり、Ctrl は取れない
  (`dts/behaviors/windmill.dtsi` の `wm_lctl`)
- BLE プロファイルを選んでも、そのプロファイルがまだ繋がっていなければ、繋がるまでは USB へ送る
  (ZMK の挙動のまま)。そのあいだの OS と配列も USB のぶんを使う

## 準備

### 書き込み

[README の書き込み方](../README.md#書き込み方) のとおり、確かめたいコミットの Build ワークフローの
成果物を書き込む。書き込んだコミットを記録しておく。

### ホストの設定

| ホスト | 配列 (JIS / US) | IME |
|--|--|--|
| Windows 11 | 「日本語キーボード (106/109 キー)」か「英語キーボード (101/102 キー)」 | Microsoft IME。入力方式を「かな入力」にする |
| Android | 物理キーボードの配列を「日本語」か「英語 (US)」 | Gboard の物理キーボードをかな入力にする。言語切替のため [Windmill 言語切り替え](https://github.com/cognitom/windmill-android/releases) を入れ、ユーザー補助サービスと英語用 IME を有効にする |

キーボード側の配列は、ホストの配列に合わせて設定レイヤーで選ぶ (下の「接続先と設定の選び方」)。
キーボード側とホスト側の配列が食い違っていると、記号が別の文字になる。

確かめるのは Windows ならメモ帳、Android なら Gboard で入力できるメモアプリでよい。
F キーや修飾キーなど文字にならないものは、ブラウザのキーの確認ページ
([Key Event Viewer](https://w3c.github.io/uievents/tools/key-event-viewer.html) など) で見る。

### 接続先と設定の選び方

設定レイヤーは、左右の <kbd>Fn</kbd> を両方押している間だけ上がる。<kbd>Fn</kbd> は英数では
最下段の左から4つ目と右から4つ目、かなでは「そ」「ね」の位置 (長押し)。以下 <kbd>Fn</kbd>+<kbd>Fn</kbd>+キー
と書く。

| 操作 | 位置 (英数の刻印) | 効果 |
|--|--|--|
| <kbd>Fn</kbd>+<kbd>Fn</kbd>+<kbd>Esc</kbd> | Esc | USB へ送る |
| <kbd>Fn</kbd>+<kbd>Fn</kbd>+<kbd>Q</kbd> / <kbd>W</kbd> / <kbd>E</kbd> / <kbd>R</kbd> | Q W E R | BLE プロファイル 0 / 1 / 2 / 3 へ送る |
| <kbd>Fn</kbd>+<kbd>Fn</kbd>+<kbd>A</kbd> / <kbd>S</kbd> | A S | いまの接続先の OS を Windows / Android にする |
| <kbd>Fn</kbd>+<kbd>Fn</kbd>+<kbd>Z</kbd> / <kbd>X</kbd> | Z X | いまの接続先の配列を JIS / US にする |
| <kbd>Fn</kbd>+<kbd>Fn</kbd>+<kbd>P</kbd> | P | いま選んでいる BLE プロファイルのペアリングを消す |

「いまの接続先」は、実際にキーを送っている先 (USB か、選んでいる BLE プロファイル) のこと。
BLE で繋ぐときは、Fn+Fn+Q などでプロファイルを選んでから、ホストで「scotch-egg」とペアリングする。

### 組み合わせ

OS (Windows / Android)、配列 (JIS / US)、接続 (USB / BLE) の8通りで確かめる。
全部の組み合わせで通すのは「どの組み合わせでも通すもの」の印 (★) の付いたケース。
残りは表の「通す組み合わせ」の欄のとおり。

| # | OS | 配列 | 接続 | 通すもの |
|--|--|--|--|--|
| 1 | Windows | JIS | USB | 全部 (Android のケースを除く) |
| 2 | Windows | JIS | BLE | ★ と、最下段・親指の組の全部 (無線の遅れで判定が変わらないこと) |
| 3 | Windows | US | USB | ★ と、配列で変わるケース (K8、K9) |
| 4 | Windows | US | BLE | ★ |
| 5 | Android | JIS | USB | ★ と、Android のケース (A1〜A8、L6〜L8) |
| 6 | Android | JIS | BLE | ★ と、Android のケース |
| 7 | Android | US | USB | ★ と、K4 |
| 8 | Android | US | BLE | ★ と、K4 |

BLE は Android と Windows で別のプロファイルにしておくと、設定の保存の確認 (下) と兼ねられる。

## 表記

- キーの位置は、英数のときは英字の刻印 (QWERTY)、かなのときはかなで書く。対応は下の図
- 「↓」は押す、「↑」は離す。「こ↓0 み↓100 み↑200」は、こ を押し、100ms 後に み を押し、
  200ms 後に み を離す
- 「すばやく」は 150ms 未満、「長押し」は 250ms 以上 (0.3〜0.5 秒) のつもりで打つ
- ⏱ の付いたケースは、手で狙うには時間の幅が狭い。数回打って、狙った打ち方になったときの
  結果を見る。1ms 単位の境界は手では打ち分けられないので、自動テストだけで見る (最後に一覧)
- 各ケースの前に <kbd>Enter</kbd> で未確定の文字を確定しておく。スペースは、未確定の文字があると
  変換になる。全角か半角かは IME の「スペースの入力」の設定による
- 「゛」「゜」は直前のかなと合わさって出ることがある (せ + ゛ → ぜ)
- テストの欄は windmill の `tests/` のテスト名 / このリポジトリの `tests/` のディレクトリ

```
かな (ベース)
 Esc  ぬ  ふ  あ  う  え  お  や  ゆ  よ  わ  Enter
 Tab  た  て  い  す  か  ん  な  に  ら  せ  ゛
 BS   ち  と  し  は  き  く  ま  の  り  れ  け
 Ctrl つ  さ  そ  ひ  こ  み  も  ね  る  め  ろ
      Win Alt Fn  記号 ⇧  ⇧  記号 Fn           ← 最下段の長押し

英数 (ベース)
 Esc  Q   W   E   R   T   Y   U   I   O   P   Enter
 Tab  A   S   D   F   G   H   J   K   L   ;   '
 BS   Z   X   C   V   B   N   M   ,   .   ↑   →
 Ctrl Win Alt Fn  \   ⇧Spc ⇧Spc /  Fn  App ←   ↓
                  記号          記号          ← \ と / は長押しで記号、⇧Spc は SandS

Shift 面 (こ / み の長押し中。・は下のかなのまま)
 ・   ・  ・  ぁ  ぅ  ぇ  ぉ  ゃ  ゅ  ょ  を  ・
 ・   ・  へ  ぃ  む  ・  ・  ほ  ・  「  」  ゜
 ・   っ  ・  ・  ・  ・  ・  ・  、  。  ・  ー
 ・   っ  ・  ・  ・  空  空  ？  、  。  ・  ・

記号 (ひ / も、英数の \ / の長押し中)
 ・   1   2   3   4   5   6   7   8   9   0   ・
 ・   !   @   #   $   %   ^   &   *   (   )   `
 ・   =   +   -   _   [   ]   ~   {   }   ↑   →
 ・   ・  ・  ・  ・  |   ?   ・  ・  ・  ←   ↓

Fn (そ / ね、英数の Fn の長押し中)
 Esc  F1  F2  F3  F4  F5  F6  F7  F8  F9  F10 なし
 Caps F11 F12 なし …                          なし
 Del  PrtSc なし なし なし 輝度- 輝度+ ミュート 音量- 音量+ ↑ →
 IME  ・  ・  ・  ・  ・  ・  ・  ・  ・  ←   ↓
```

## 確かめるケース

### 文字キーの出力

windmill の `test_key_output.cpp` と `test_shift_pair.cpp`。

| # | 打ち方 | 画面に出るべきもの | テスト |
|--|--|--|--|
| K1 ★ | かなで、最下段も含めて全キーを左上から順にすばやくタップ | ぬふあうえおやゆよわ / たていすかんなにらせ゛ / ちとしはきくまのりれけ / つさそひこみもねるめろ | `KeyOutput(Jis).kana_keys_without_shift` / `key-output/kana` |
| K2 ★ | かなで み を長押ししたまま、Shift 面を左上から順にタップ (こ み の位置は飛ばす) | ぬふぁぅぇぉゃゅょを / たへぃむかんほに「」゜ / っとしはきくま、。・ー / っさそひ?、。・ろ (? は半角) | `KeyOutput(Jis).kana_shift_layer_with_right_thumb` / `key-output/kana-shift-right` |
| K3 ★ | K2 を こ の長押しで | K2 と同じ | `KeyOutput(Jis).kana_shift_layer_with_left_thumb` / `key-output/kana-shift-left` |
| K4 | (Android + US のみ) かなで ら せ、み を長押しして ら せ | らせ「」 | `KeyOutputAndroid.kana_brackets_follow_host_os` / `host/android` |
| K5 ★ | 英数で ; ' , . と、\ の位置と / の位置をすばやくタップ | ;',.\\/ | `KeyOutput(Jis).alpha_symbols_without_shift` / `alpha-symbol/jis` |
| K6 ★ | 英数で右の SandS を長押ししたまま K5 と同じキー | :"<>\|? | `KeyOutput(Jis).alpha_symbols_with_thumb_shift` / `alpha-symbol/jis-right-shift` |
| K7 ★ | 英数で左の SandS を長押ししたまま ; ' \ | :"\| | `KeyOutputJis.alpha_symbols_with_left_thumb_shift` / `alpha-symbol/jis-left-shift` |
| K8 | 英数で右の SandS を長押ししたまま Q〜P、A〜L、Z〜M | QWERTYUIOPASDFGHJKLZXCVBNM | `KeyOutput.alpha_letters_with_thumb_shift` |
| K9 | 英数で ; を 0.6 秒以上押し続ける | JIS: ; が1つだけ (リピートしない)。US: ;;;; とリピートする | `KeyOutputJis.alpha_symbol_does_not_repeat` / `alpha-symbol/jis-no-repeat`、`KeyOutput.alpha_symbol_stays_down_while_held` / `alpha-symbol/us-held` |
| K10 ★ | かなで ぬ を 0.6 秒以上押し続け、押したまま は を押し、両方離す | ぬは (ぬ はリピートせず、離しても増えない) | `KeyOutput.kana_key_does_not_repeat` / `key-output/kana-no-repeat` |
| K11 ★ | 英数で \ の位置を長押ししたまま、記号を左上から順にタップ | 1234567890 / !@#$%^&*()\` / =+-_[]~{} / \|? (すべて半角) | `KeyOutput(Jis).sym_layer_on_alpha` / `key-output/sym-on-alpha` |
| K12 ★ | かなで ひ を長押ししたまま、K11 と同じキー。続けて ぬ | K11 と同じ半角の記号のあと、ぬ (かなに戻っている) | `KeyOutput(Jis).sym_layer_on_kana_wraps_with_ime_switch` / `key-output/sym-on-kana` |
| K13 | 英数で右の SandS を長押ししたまま K11 (? の位置は飛ばす) | K11 と同じ (記号では Shift が効かない) | `KeyOutput(Jis).sym_layer_on_alpha_ignores_shift` / `key-output/sym-on-alpha-ignores-shift` |
| K14 | かなで こ を長押ししたまま K12 (\| の位置は飛ばす) | K11 と同じ | `KeyOutput(Jis).sym_layer_on_kana_ignores_shift` / `key-output/sym-on-kana-ignores-shift` |
| K15 | 英数で \ の位置を長押しし、! ↓ = ↓ ! ↑ = ↑ と重ねて打つ | != (+ にならない) | `KeyOutput.sym_roll_over_does_not_leak_shift` / `key-output/sym-roll-over` |
| K16 | かなで ぬ、ひ を長押ししたまま こ の位置を3回 | ぬ\|\|\| (1つ目も \| で、\ にならない) | `KeyOutput.sym_after_kana_waits_between_shift_and_key` / `key-output/sym-after-kana` |
| K17 | かなで ぬ↓、ひ↓ (0.3 秒)、ぬ↑、ひ↑。続けて何か打つ | ぬ だけ。押しっぱなしのキーが残らない | `KeyOutput.kana_key_released_under_sym_layer_is_not_stuck` / `key-output/kana-released-under-sym` |
| K18 | (JIS) 英数で \ の位置を長押ししたまま @ の位置 (W) | @ (\ は出ない) | `KeyOutputJis.backslash_key_hold_is_sym_layer` / `alpha-symbol/jis-backslash-hold` |
| K19 | 英数で f、Ctrl タップでかなへ、み を長押ししたまま の を2回 | f、、 (1つ目も 、 で、ね にならない) | `ShiftPair.thumb_shift_keeps_held_shift_on_first_keypress` / `shift-pair/keeps-held-shift` |
| K20 | 英数で f、Ctrl タップでかなへ、み を長押ししたまま な を2回 | fほほ (1つ目も ほ で、ー にならない) | `ShiftPair.unshifted_pair_waits_for_ime_on_first_keypress` / `shift-pair/waits-for-ime` |
| K21 | かなで み を長押ししたまま ら | 「 | `ShiftPair.shifted_pair_reuses_held_shift` / `shift-pair/reuses-held-shift` |
| K22 | かなで み を長押ししたまま す | む | `ShiftPair.unshifted_pair_drops_and_restores_shift` / `shift-pair/drops-and-restores` |
| K23 | 英数で \ の位置を長押しして ! を打ち、すぐ Enter。かなで み を長押ししたまま ぬ を打ち、すぐ BS | ! のあとに改行。ぬ が出て、BS で消える (BS が ぬ を追い越さない) | (ZMK で足したもの) `key-output/plain-key-waits-for-output` |

`KeyOutputJisAndroid.same_as_windows` は、組み合わせ 5・6 で K1〜K3、K11 を通せば見たことになる。

### かなの「？」

windmill の `test_kana_qmark.cpp`。

| # | 打ち方 | 画面に出るべきもの | テスト |
|--|--|--|--|
| Q1 | かなで も をすばやくタップ | も | `KanaQmark.without_shift_sends_plain_keycode` / `kana-qmark/without-shift-sends-plain-keycode` |
| Q2 ★ | かなで み を長押ししたまま も | ? (半角。・ や / にならない) | `KanaQmark.shifted_tap_sends_halfwidth_question_mark` / `kana-qmark/shifted-tap-sends-halfwidth-question-mark` |
| Q3 | かなで み を長押ししたまま は、も、も | は?? (1つ目の ? も半角の ?) | `KanaQmark.shifted_tap_is_stable_after_other_key` / `kana-qmark/shifted-tap-is-stable-after-other-key` |
| Q4 | かなで も を押し、すぐ (150ms 未満で) 1 の位置 (ぬ) をタップし、も を離す | 1 (半角)。かなのまま | `KanaQmark.hold_still_enters_sym_layer` / `kana-qmark/hold-still-enters-sym-layer` |

### 最下段のタップ・ホールド

windmill の `test_bottom_row.cpp` の issue #85 のケースと、待っている間の出方。すべてかなで打つ。
⏱ は時間の幅が狭いもの。

| # | 打ち方 | 画面に出るべきもの | テスト |
|--|--|--|--|
| a ⏱ | こ↓0 み↓100 わ↓200 わ↑230 み↑240 こ↑250 | みを | `a_shift_then_partner_tap_then_key` / `bottom-row/a-…` |
| b | こ↓0 み↓20 わ↓40 わ↑60 み↑70 こ↑80 (3つを転がすように) | こみわ | `b_quick_roll_keeps_press_order` |
| c | こ↓0 わ↓100 わ↑130 こ↑160 | を | `c_key_within_150ms_waits_then_shifted` |
| d | こ↓0 わ↓200 わ↑230 こ↑260 | を | `d_key_after_150ms_holds_at_once` |
| e ⏱ | こ↓0 こ↑200 | こ (離したときに出る) | `e_release_before_250ms_is_tap`、`tap_does_not_stay_down` |
| f | こ を 0.3 秒以上押して離す | 何も出ない | `f_long_press_is_shift_only` |
| g | こ↓0 み↓200 み↑230 こ↑260 | スペース | `g_partner_tap_is_space` |
| h | こ↓0 あ↓200 あ↑230 み↓300 こ↑350 わ↓400 わ↑430 み↑470 | ぁを | `h_partner_after_shift_was_used_holds_by_key` |
| i | こ↓0 あ↓200 あ↑230 み↓300 こ↑350 わ↓500 わ↑530 み↑550 | ぁを | `i_partner_after_shift_was_used_holds_at_once` |
| j ⏱ | ひ↓0 も↓100 た↓200 た↑230 も↑240 ひ↑250 | も! (! は半角) | `j_sym_hold_then_partner_tap_then_key` |
| k | ひ↓0 も↓300 も↑350 ひ↑400 | も | `k_partner_tap_on_sym_layer` |
| l ⏱ | そ↓0 ね↓100 ぬ↓200 ぬ↑230 ね↑240 そ↑250 | ね のあと F1 (キーの確認ページで見る) | `l_fn_hold_then_partner_tap_then_key` |
| m | (Android にしてから) そ↓0 ね↓50 た↓300 た↑330 ね↑340 そ↑350 | 何も出ない (そ ね た も出ない)。Windows に戻っていて、Ctrl タップで IME のモードが切り替わる | `BottomRowConf.m_both_fn_hold_reach_conf_layer` |
| n | つ を 0.3 秒以上押して離す | Win の単独押し (Windows ならスタートメニューが開く) | `n_long_press_is_gui_only` |
| o | さ を 0.3 秒以上押して離す | Alt の単独押し (メモ帳ならメニューにフォーカスが移る) | `o_long_press_is_alt_only` |
| p | こ↓0 み↓200 こ↑300 み↑400 | スペース | `p_space_x_released_first_after_250ms` |
| q | こ↓0 み↓200 み↑300 こ↑400 | スペース | `q_space_y_released_first_after_250ms` |
| r ⏱ | こ↓0 み↓100 こ↑200 み↑300 | スペース | `r_space_x_released_in_150_to_250ms` |
| s | こ↓0 み↓100 み↑200 こ↑300 | スペース | `s_space_y_released_first` |
| t ⏱ | こ↓0 み↓100 こ↑130 み↑200 | こみ | `t_x_released_before_150ms_is_tap` |
| u ⏱ | こ↓0 み↓100 み↑130 こ↑200 | スペース | `u_space_both_short` |
| v ⏱ | こ↓0 み↓100 こ↑130 み↑400 | こ | `v_x_tap_then_y_hold` |
| w ⏱ | こ↓0 み↓100 こ↑160 み↑400 | こ (み を押して 250ms たったときに出る) | `w_x_waits_for_y_then_tap` |
| x | こ↓0 み↓100 こ↑300 み↑400 | 何も出ない | `x_both_hold` |
| y | こ↓0 あ↓200 あ↑230 み↓300 こ↑350 み↑400 | ぁみ | `y_partner_tap_after_shift_was_used_is_tap` |
| z | ひ↓0 こ↓200 こ↑230 ひ↑260 | \| (半角) | `z_later_tap_promotes_earlier_to_hold` |
| aa | こ↓0 み↓200 み↑230 み↓300 み↑330 こ↑400 | スペースが2つ | `aa_each_partner_tap_is_space` |
| ab | こ↓0 み↓200 み↑230 わ↓300 わ↑330 こ↑400 | スペースのあと を | `ab_space_stays_when_shift_used_later` |
| W1 | こ を押し、すぐ わ をタップし、こ を押したまま様子を見る | こ を押して 150ms たつまで何も出ず、そこで を が出る | `no_report_while_waiting` |
| W2 | こ↓0 あ↓20 わ↓40 あ↑60 わ↑80 こ↑100 | こあわ (押した順) | `waiting_keys_keep_press_order` |
| W3 ⏱ | ひ↓0 こ↓200 ひ↑240 こ↑260 | ひこ | `bottom_press_does_not_decide_earlier` |
| W4 | Ctrl を長押ししたまま こ をすばやくタップ | Ctrl+Space (キーの確認ページで見る) | (ZMK で足したもの) `ctrl-then-thumb-is-ctrl-space` |

テスト名は windmill の `BottomRow.` の下のもの。このリポジトリのテストは `tests/bottom-row/` の下の、
テスト名の `_` を `-` にしたディレクトリ (a なら `a-shift-then-partner-tap-then-key`)。
`tap_does_not_stay_down` は e と同じ打ち方で、出る時刻まで見るものは移していない。

### 親指の組 (こ + み の半角スペース)

windmill の `test_bottom_row.cpp` の `ThumbChordTest`。かなで、先に押すほう (X) と後から押すほう (Y) を
「こ→み」と「み→こ」の両方で打つ。結果の「X」「Y」は、X と Y のかな (こ→み なら こ と み)。
ディレクトリは `tests/thumb-chord/` の下。

| # | 打ち方 | 画面に出るべきもの | テスト |
|--|--|--|--|
| T1 | X↓0 Y↓20 Y↑70 X↑80 (ほぼ同時に押して、Y から離す) | スペース | `y_first_overlap_over_lag_is_space` |
| T2 ⏱ | X↓0 Y↓50 Y↑100 X↑120 | スペース | `y_first_overlap_equals_lag_is_space` |
| T3 ⏱ | X↓0 Y↓50 Y↑99 X↑120 (重なりが時間差より短い) | XY | `y_first_overlap_under_lag_is_two_taps` |
| T4 ⏱ | X↓0 Y↓50 X↑100 Y↑140 | スペース | `x_first_overlap_equals_lag_is_space` |
| T5 ⏱ | X↓0 Y↓50 X↑99 Y↑140 | XY | `x_first_overlap_under_lag_is_two_taps` |
| T6 | X↓0 Y↓40 X↑70 Y↑110 (X を先にすぐ離す) | XY | `x_first_short_overlap_is_two_taps` |
| T7 | X↓0 Y↓100 X↑130 Y↑200 | XY | `x_first_late_partner_is_two_taps` |
| T8 | X↓0 Y↓50 X↑100 Y↑400 (Y を長押し) | X だけ (Y を押して 250ms たったときに出る) | `x_first_y_reaches_hold` |
| T9 | X↓0 Y↓20 わ↓40 わ↑60 Y↑70 X↑80 | XYわ | `other_key_before_first_release_is_not_chord` |
| T10 | X↓0 わ↓10 わ↑15 Y↓20 Y↑70 X↑80 | Xわ Y | `other_key_before_y_is_not_chord` |
| T11 | X↓0 Y↓20 ひ↓30 ひ↑40 Y↑70 X↑80 | XYひ | `other_bottom_key_before_first_release_is_not_chord` |
| T12 | X↓0 Y↓50 X↑100 わ↓120 わ↑130 Y↑140 | XYわ | `other_key_while_waiting_for_y_breaks_chord` |
| T13 | X↓0 Y↓50 X↑100 ひ↓110 ひ↑120 Y↑140 | XYひ | `other_bottom_key_while_waiting_for_y_breaks_chord` |
| T14 | X↓0 Y↓20 Y↑70 わ↓100 わ↑130 X↑200 | スペースのあと を | `space_then_x_stays_as_shift` |
| T15 | X↓0 Y↓20 Y↑30 Y↓40 Y↑100 X↑120 | XYY | `repress_after_failed_first_release_is_not_chord` |

### 親指の Shift

windmill の `test_thumb_shift.cpp` と `test_alpha_thumb_shift.cpp`。

| # | 打ち方 | 画面に出るべきもの | テスト |
|--|--|--|--|
| S1 | かなで こ を長押し、み も長押し、こ を離し、わ、み を離す | を (持ち替えても Shift 面のまま) | `ThumbShift.handover_left_to_right_keeps_shift` / `thumb-shift/handover-left-to-right` |
| S2 | S1 の左右を入れ替える | を | `ThumbShift.handover_right_to_left_keeps_shift` / `thumb-shift/handover-right-to-left` |
| S3 | かなで こ と み を長押しして両方離し、わ | わ (Shift が残っていない) | `ThumbShift.both_released_clears_shift` / `thumb-shift/both-released-clears-shift` |
| S4 | かなで こ↓0 み↓100 み↑200 こ↑250 | スペースが1つ (Shift+Space の扱いにならない) | `ThumbShift.space_drops_shift_in_its_own_report` / `thumb-shift/space-drops-shift-in-its-own-report` |
| S5 | かなで こ↓0 み↓200 こ↑300 み↑350、続けて は | スペースのあと は | `ThumbShift.no_shift_left_after_space` / `thumb-shift/no-shift-left-after-space` |
| S6 | 英数で左の SandS を長押し、右も長押し、左を離して q、右を離す | Q | `AlphaThumbShift.handover_left_to_right_keeps_shift` / `alpha-thumb-shift/handover-left-to-right` |
| S7 | S6 の左右を入れ替える | Q | `AlphaThumbShift.handover_right_to_left_keeps_shift` / `alpha-thumb-shift/handover-right-to-left` |
| S8 | 英数で左右の SandS を長押しして両方離し、q | q | `AlphaThumbShift.both_released_clears_shift` / `alpha-thumb-shift/both-released` |
| S9 | 英数で左の SandS を長押ししたまま右の SandS をタップ | スペース (Shift+Space) | `AlphaThumbShift.hold_left_tap_right_sends_shift_space` / `alpha-thumb-shift/hold-left-tap-right` |
| S10 | S9 の左右を入れ替える | スペース (Shift+Space) | `AlphaThumbShift.hold_right_tap_left_sends_shift_space` / `alpha-thumb-shift/hold-right-tap-left` |

### 言語切替

windmill の `test_lang_toggle.cpp`。かな / 英数は IME の表示 (Windows はタスクバーの「あ」「A」、
Android は切り替わった IME) で見る。

| # | 打ち方 | 画面に出るべきもの | テスト |
|--|--|--|--|
| L1 ★ | 起動直後に ぬ の位置、Ctrl タップ、ぬ、Ctrl タップ、ぬ の位置 | q (起動直後は英数)、IME がかなになって ぬ、IME が英数に戻って q | `LangToggle.tap_toggles_ime_and_base_layer` / `lang-toggle/tap-toggles` |
| L2 | 英数で Ctrl をすばやく2回タップしてから ぬ の位置 | q (2回切り替わって英数に戻る) | `LangToggle.quick_double_tap_toggles_twice` / `lang-toggle/quick-double-tap` |
| L3 | 英数で Ctrl をタップし、間を空けずに ぬ | ぬ (もうかな) | `LangToggle.key_right_after_tap_resolves_on_new_layer` / `lang-toggle/key-right-after-tap` |
| L4 | 英数のまま IME だけをマウスなどでかなにしてから Fn+Ctrl、ぬ の位置。かなに切り替え、IME だけ英数にしてから Fn+Ctrl、ぬ | IME が英数に戻って q。IME がかなに戻って ぬ (キーボード側のかな / 英数は動かない) | `LangToggle.fn_ctrl_switches_host_only` / `lang-toggle/ime-only` |
| L5 | 英数で Ctrl を押し、ぬ の位置を打ち、Ctrl を離す。続けて ぬ の位置 | Ctrl+Q (メモ帳では何も起きない)。IME は切り替わらず q | `LangToggle.hold_does_not_toggle` / `lang-toggle/hold-does-not-toggle` |
| L6 | (Android) L1 と同じ | L1 と同じ (アプリが Gboard と Windmill 英語を切り替える) | `LangToggleAndroid.tap_sends_henkan_and_muhenkan` / `lang-toggle/android-tap` |
| L7 | (Android) L4 と同じ | L4 と同じ | `LangToggleAndroid.fn_ctrl_specifies_host_only` / `lang-toggle/android-ime-only` |
| L8 | (Android) かなで も を押し、すぐ 1 の位置をタップし、も を離す | 1 (半角)。IME はかなのまま (Windmill 英語に切り替わらない) | `LangToggleAndroid.kana_symbol_still_uses_lng` / `lang-toggle/android-kana-symbol` |

### 修飾のホールド中の英数

windmill の `test_hold_layer.cpp`。かなで打つ。修飾つきのキーは、キーの確認ページで見るか、
効果 (Win+Q は Windows の検索が開く) で見る。検索などが開いたら、閉じて入力欄へ戻ってから続ける。

| # | 打ち方 | 画面に出るべきもの | テスト |
|--|--|--|--|
| H1 | つ を長押ししたまま ぬ、つ を離して ぬ | Win+Q、そのあと ぬ (かなに戻る) | `HoldLayer.gui_hold_switches_to_alpha_and_back` / `hold-layer/gui-hold` |
| H2 | さ を長押ししたまま ぬ、さ を離して ぬ | Alt+Q (文字は入らない)、そのあと ぬ | `HoldLayer.alt_hold_switches_to_alpha_and_back` / `hold-layer/alt-hold` |
| H3 | つ を押してすぐ ぬ をタップし、つ を離す | Win+Q (ぬ は出ない) | `HoldLayer.gui_hold_by_interrupt_resolves_on_alpha` / `hold-layer/gui-hold-interrupt` |
| H4 | つ をすばやくタップし、ぬ | つぬ | `HoldLayer.tap_stays_on_kana` / `hold-layer/gui-tap` |
| H5 | Ctrl を長押し、つ も長押し、Ctrl を離して ぬ、つ を離して ぬ | Win+Q、そのあと ぬ | `HoldLayer.ctrl_and_gui_overlap_keeps_alpha` / `hold-layer/ctrl-and-gui-overlap` |
| H6 | (Windows) つ を長押し、さ も長押し、つ を離して ぬ、さ を離して ぬ | Alt+Q、そのあと ぬ | `HoldLayer.gui_and_alt_overlap_keeps_alpha` / `hold-layer/gui-and-alt-overlap` |
| H7 | つ を長押しして離す、Ctrl タップ、ぬ の位置、Ctrl タップ、ぬ | Win の単独押し、q、ぬ | `HoldLayer.base_layer_switch_works_after_gui_hold` / `hold-layer/base-switch-after-gui-hold` |
| H8 | 英数で Ctrl を長押ししたまま q、離して q。Win を押して離し、q | Ctrl+Q、q、Win の単独押し、q (英数のまま) | (ZMK で足したもの) `hold-layer/ctrl-on-alpha-base` |
| H9 | かなで Ctrl を長押しし、\ の位置 (ひ) も長押ししたまま 1 の位置 | Ctrl+1 (英数に切り替えて送り、かなに戻る。離したあと ぬ を打つと ぬ) | (ZMK で足したもの) `hold-layer/sym-under-ctrl` |

### Android の Win / Alt

windmill の `test_android_mods.cpp`。W1 は Windows、A1〜A8 は Android で打つ。Android では
Win と Alt を重ねて離すと Caps Lock が切り替わるので、A1〜A8 のどれでも Caps Lock が勝手に
切り替わらないことを見る。

| # | 打ち方 | 画面に出るべきもの | テスト |
|--|--|--|--|
| W1 | (Windows) 英数で Win を押したまま . | 絵文字パネルが開く | `WinMods.gui_dot_is_sent_as_is` / `android-mods/win-gui-dot-is-sent-as-is` |
| A1 | 英数で Win↓ Alt↓ q Alt↑ Win↑ | Win+Q だけ (Alt は出ない) | `AndroidMods.alt_after_gui_is_not_sent` / `android-mods/alt-after-gui-is-not-sent` |
| A2 | 英数で Alt↓ Win↓ Alt↑ q Win↑ | q (小文字のまま) | `AndroidMods.gui_after_alt_is_not_sent` / `android-mods/gui-after-alt-is-not-sent` |
| A3 | 英数で Win↓ Alt↓ Alt↑ Win↑、続けて Alt を押して離す | Win の単独押し (アプリ一覧が開くことがある) と、Alt の単独押し (2回目の Alt は出る) | `AndroidMods.blocked_mod_works_again_after_release` / `android-mods/blocked-mod-works-again-after-release` |
| A4 | かなで つ↓ さ↓ (すぐ) を 0.3 秒以上押し、つ↑、ぬ、さ↑、ぬ | ぬぬ (Caps Lock は切り替わらない) | `AndroidMods.kana_roll_over_gui_and_alt_sends_gui_only` / `android-mods/kana-roll-over-gui-and-alt-sends-gui-only` |
| A5 | かなで つ さ をすばやく続けて打つ | つさ | `AndroidMods.kana_quick_roll_over_is_two_taps` / `android-mods/kana-quick-roll-over-is-two-taps` |
| A6 | 英数で Win を押したまま . を2回、q | Alt+. が2回、Win+Q (アプリ一覧は開かない) | `AndroidMods.gui_dot_is_sent_as_alt_dot` / `android-mods/gui-dot-is-sent-as-alt-dot` |
| A7 | かなで つ を押してすぐ り をタップし、ぬ、つ を離して ぬ | Alt+.、Win+Q、ぬ (アプリ一覧は開かない) | `AndroidMods.kana_gui_dot_is_sent_as_alt_dot` / `android-mods/kana-gui-dot-is-sent-as-alt-dot` |
| A8 | 英数で .、Alt を押したまま . | .、Alt+. | `AndroidMods.dot_without_gui_is_sent_as_is` / `android-mods/dot-without-gui-is-sent-as-is` |

### Fn レイヤー

windmill の `test_fn_layer.cpp`。キーの確認ページで見る。

| # | 打ち方 | 画面に出るべきもの | テスト |
|--|--|--|--|
| F1 | 英数で左の Fn を長押ししたまま Esc、1〜0 の位置、Tab、A、S。右の Fn でも | Esc F1〜F10 CapsLock F11 F12 | `FnLayer.function_keys_follow_number_row_on_alpha` / `fn-layer/function-keys-on-alpha` |
| F2 | かなで そ / ね を長押しして F1 と同じキー | F1 と同じ (そ ね は出ない) | `FnLayer.function_keys_follow_number_row_on_kana` / `fn-layer/function-keys-on-kana` |
| F3 | Fn を長押ししたまま 2段目の D〜' | 何も出ない | `FnLayer.rest_of_second_row_is_empty` / `fn-layer/rest-of-second-row-empty` |
| F4 | Fn を長押ししたまま Tab を2回、Fn を離して Tab | Caps Lock がオンになってオフに戻る、Tab | `FnLayer.fn_tab_sends_caps_lock_each_time` / `fn-layer/tab-sends-caps-lock` |
| F5 | Fn を長押ししたまま BS、Z、B、N、M、,、. | Delete、PrintScreen、画面が暗く / 明るく、ミュート、音量 - / + | (windmill にテストは無い) |

### 設定レイヤー

windmill の `test_conf_layer.cpp`。OS を Android に変えるケースのあとは、Fn+Fn+A で Windows に戻しておく。

| # | 打ち方 | 画面に出るべきもの | テスト |
|--|--|--|--|
| C1 | 英数で左の Fn だけ長押しして S。右の Fn だけでも。そのあと Ctrl タップ | F12 が2回。OS は変わっておらず、Ctrl タップで IME のモードが切り替わる | `ConfLayer.single_fn_stays_on_fn_layer` / `conf-layer/single-fn-stays-on-fn` |
| C2 | 英数で左右の Fn を押して A、左の Fn を離して A、右も離して A | (設定レイヤーなので何も出ない)、F11、a | `ConfLayer.releasing_left_fn_returns_to_fn_layer` / `conf-layer/release-left-fn` |
| C3 | 英数で左右の Fn を押し、右を離して A、右を押し直して A、両方離して A | F11、(何も出ない)、a | `ConfLayer.releasing_right_fn_returns_to_fn_layer_and_back` / `conf-layer/release-right-fn-and-back` |
| C4 | かなで そ ね を長押しして た、そ を離して た、ね も離して た | (何も出ない)、F11、た | `ConfLayer.releasing_one_fn_on_kana_returns_to_fn_layer` / `conf-layer/release-one-fn-on-kana` |
| C5 | 英数で Fn+Fn+S、そのあと Ctrl タップ | OS が Android になる。Ctrl タップで変換が送られる (Windows ならキーの確認ページで Convert、Android ならアプリが Gboard へ切り替える) | `ConfLayerAlpha.both_fn_reach_conf_layer` / `conf-layer/both-fn-on-alpha` |
| C6 | かなで そ ね を長押しして S (と) の位置、そのあと Ctrl タップ | C5 と同じ | `ConfLayerKana.both_fn_reach_conf_layer` / `conf-layer/both-fn-on-kana` |
| C7 | かなで そ↓ ね↓ (すぐ) と をタップし、0.2 秒ほど押したまま両方離す | そ ね と は出ず、C5 と同じ | `ConfLayerKanaInterrupt.both_fn_confirmed_by_interrupt` / `conf-layer/both-fn-quick-on-kana` |
| C8 | かなで ひ も を長押しし、ひ を離して 1 の位置、も も離して ぬ | 1 (半角)、ぬ | (ZMK で足したもの) `sym-layer/release-one-sym` |

### 接続先ごとの設定

windmill の `test_host_os.cpp` と `test_host_layout.cpp`。記号の @ (英数で \ の位置を長押しして W)
で配列を、Ctrl タップで OS を見る。キーボード側の配列がホストと食い違うと、@ は JIS のホストで "、
US のホストで [ になる。

| # | 打ち方 | 画面に出るべきもの | テスト |
|--|--|--|--|
| N1 ★ | 書き込み直後 (設定を一度も変えていない) に @ | どの接続先でも、JIS のホストで @ (既定は JIS) | `HostLayout.default_is_jis` / `host/default-jis` |
| N2 | USB で Fn+Fn+X (US)、@。Fn+Fn+Z (JIS)、@ | US のホストで @、JIS に戻すと [ (ホストを US にしている場合) | `HostLayoutOutput.output_follows_host_layout` / `host/us` |
| N3 | BLE 0 で Fn+Fn+S (Android)、Ctrl タップ。USB と BLE 1 に切り替えて Ctrl タップ。BLE 0 に戻して Ctrl タップ | BLE 0 だけが Android の切り替え (変換 / 無変換) で、USB と BLE 1 は Windows のまま | `HostOs.android_is_remembered_per_host` / `host/android`、`tests/unit/host-config` |
| N4 | BLE 1 で Fn+Fn+X (US)、続けて Fn+Fn+S (Android) | BLE 1 は US のまま Android になる。USB と BLE 0 の配列は JIS のまま | `HostLayoutPerHost.us_is_remembered_per_host`、`tests/unit/host-config` |
| N5 | 全部の接続先で Fn+Fn+S にしてから、BLE 2 だけ Fn+Fn+A | BLE 2 だけ Windows、ほかは Android | `HostOsWinBack.win_on_one_host_keeps_others`、`HostLayoutJisBack.jis_on_one_host_keeps_others`、`tests/unit/host-config` |

## 設定が再起動後も残ること

接続先ごとの OS と配列は、Zephyr settings の `windmill/host` に 32bit のまま保存し、起動時に
読み込む (`src/host.c`)。native_posix の自動テストでは保存も読み込みも通っていないので、
ここは実機でしか確かめられない。windmill の `HostLayoutReboot.survives_reboot` と
`HostLayoutBootRead.boot_reads_eeprom` に当たる。

保存は最後に設定を変えてから 60 秒後 (ZMK の `CONFIG_ZMK_SETTINGS_SAVE_DEBOUNCE` の既定) に
まとめて書く。

1. 接続先ごとに違う設定にする。例:

   | 接続先 | OS | 配列 |
   |--|--|--|
   | USB | Windows | US |
   | BLE 0 | Android | JIS |
   | BLE 1 | Windows | JIS (既定のまま) |
   | BLE 2 | Android | US |

2. 最後に変えてから **60 秒以上** 待つ
3. 電源を切る。USB だけで動かしているなら抜き差し、電池をつないでいるならリセットボタンを1回押す
4. 各接続先へ切り替えて、@ と Ctrl タップで設定を見る

| # | 確かめること | 期待 |
|--|--|--|
| R1 | 再起動後、各接続先の OS と配列 | 1. で選んだとおり |
| R2 | 再起動後に何も変えずにもう一度再起動 | R1 と同じ (起動時に書き戻したり、別の接続先のものとして読んだりしない) |
| R3 | 設定を変えて、60 秒たたないうちに再起動 | 変える前の設定に戻る (想定どおり。上の差分を参照) |
| R4 | 再起動直後のかな / 英数 | 英数から始まる (かな / 英数は保存しない) |
| R5 | Fn+Fn+P でペアリングを消し、同じプロファイルで繋ぎ直す | そのプロファイルの OS と配列は残っている (ペアリングとは別に持っている) |

## 自動テストだけで見るもの

手では打ち分けられない境界や、実機では作れない状態は、自動テストだけで見る。ここが通っていれば、
上のケースで狙った打ち方をしたときの結果も揃う。

- 判定の境界 (`BottomRow.` / `tests/bottom-row/`): `interrupt_boundary_149ms_is_tap`、
  `interrupt_boundary_150ms_is_hold`、`key_at_150ms_holds_at_once`、`hold_boundary_249ms_is_tap`、
  `hold_boundary_250ms_is_hold`、`thumb_pair_boundary_149ms_is_chord_space`、
  `thumb_pair_boundary_150ms_is_space`、`thumb_pair_boundary_149ms_without_overlap_is_two_taps`、
  `thumb_pair_y_249ms_is_space`、`thumb_pair_y_250ms_is_hold`
- 親指の組の境界 (`ThumbChordTest.` / `tests/thumb-chord/`): `x_first_y_249ms_is_space`、
  `x_first_y_250ms_is_hold`
- 保存した値の形 (windmill は EEPROM のビット位置): `HostOsEeprom.holds_two_bits_per_host`、
  `HostOsOutOfRange.falls_back_to_usb`、`HostOsLegacy.reserved_bit_is_ignored`、
  `HostLayoutOutOfRange.falls_back_to_usb`、`HostLayoutUpgrade.eeprom_without_layout_is_jis`、
  `HostLayoutReserved.unknown_value_is_jis`。ここでは `tests/unit/host-config` が、
  32bit への詰め方と範囲外のプロファイルの扱いを見る
- Shift や英数 / かなの切り替えとキーの間の待ち (`IME_WAIT_MS`)。実機では K16、K19、K20、Q3 の結果で見る
