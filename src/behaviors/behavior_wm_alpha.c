/*
 * 英数レイヤーの記号キー (&wm_alpha)。windmill の process_jis_alpha_symbol()。
 *
 * 引数は US 配列のキー (SEMI / SQT / BSLH など)。英数レイヤーのキーは素のキーのままで、
 * リピートも Shift もホストに任せるが、; ' \ は JIS として受け取られると別の文字が出る
 * (SQT が「:」)。接続先が JIS のときだけ横取りし、押されている Shift で送るものを決めて
 * 1キー1出力で送る (windmill の issue #80)。Shift の要否が US と食い違うので
 * wm_send_output() に打ち消させ、そのぶんリピートしない。
 *
 * それ以外 (US のとき、表に無いキー) は &kp と同じく押し離しを流す。LT のタップ側にも
 * 置けるよう、表に無いキーも受ける
 */

#define DT_DRV_COMPAT zmk_behavior_wm_alpha

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <dt-bindings/zmk/modifiers.h>

#include <zmk/behavior.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>
#include <zmk/matrix.h>

#include <windmill/windmill.h>
#include <windmill/key_outputs.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata param_values[] = {
    {
        .display_name = "Key",
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

/* 「\」に ¥ のキー (INT3) ではなく ろ のキー (INT1) を使うのは、¥ のキーは円記号を出す
 * 環境 (macOS の JIS など) があるため */
static const struct {
    uint32_t keycode; /* 英数レイヤーのキー (US) */
    uint32_t plain;   /* Shift 無しで送るもの (JIS) */
    uint32_t shifted; /* Shift 時に送るもの (JIS) */
} jis_alpha_symbols[] = {
    {SEMI, JP_SCLN, JP_COLN}, /* ; : */
    {SQT, JP_QUOT, JP_DQUO},  /* ' " */
    {BSLH, JP_BSLS, JP_PIPE}, /* \ | */
};

/* 押したときに横取りした位置。送るものは押した時点の配列で決まり、離すまでに接続先を
 * 変えられても、押していないキーを離さないよう押した側で覚える */
static bool intercepted[ZMK_KEYMAP_LEN];

static int on_wm_alpha_binding_pressed(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    const bool jis = wm_host_layout() == WM_LAYOUT_JIS;
    LOG_DBG("position %d keycode 0x%08X jis %d", event.position, binding->param1, jis);

    if (event.position < ZMK_KEYMAP_LEN) {
        intercepted[event.position] = false;
    }
    for (size_t i = 0; jis && i < ARRAY_SIZE(jis_alpha_symbols); ++i) {
        if (jis_alpha_symbols[i].keycode != binding->param1) {
            continue;
        }
        const bool shifted = zmk_hid_get_explicit_mods() & (MOD_LSFT | MOD_RSFT);
        if (event.position < ZMK_KEYMAP_LEN) {
            intercepted[event.position] = true;
        }
        wm_send_output(shifted ? jis_alpha_symbols[i].shifted : jis_alpha_symbols[i].plain,
                       false);
        return ZMK_BEHAVIOR_OPAQUE;
    }
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp);
}

static int on_wm_alpha_binding_released(struct zmk_behavior_binding *binding,
                                        struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d keycode 0x%08X", event.position, binding->param1);

    if (event.position < ZMK_KEYMAP_LEN && intercepted[event.position]) {
        /* 押したときに送り切っている */
        intercepted[event.position] = false;
        return ZMK_BEHAVIOR_OPAQUE;
    }
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp);
}

static const struct behavior_driver_api behavior_wm_alpha_driver_api = {
    .binding_pressed = on_wm_alpha_binding_pressed,
    .binding_released = on_wm_alpha_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};

#define WM_ALPHA_INST(n)                                                                           \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_wm_alpha_driver_api);

DT_INST_FOREACH_STATUS_OKAY(WM_ALPHA_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
