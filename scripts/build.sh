#! /usr/bin/env bash
set -euo pipefail

# west は compose.agent.yaml の zmk サービスのものを呼ぶ。コンテナの中で動くので、
# パスはコンテナ側 (/scotch-egg) のもので渡す
cd "$(dirname "$0")/.."
root=/scotch-egg

# build.yaml (Build ワークフローの組み合わせ) と揃える
board=seeeduino_xiao_ble
shield=scotch_egg
snippet=studio-rpc-usb-uart
name="$shield-$board-zmk"
build_dir=".zmk/build/$name"

if [ ! -f .zmk/.ready ]; then
  echo "🚨  .zmk/ の準備ができていません。compose.agent.yaml の zmk サービスを起動してください" 1>&2
  exit 1
fi

# このリポジトリをモジュールとして読ませる (windmill の behavior)。
# Build ワークフローも zephyr/module.yml があれば同じように渡している。
# west は作業場所 (.zmk/) の中で呼ぶ。shim によってはホストの cwd をそのまま
# コンテナへ持ち込むので、compose の working_dir に頼らない
(cd .zmk && west build -p -s "$root/.zmk/zmk/app" -d "$root/$build_dir" -b "$board" -S "$snippet" -- \
  -DSHIELD="$shield" \
  -DZMK_CONFIG="$root/config" \
  -DZMK_EXTRA_MODULES="$root")

# ビルドの出力はマウント越しにホストの .zmk/build/ にできるので、そこから拾う
mkdir -p output
cp "$build_dir/zephyr/zmk.uf2" "output/$name.uf2"
echo "output/$name.uf2"
