/*
 * Fn と記号レイヤーの上げ下げ (windmill.c の set_fn() / set_sym() を移したもの)
 *
 * Fn と記号は左右に1つずつあり、同じレイヤーを上げる。ZMK のレイヤーは参照カウントを
 * 持たないので、&mo に任せると先に離したほうが、まだ押している側のぶんまで落とす。
 * そこで押している側を1ビットずつ覚え、レイヤーはそこから決め直す
 */

#include <zephyr/logging/log.h>

#include <zmk/keymap.h>

#include <windmill/windmill.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* 押している Fn / 記号 (WM_SIDE_*) */
static uint8_t fn_held;
static uint8_t sym_held;

static void set_layer(uint8_t layer, bool on) {
    if (on == zmk_keymap_layer_active(layer)) {
        return;
    }
    if (on) {
        zmk_keymap_layer_activate(layer);
    } else {
        zmk_keymap_layer_deactivate(layer);
    }
}

/* 設定レイヤーは左右の Fn を両方ホールドしている間だけ (windmill の issue #62)。
 * 誤って設定を変えないよう、片手では届かないようにしてある。
 *
 * 設定レイヤーの Fn の位置は透過にしておくこと。離した側を押し直したとき、下の
 * レイヤーの Fn に落ちないと Fn として押せない */
void wm_fn_set(uint8_t side, bool on) {
    fn_held = on ? (fn_held | side) : (fn_held & ~side);
    LOG_DBG("side %d on %d held %d", side, on, fn_held);

    set_layer(LAYER_FN, fn_held);
    set_layer(LAYER_CONF, fn_held == WM_SIDE_BOTH);
}

/* 押すたびに上げ、両方離れたときだけ下ろす。英数レイヤーの LT (&wm_lt) も同じ記号
 * レイヤーを &mo で上げ下げするが、windmill も LT() は QMK に任せていて同じ */
void wm_sym_set(uint8_t side, bool on) {
    sym_held = on ? (sym_held | side) : (sym_held & ~side);
    LOG_DBG("side %d on %d held %d", side, on, sym_held);

    if (on) {
        set_layer(LAYER_SYM, true);
    } else if (!sym_held) {
        set_layer(LAYER_SYM, false);
    }
}
