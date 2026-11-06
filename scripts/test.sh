#! /usr/bin/env bash
set -euo pipefail

# west は compose.agent.yaml の zmk サービスのものを呼ぶ (build.sh 参照)
cd "$(dirname "$0")/.."
root=/scotch-egg

if [ ! -f .zmk/.ready ]; then
  echo "🚨  .zmk/ の準備ができていません。compose.agent.yaml の zmk サービスを起動してください" 1>&2
  exit 1
fi

# `west test` は ZMK の run-test.sh を回す。tests/ の下の native_posix_64.keymap を
# 1件ずつ native_posix_64 向けにビルドして動かし、keycode_events.snapshot と比べる。
# このリポジトリのモジュールは ZMK_EXTRA_MODULES で読ませる
# (run-test.sh が環境変数から読むので、コンテナの中で立てる)
if ! west test "$root/tests"; then
  echo "🚨  失敗したテストのログは .zmk/zmk/app/build/tests/ の下にあります" 1>&2
  exit 1
fi
