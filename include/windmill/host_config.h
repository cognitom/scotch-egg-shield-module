/*
 * 接続先ごとの設定 (windmill.c の「接続先ごとの設定」を移したもの) のうち、ZMK の
 * 状態に触らない部分。
 *
 * BLE のプロファイルを切り替えて見るテストは native_posix では回せない (BLE が無い) ので、
 * 接続先の決め方と設定の読み書きはここに切り出し、tests/unit/host-config で単体テストする。
 * そのため ZMK のヘッダーは型とキーコードの定義だけを読む
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <zmk/endpoints_types.h>

/* 接続先の番号。windmill の WINDMILL_HOST_* と同じ並びで、設定の格納位置になるので
 * 並びを変えないこと。windmill の 2.4G (WINDMILL_HOST_2P4G) の枠は BLE プロファイル 3 に
 * 当てる。scotch-egg に 2.4G は無く、4つ目の無線の接続先として同じ枠を使えば、
 * windmill と設定の並びがずれない */
#define WM_HOST_USB 0
#define WM_HOST_BLE_FIRST 1 /* BLE プロファイル 0。以下プロファイル n が 1 + n */
#define WM_HOST_BLE_COUNT 4
#define WM_HOST_COUNT (WM_HOST_BLE_FIRST + WM_HOST_BLE_COUNT)

/* 接続先の OS。言語切替と「」は、IME の実装で要るキーが違う */
#define WM_OS_WIN 0  /* Windows/デスクトップ */
#define WM_OS_ANDR 1 /* Android */
/* 2, 3 は予約 (Mac / iOS 用)。読めても Windows と同じに扱う */

/* 接続先のキーボード配列。相手の OS がこのキーボードをどの配列として受け取るか */
#define WM_LAYOUT_JIS 0 /* 日本語 (JIS) */
#define WM_LAYOUT_US 1  /* English (US) */

/* 設定の種類。設定は接続先ごと・種類ごとに 2bit ずつ、
 * (種類 * WM_HOST_COUNT + 接続先) * 2 bit 目から並ぶ。0 を既定 (Windows / JIS) にして
 * あるのは、何も保存されていない機体が 0 のまま起動するから */
#define WM_SETTING_OS 0
#define WM_SETTING_LAYOUT 1
#define WM_SETTING_COUNT 2

/* 選ばれている endpoint の接続先。範囲外の BLE プロファイルは USB とみなす */
uint8_t wm_host_from_endpoint(struct zmk_endpoint_instance endpoint);

/* config の中の、host の OS (WM_OS_*) と配列 (WM_LAYOUT_*)。予約の値は既定に倒す。
 * 範囲外の host は USB として読む */
uint8_t wm_host_config_os(uint32_t config, uint8_t host);
uint8_t wm_host_config_layout(uint32_t config, uint8_t host);

/* config の host の setting (WM_SETTING_*) だけを value に書き換えたものを返す。
 * 範囲外の host は USB として書き、隣の接続先を壊さない */
uint32_t wm_host_config_set(uint32_t config, uint8_t setting, uint8_t host, uint8_t value);

/* ホストの IME を、kana ならかな入力へ、そうでなければ英数へ切り替えるキー
 * (ZMK のエンコード済みキーコード) */
uint32_t wm_lang_output(uint8_t os, bool kana);
