/*
 * Android での Win / Alt (windmill.c の「Android での Win / Alt」を移したもの)。
 *
 * Android は Win (Meta) と Alt の押し離しの順序に反応する (windmill の issue #57)。
 *
 * - Win と Alt を重ねると、どちらかを離した時点で Caps Lock が切り替わる
 * - Win を、他のキーを挟まずに離すと、アプリ一覧が開く
 *
 * つ / さ は隣り合っているので、続けて打つだけで前者を踏む。後から出るほうを握りつぶす
 * (&wm_mod と &wm_bottom の Win / Alt)。後者は Win+. を Alt+. に読み替えるとき
 * (&wm_alpha のピリオド) に踏むので、Win を外す前と押し直した後に空打ちを挟む
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <dt-bindings/zmk/hid_usage.h>
#include <dt-bindings/zmk/hid_usage_pages.h>
#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/modifiers.h>

#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>

#include <windmill/windmill.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define GUI_MODS (MOD_LGUI | MOD_RGUI)
#define ALT_MODS (MOD_LALT | MOD_RALT)

static bool is_android(void) { return wm_host_os() == WM_OS_ANDR; }

/* 押下時の OS 設定で決める。Win / Alt 以外 (MY_LCTL の Ctrl) は握りつぶさない。
 * windmill はこの関数を Win / Alt のキーからしか呼ばないが、&wm_mod は Ctrl も通るので
 * ここで弾く */
bool wm_android_blocks_mod(uint8_t mod) {
    uint8_t other;
    if (mod & GUI_MODS) {
        other = ALT_MODS;
    } else if (mod & ALT_MODS) {
        other = GUI_MODS;
    } else {
        return false;
    }
    const bool blocked = is_android() && (zmk_hid_get_explicit_mods() & other);
    if (blocked) {
        LOG_DBG("mod 0x%02X explicit 0x%02X", mod, zmk_hid_get_explicit_mods());
    }
    return blocked;
}

static void raise_key(uint32_t keycode, bool pressed) {
    raise_zmk_keycode_state_changed_from_encoded(keycode, pressed, k_uptime_get());
}

/* 修飾のビット 1 つぶんのキー (LCTRL 〜 RGUI) */
static uint32_t mod_keycode(uint8_t bit) {
    return ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_LEFTCONTROL + bit);
}

static void raise_mods(uint8_t mods, bool pressed) {
    for (uint8_t bit = 0; bit < 8; ++bit) {
        if (mods & BIT(bit)) {
            raise_key(mod_keycode(bit), pressed);
        }
    }
}

/* 「Win を単独で押して離した」と見なされないよう、何も起こさないキーを空打ちする。
 * 右Ctrl はこのキーマップに無く、Win との組み合わせにショートカットも無い
 * (QMK の DUMMY_MOD_NEUTRALIZER_KEYCODE の推奨と同じ) */
static void neutralize_gui_tap(void) {
    raise_key(RCTRL, true);
    raise_key(RCTRL, false);
}

/* Win+. は、Windows の Win+. (絵文字) に揃えて Alt+. として送る。Win を重ねたままだと
 * Caps Lock、ただ離すとアプリ一覧になるので、離す前と押し直した後に空打ちを挟む。
 * Win を外している間にほかのキーが割り込まないよう、押下の中で送り切る。
 *
 * Alt+. は LA(DOT) の1本ではなく Alt と . を別々に押し離す。windmill (QMK の
 * tap_code16) と同じく、Alt だけのレポートを前後に挟むため */
bool wm_android_gui_dot(void) {
    const uint8_t gui = zmk_hid_get_explicit_mods() & GUI_MODS;
    if (!is_android() || !gui) {
        return false;
    }
    LOG_DBG("gui 0x%02X", gui);

    neutralize_gui_tap();
    raise_mods(gui, false);
    raise_key(LALT, true);
    raise_key(DOT, true);
    raise_key(DOT, false);
    raise_key(LALT, false);
    raise_mods(gui, true);
    neutralize_gui_tap();
    return true;
}
