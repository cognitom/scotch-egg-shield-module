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
| `src/` | behavior から呼ぶ windmill の処理 (出力表の引き方、文字の送り方、接続先ごとの設定) |
| `include/` | windmill のキーとレイヤーの番号 (`dt-bindings/windmill/`、キーマップからも読む)、出力表と behavior どうしで共有する宣言 (`windmill/`) |
| `tests/` | ZMK 形式のテスト。1件1ディレクトリで `native_posix_64.keymap` `events.patterns` `keycode_events.snapshot` |
| `tests/windmill.dtsi` | テスト用のキーマップ。`config/scotch_egg.keymap` と同じ並びを native_posix_64 で動かせる形で持ち、各テストが `#include` する |
| `tests/unit/` | ztest の単体テスト (`prj.conf` のあるディレクトリ)。native_posix の ZMK で回せないもの (BLE のプロファイルの切り替え) を、切り出した関数で見る |
| `docs/check.md` | 実機確認の手順書。windmill の `tests/` のケースを打つキーと画面に出るべきものの組で並べ、冒頭に windmill の README との差分を置く。挙動やキーマップを変えたら合わせて直す |
| `compose.agent.yaml` | ZMK を動かすコンテナ (`zmkfirmware/zmk-dev-arm:3.5`)。`zmk` サービスが常駐する |
| `CHANGES.md` | 変更履歴。`scripts/release.sh` が書き足す |
| `scripts/` | ビルド (`build.sh`)、テスト (`test.sh`)、リリース (`release.sh`)。`container/entrypoint.sh` はコンテナの起動処理、`bin/west` は shim が無い環境向けのラッパー |

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
- スクリプトは `west` を呼ぶ前に `.zmk/` へ移る。polar の shim はホストの cwd をそのまま
  コンテナへ持ち込むので、compose の `working_dir` に頼ると作業場所の外で呼んでしまう
- `test.sh` は ZMK の `app/run-test.sh` と同じ手順 (native_posix_64 でビルドして動かし、
  `events.patterns` で絞って `keycode_events.snapshot` と比べる) を、`west build` と
  `west build -t run` で回す。コンテナへ届くのは `west` だけで、`west test` は ZMK の
  west-commands が import 越しに載らず使えないため。絞り込みと比較はホスト側でやる
- `bash scripts/test.sh key-output` で `tests/key-output` の下だけ回せる
- `tests/unit/` の下の単体テストは ztest で、ZMK を通さず native_posix_64 でビルドして
  動かす。snapshot は無く、ztest の結果 (`output/tests/unit/<テスト>/run.log`) で判定する
- テストが落ちたら差分が出る。ログは `output/tests/<テスト>/` の下
  (`build.log`、`keycode_events_full.log` が全部、`keycode_events.log` が patterns を通した後)。
  `--accept` で snapshot を出た列に書き換えられるが、書き換えた中身は必ず読む

**コードを触ったら `scripts/test.sh` を通す。** キーマップやシールドを触ったときは
`scripts/build.sh` も通す。PR を出せば Test ワークフロー (`.github/workflows/test.yml`)
が同じ `test.sh` と `build.sh` を回し、Build ワークフロー (`.github/workflows/build.yml`)
が ZMK の `build-user-config.yml` で `build.yaml` の組み合わせを作る。

ZMK を上げるときは `config/west.yml` の `revision`、`.github/workflows/build.yml` の
`@` の版、`compose.agent.yaml` のイメージ (Zephyr の版) を揃える。
最下段の listener は hold-tap より後、keymap より前に並べていて (`CMakeLists.txt` の
`wm_place_bottom_listener`)、ZMK の `app/CMakeLists.txt` の作りに頼っている。上げたら
`tests/bottom-row/ctrl-then-thumb-is-ctrl-space` で並びを確かめる。

## リリース

`main` で `scripts/release.sh` を実行する。次のバージョン番号を決めて、
`CHANGES.md` を書き足してコミット・プッシュし、タグを打つところまでやる。
最初の版は `v0.1.0`。

```bash
bash scripts/release.sh              # パッチ版を上げる (v0.1.0 → v0.1.1)
bash scripts/release.sh --minor      # マイナー版を上げる (v0.1.1 → v0.2.0)
bash scripts/release.sh v1.0.0       # バージョンを直接指定する
bash scripts/release.sh --dry-run    # CHANGES.md へ書く内容だけ見て終わる
```

書き足す中身は、前回のタグ以降に main へ入ったPRのリンクを並べただけ。
`git log --first-parent` から拾うので、**PRのタイトルがそのまま履歴の1行になる**。
タグを push すると Release ワークフロー (`.github/workflows/release.yml`) が
テストとビルドを通して、GitHubのリリースへ `scotch_egg-seeeduino_xiao_ble-zmk.uf2` を貼る。

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
- **`CHANGES.md` は手で書かない。** リリースのたびに `release.sh` が生成する。
  PR のタイトルがそのまま履歴の1行になるので、タイトルは変更の中身が分かるように付ける
