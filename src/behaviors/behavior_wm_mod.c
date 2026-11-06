/*
 * ホールド中は英数レイヤーを上げる修飾キー (&wm_mod)。windmill の MY_LCTL のホールドと、
 * つ / さ のホールド、英数レイヤーの KC_LGUI / KC_LALT (process_alpha_gui_alt())。
 *
 * 引数は修飾のキー (LCTRL / LGUI / LALT など)。修飾そのものは &kp と同じく押し離しを
 * 流す。かなベースのときだけ、押している間は英数レイヤーを上げる。かな中でも Ctrl+C や
 * Win+V がかな配列のまま解決されないようにするため (windmill の issue #34)。英数ベースでは
 * レイヤーが元から上がっているので何もしない。
 *
 * Ctrl で英数レイヤーが上がった後の つ / さ は、英数レイヤーの &wm_mod になる。これも
 * 数えないと、Ctrl を先に離した時点でレイヤーが落ちる。
 *
 * windmill の update_hold_layer() は移さない。QMK はレイヤーを上げたせいで解放を別の
 * キーコードに引き直すことがあり、Ctrl はホストへ出ている修飾を見て下ろしていた。ZMK は
 * 解放を押したときのレイヤーの behavior へ送り、hold-tap もホールドに決めた behavior へ
 * 送るので、解放を待てば足りる
 */

#define DT_DRV_COMPAT zmk_behavior_wm_mod

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <dt-bindings/zmk/hid_usage.h>
#include <dt-bindings/zmk/hid_usage_pages.h>

#include <zmk/behavior.h>
#include <zmk/events/keycode_state_changed.h>

#include <windmill/windmill.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata param_values[] = {
    {
        .display_name = "Modifier",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_HID_USAGE,
    },
};

static const struct behavior_parameter_metadata_set param_metadata_set[] = {{
    .param1_values = param_values,
    .param1_values_len = ARRAY_SIZE(param_values),
}};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(param_metadata_set),
    .sets = param_metadata_set,
};

#endif

/* 修飾のキーなら MOD_* のビット、そうでなければ 0 (レイヤーは上げず &kp と同じになる) */
static uint8_t mod_bit(uint32_t keycode) {
    const uint16_t id = ZMK_HID_USAGE_ID(keycode);
    if (ZMK_HID_USAGE_PAGE(keycode) != HID_USAGE_KEY ||
        id < HID_USAGE_KEY_KEYBOARD_LEFTCONTROL || id > HID_USAGE_KEY_KEYBOARD_RIGHT_GUI) {
        return 0;
    }
    return 1 << (id - HID_USAGE_KEY_KEYBOARD_LEFTCONTROL);
}

static int on_wm_mod_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const uint8_t mod = mod_bit(binding->param1);
    LOG_DBG("position %d keycode 0x%08X kana %d", event.position, binding->param1,
            wm_is_kana_base());

    if (mod && wm_is_kana_base()) {
        wm_hold_layer_on(mod);
    }
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp);
}

static int on_wm_mod_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d keycode 0x%08X", event.position, binding->param1);

    const int ret =
        raise_zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp);
    wm_hold_layer_off(mod_bit(binding->param1));
    return ret;
}

static const struct behavior_driver_api behavior_wm_mod_driver_api = {
    .binding_pressed = on_wm_mod_binding_pressed,
    .binding_released = on_wm_mod_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};

#define WM_MOD_INST(n)                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_wm_mod_driver_api);

DT_INST_FOREACH_STATUS_OKAY(WM_MOD_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
