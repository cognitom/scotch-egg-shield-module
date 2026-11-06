/*
 * windmill のキー処理の、behavior どうしで共有するもの
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <dt-bindings/windmill/keys.h>
#include <dt-bindings/windmill/layers.h>

/* IME は修飾の上げ下げも英数/かなの切り替えも非同期に処理するので、レポートを続けて
 * 送ると前のぶんを取りこぼす (windmill の issue #36)。効かなければ増やし、もたつくなら減らす */
#define IME_WAIT_MS 10

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
