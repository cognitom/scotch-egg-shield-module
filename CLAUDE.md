# CLAUDE.md

scotch-egg (Seeed XIAO nRF52840、4x12 のオーソ配列、LED なし) の ZMK のシールドと、
そこで動かす windmill のキー処理のモジュール。windmill
(https://github.com/cognitom/windmill) の QMK 実装を ZMK へ移植している途中で、
移植元は windmill の `firmware/windmill.c` `firmware/windmill.h`、キーマップは
`firmware/technik/keymaps/default/keymap.c`。挙動の仕様は windmill の `CLAUDE.md` の
「用語」と `tests/`。LED と `MY_DARK` は移植しない。

## 構成

| パス | 中身 |
|--|--|
| `config/west.yml` | ZMK の版の正。`.github/workflows/build.yml` の `build-user-config.yml@` と揃える |
| `config/scotch_egg.keymap` | scotch-egg のキーマップ |
| `config/boards/shields/scotch_egg/` | シールドの定義 (配線、物理配列、Kconfig) |
| `build.yaml` | Build ワークフローが作るボードとシールドの組み合わせ。ZMK Studio の `studio-rpc-usb-uart` スニペット付き |
| `zephyr/module.yml` | このリポジトリを Zephyr モジュールにする。下の CMake、Kconfig、DT を読ませる |
| `CMakeLists.txt` `Kconfig` | windmill の behavior を ZMK の app へ足す |
| `dts/bindings/behaviors/` | behavior の DT バインディング |
| `dts/behaviors/windmill.dtsi` | behavior のノード (`&wm_char` など)。キーマップから `#include <behaviors/windmill.dtsi>` |
| `src/behaviors/` | behavior の実装 |
| `tests/` | ZMK 形式のテスト。1件1ディレクトリで `native_posix_64.keymap` `events.patterns` `keycode_events.snapshot` |
| `compose.agent.yaml` | ZMK を動かすコンテナ (`zmkfirmware/zmk-dev-arm:3.5`)。`zmk` サービスが常駐する |
| `scripts/` | ビルド (`build.sh`) とテスト (`test.sh`)。`container/entrypoint.sh` はコンテナの起動処理、`bin/west` は shim が無い環境向けのラッパー |

`.zmk/` (west の取得物とビルドの作業場所) と `output/` は生成物なのでコミットしない
(`.gitignore` 済み)。

## ビルドとテスト

`build.sh` と `test.sh` は `west` を直接呼ぶ。この `west` は `compose.agent.yaml` の
`zmk` サービスの中で動くもので、ホストからそこへ届くようにしておく
(shim が無ければ `scripts/bin` を PATH に足す。CI もそうしている)。

```bash
docker compose -f compose.agent.yaml up -d --wait   # 初回は west update で数分かかる
bash scripts/test.sh    # tests/ の全件を native_posix_64 で回す
bash scripts/build.sh   # seeeduino_xiao_ble + scotch_egg を output/ に出す
```

- コンテナは起動時に `config/west.yml` を `.zmk/config/` へ写して `west update` する。
  リポジトリの直下には `zephyr/module.yml` があり、ここを west の作業場所にすると
  取ってくる `zephyr/` とぶつかるため。**`config/west.yml` を変えたらコンテナを起動し直す**
- `west` はコンテナの中で動くので、スクリプトからはコンテナ側のパス (`/scotch-egg/...`)
  で渡す
- テストが落ちたら差分が出る。ログは `.zmk/zmk/app/build/tests/<テスト>/` の下
  (`keycode_events_full.log` が全部、`keycode_events.log` が patterns を通した後)。
  `ZMK_TESTS_VERBOSE=1` でビルドの出力も出る。`scripts/bin/west` 経由なら
  `ZMK_TESTS_AUTO_ACCEPT=1` で snapshot を書き換えられるが、書き換えた中身は必ず読む

**コードを触ったら `scripts/test.sh` を通す。** キーマップやシールドを触ったときは
`scripts/build.sh` も通す。PR を出せば Test ワークフロー (`.github/workflows/test.yml`)
が同じ `test.sh` と `build.sh` を回し、Build ワークフロー (`.github/workflows/build.yml`)
が ZMK の `build-user-config.yml` で `build.yaml` の組み合わせを作る。

ZMK を上げるときは `config/west.yml` の `revision`、`.github/workflows/build.yml` の
`@` の版、`compose.agent.yaml` のイメージ (Zephyr の版) を揃える。

## 守ってほしいこと

- **キー処理の変更にはテストを添え、ホストへ出るキーの列で固定する。**
  キーコードだけ合っていても修飾キーの出方が壊れることがある。`events.patterns` で
  `hid_listener_keycode_` の行 (押し離しと implicit/explicit の修飾) を拾い、
  `keycode_events.snapshot` に1行ずつ固定する
- **windmill の挙動は windmill の `CLAUDE.md`「用語」と `tests/` を仕様として移す。**
  ZMK に合わせて挙動を変えるときは、その理由を issue か PR に書く
- **コメントとコミットメッセージは日本語。** 既存のコードとログの書き方に合わせる。
  コメントは「何をしているか」ではなく「なぜそうしたか」を書く
- **キー配列そのものの変更は勝手に決めない。** 配置は作者の好みの問題なので、
  issue で確認してから動く
