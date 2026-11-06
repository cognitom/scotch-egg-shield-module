#! /usr/bin/env bash
# compose.agent.yaml の zmk サービスが起動時に回す。.zmk/ を west の作業場所にして
# zmk と zephyr などを取ってきてから、exec で入られるのを待つ。
set -euo pipefail

root=/scotch-egg
ws="$root/.zmk"

# 前回の起動で置いたものが残っていると、取り直しが終わる前に healthy になってしまう
rm -f "$ws/.ready"

# リポジトリの直下には zephyr/module.yml があり、そこを作業場所にすると west が取ってくる
# zephyr/ とぶつかる。だから .zmk/ を別の作業場所にし、マニフェストだけ写す
# (ZMK の build-user-config.yml が一時ディレクトリへ写すのと同じ理由)。
# 版の正は config/west.yml のまま
mkdir -p "$ws/config"
cp "$root/config/west.yml" "$ws/config/west.yml"

cd "$ws"
if [ ! -d .west ]; then
  west init -l config
fi
west update --fetch-opt=--filter=tree:0
west zephyr-export

touch .ready
exec sleep infinity
