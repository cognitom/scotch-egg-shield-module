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

/* ベースがかなか。起動のたびに英数から始める (windmill の reset_default_layer())。
 * 英数レイヤーが上がっているかとは別で、Ctrl / Win / Alt のホールド中に英数レイヤーが
 * 上がっていても、ベースはかなのまま */
bool wm_is_kana_base(void);

/* かな / 英数のベースを入れ替え、ホストの IME もそちらへ切り替える (MY_LCTL のタップ) */
void wm_switch_lang(void);

/* ホストの IME だけを今のベースの側へ切り替える (MY_IME) */
void wm_send_lang(void);

/* 修飾 (MOD_* のビット) のホールド中だけ英数レイヤーを上げる / 下ろす。
 * 上げている修飾を1ビットずつ覚え、全部離れたら下ろす */
void wm_hold_layer_on(uint8_t mod);
void wm_hold_layer_off(uint8_t mod);

/* 接続先が Android で、mod (MOD_* のビット) が Win なら Alt、Alt なら Win が押されて
 * いるとき true。Android は Win と Alt を重ねると Caps Lock を切り替えるので、後から
 * 押したほうを出さない (src/android.c)。Win / Alt 以外は false */
bool wm_android_blocks_mod(uint8_t mod);

/* 接続先が Android で Win が押されていれば、ピリオドを Alt+. に読み替えて押し離しまで
 * 送り切り、true を返す。そうでなければ何もせず false */
bool wm_android_gui_dot(void);

/* 左右の Fn / 記号のどちらか */
#define WM_SIDE_L 1
#define WM_SIDE_R 2
#define WM_SIDE_BOTH (WM_SIDE_L | WM_SIDE_R)

/* side (WM_SIDE_*) の Fn を押した / 離した。Fn レイヤーはどちらかを押している間、
 * 設定レイヤーは両方を押している間だけ上がる (windmill の set_fn()) */
void wm_fn_set(uint8_t side, bool on);

/* side の記号を押した / 離した。両方離れるまで記号レイヤーを下ろさない
 * (windmill の set_sym()) */
void wm_sym_set(uint8_t side, bool on);

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

/* wm_send_output() と同じだが、順番が来てから wait_ms 待って送り始める。直前に
 * windmill の外で修飾を下ろしたとき (親指の組のスペースの前に Shift のホールドを
 * 下ろす) に、そのレポートと文字の間を空けるため */
int wm_send_output_after_wait(uint32_t output, bool alpha, uint8_t wait_ms);
