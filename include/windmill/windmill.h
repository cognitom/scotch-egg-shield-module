/*
 * windmill のキー処理の、behavior どうしで共有するもの
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <dt-bindings/windmill/keys.h>
#include <dt-bindings/windmill/layers.h>
#include <windmill/host_config.h>

/* IME は修飾の上げ下げも英数/かなの切り替えも非同期に処理するので、レポートを続けて
 * 送ると前のぶんを取りこぼす (windmill の issue #36)。効かなければ増やし、もたつくなら減らす */
#define IME_WAIT_MS 10

/* いま繋がっている接続先 (USB か、BLE のアクティブなプロファイル) の OS (WM_OS_*) と
 * 配列 (WM_LAYOUT_*)。接続先の切り替えを追わずに済むよう、呼ぶたびに引く */
uint8_t wm_host_os(void);
uint8_t wm_host_layout(void);

/* いま繋がっている接続先の setting (WM_SETTING_*) を value にして保存する */
void wm_host_set(uint8_t setting, uint8_t value);

/* ホストの IME を、kana ならかな入力へ、そうでなければ英数へ切り替えるキー。
 * いま繋がっている接続先の OS で決まる */
uint32_t wm_lang_key(bool kana);

/* 出力表で引くキー (WM_KEY_FIRST 〜 WM_KEY_LAST) なら true */
bool wm_is_key(uint32_t key);

/* ホストへ送るキー (ZMK のエンコード済みキーコード)。Shift の要るものは LS() 付き。
 * key は wm_is_key() を満たすこと */
uint32_t wm_key_output(uint32_t key);

/* 英数に切り替えて送る文字か。記号レイヤーの数字・記号と、Shift 面の半角「？」は
 * かな入力のままでは打てない。押した時点のレイヤーで決まる */
bool wm_is_alpha_output(uint32_t key);

/* 文字を1つ、押し離しまで送り切る (1キー1出力)。押されている Shift は打ち消し、
 * output に Shift が要るときだけ足す。alpha なら英数に切り替えて送り、かなに戻す。
 *
 * 間に IME_WAIT_MS を挟むのでブロックせず、順番待ちに積んで返る。前の文字を送り
 * 終えてから次を送るので、続けて押しても混ざらない */
int wm_send_output(uint32_t output, bool alpha);
