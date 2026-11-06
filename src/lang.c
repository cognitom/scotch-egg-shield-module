/*
 * かな / 英数のベースと、修飾のホールド中の英数レイヤー (windmill.c の「起動時の
 * ベースレイヤー」「修飾キーのホールド中だけ英数レイヤーへ移す」と switch_lang() を
 * 移したもの)
 *
 * windmill はかなと英数をどちらもベースレイヤー (default_layer_state) に置き、ホールド中の
 * 英数は別に layer_on() する。ZMK にはベースレイヤーの入れ替えが無く、かながいちばん下の
 * レイヤーで、英数はその上に重ねる。英数ベースもホールド中の英数も同じ LAYER_ALPHA を
 * 上げることになり、ZMK のレイヤーは参照カウントを持たないので、片方の都合で下ろすと
 * もう片方のぶんまで落ちる。そこでベースがかなかを状態として持ち、英数レイヤーを
 * 上げるかは「英数ベースか、ホールド中の修飾があるか」から毎回決め直す
 */

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/events/keycode_state_changed.h>
#include <zmk/keymap.h>

#include <windmill/windmill.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* 英数/かなは保存せず、起動のたびに英数から始める (windmill の issue #22) */
static bool kana_base;

/* 英数レイヤーを上げている修飾 (MOD_* のビット)。重ね押しで先に離したほうが、まだ
 * 押している側のぶんまで落とさないよう、1ビットずつ覚える (windmill の issue #34) */
static uint8_t hold_layer_mods;

static void update_alpha_layer(void) {
    const bool on = !kana_base || hold_layer_mods;
    if (on == zmk_keymap_layer_active(LAYER_ALPHA)) {
        return;
    }
    if (on) {
        zmk_keymap_layer_activate(LAYER_ALPHA);
    } else {
        zmk_keymap_layer_deactivate(LAYER_ALPHA);
    }
}

bool wm_is_kana_base(void) { return kana_base; }

/* ホストの IME を kana の側へ切り替える。windmill の tap_code() と同じく Shift は
 * 打ち消さない。送出待ちの文字があれば、このイベントは src/output.c が捕まえて
 * 文字の後に並べるので、先に打ったかなが英数で入ることは無い */
static void send_lang_to_host(bool kana) {
    const uint32_t keycode = wm_lang_key(kana);
    const int64_t now = k_uptime_get();

    LOG_DBG("kana %d keycode 0x%08X", kana, keycode);
    raise_zmk_keycode_state_changed_from_encoded(keycode, true, now);
    raise_zmk_keycode_state_changed_from_encoded(keycode, false, now);
}

/* キーボードはホストの IME の状態を読めないので、ベースを自分で反転させて追う */
void wm_switch_lang(void) {
    kana_base = !kana_base;
    send_lang_to_host(kana_base);
    update_alpha_layer();
}

/* 入力欄を移るなどして IME だけがずれたとき、キーボードを動かさずに合わせ直すため。
 * モードを直接指定するので、揃っている状態で押しても何も起きない */
void wm_send_lang(void) { send_lang_to_host(kana_base); }

void wm_hold_layer_on(uint8_t mod) {
    hold_layer_mods |= mod;
    update_alpha_layer();
}

void wm_hold_layer_off(uint8_t mod) {
    /* 上げていない修飾の解放 (英数ベースで押した Win など) なら何もしない */
    if (!(hold_layer_mods & mod)) {
        return;
    }
    hold_layer_mods &= ~mod;
    update_alpha_layer();
}

static int wm_lang_init(void) {
    update_alpha_layer();
    return 0;
}

SYS_INIT(wm_lang_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
