# src/host_config.c の単体テスト (Zephyr の ztest)。ZMK の app を通さず、このファイルだけを
# native_posix_64 でビルドして動かす。BLE のプロファイルを切り替えて見たいことは
# native_posix の ZMK では試せないので、ここで関数を直接呼んで見る。
#
# scripts/test.sh が -DZMK_APP=<zmk/app> を渡す。ZMK からはヘッダー (型とキーコード) だけを読む
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(windmill_host_config_test)

set(MODULE_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)

target_include_directories(app PRIVATE ${MODULE_ROOT}/include ${ZMK_APP}/include)
target_sources(app PRIVATE src/main.c ${MODULE_ROOT}/src/host_config.c)
