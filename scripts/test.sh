#! /usr/bin/env bash
# tests/ の下の ZMK 形式のテストを回す。
#
#   bash scripts/test.sh            # 全件
#   bash scripts/test.sh wm-char    # tests/wm-char の下だけ
#   bash scripts/test.sh --accept   # 出たキーの列で keycode_events.snapshot を書き換える
#
# 手順は ZMK の app/run-test.sh と同じ: native_posix_64.keymap を native_posix_64 向けに
# ビルドして動かし、ログを events.patterns で絞って keycode_events.snapshot と比べる。
# run-test.sh をそのまま使わないのは、コンテナへ届くのが `west` だけだから
# (`west test` は ZMK の west-commands が import 越しに載らず使えない)。
# ビルドと実行は west に任せ、絞り込みと比較はホスト側でやる
set -euo pipefail

# west は compose.agent.yaml の zmk サービスのものを呼ぶ (build.sh 参照)
cd "$(dirname "$0")/.."
root=/scotch-egg

accept=
filter=tests
for arg in "$@"; do
  case "$arg" in
    --accept) accept=1 ;;
    *) filter="tests/$arg" ;;
  esac
done

if [ ! -f .zmk/.ready ]; then
  echo "🚨  .zmk/ の準備ができていません。compose.agent.yaml の zmk サービスを起動してください" 1>&2
  exit 1
fi

mapfile -t cases < <(find "$filter" -name native_posix_64.keymap -printf '%h\n' | sort)
if [ "${#cases[@]}" -eq 0 ]; then
  echo "🚨  $filter の下にテストがありません" 1>&2
  exit 1
fi

failed=()
for dir in "${cases[@]}"; do
  name="${dir#tests/}"
  # ビルドはコンテナが .zmk/ の下に作る。.zmk/ はコンテナ (root) のものでホストから
  # 書けないことがあるので、ホストで書くログは output/ の下に分ける
  build=".zmk/build/tests/$name"
  out="output/tests/$name"
  mkdir -p "$out"

  # ビルドの出力は多いので、落ちたときだけ見せる
  if ! west build -p -s "$root/.zmk/zmk/app" -d "$root/$build" -b native_posix_64 -- \
      -DCONFIG_ASSERT=y \
      -DZMK_CONFIG="$root/$dir" \
      -DZMK_EXTRA_MODULES="$root" >"$out/build.log" 2>&1; then
    cat "$out/build.log"
    echo "FAILED: $name (ビルドできない)"
    failed+=("$name")
    continue
  fi

  # native_posix の zmk.exe を west 経由で動かす。kscan のモックが exit-after で
  # 終わらせる。ログの "<dbg> " までを落とすのは run-test.sh と同じ
  if ! west build -d "$root/$build" -t run 2>&1 |
      sed -e "s/.*> //" |
      tee "$out/keycode_events_full.log" |
      sed -n -f "$dir/events.patterns" >"$out/keycode_events.log"; then
    cat "$out/keycode_events_full.log"
    echo "FAILED: $name (動かせない)"
    failed+=("$name")
    continue
  fi

  if diff -auZ "$dir/keycode_events.snapshot" "$out/keycode_events.log"; then
    echo "PASS: $name"
  elif [ -n "$accept" ]; then
    cp "$out/keycode_events.log" "$dir/keycode_events.snapshot"
    echo "ACCEPTED: $name"
  else
    echo "FAILED: $name"
    failed+=("$name")
  fi
done

if [ "${#failed[@]}" -gt 0 ]; then
  echo "🚨  ${#failed[@]} 件失敗。ログは output/tests/<テスト>/ の下" 1>&2
  exit 1
fi
