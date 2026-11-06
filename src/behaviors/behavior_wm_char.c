/*
 * windmill の文字キー (&wm_char)。
 *
 * 引数は windmill のキー (dt-bindings/windmill/keys.h の KN_* / SY_* / KB_*)。
 * ホストへ送るキーは出力表 (src/key_output.c) で引く。
 *
 * windmill の「1キー1出力」: 押した瞬間に押し離しを送り切り、リピートしない。
 * 押されている Shift は打ち消し、要るときだけ足す (src/output.c)。
 */

#define DT_DRV_COMPAT zmk_behavior_wm_char

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>

#include <windmill/windmill.h>
#include <windmill/key_outputs.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

/* ZMK Studio で、出る文字の名前からキーを選べるようにする */
#define WM_KEY_PARAM_VALUE(key, label, output)                                                     \
    {                                                                                              \
        .display_name = label,                                                                     \
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,                                               \
        .value = (key),                                                                            \
    },

static const struct behavior_parameter_value_metadata param_values[] = {
    WM_KEY_OUTPUTS(WM_KEY_PARAM_VALUE)};

static const struct behavior_parameter_metadata_set param_metadata_set[] = {{
    .param1_values = param_values,
    .param1_values_len = ARRAY_SIZE(param_values),
}};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(param_metadata_set),
    .sets = param_metadata_set,
};

#endif

static int on_wm_char_binding_pressed(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d key %d", event.position, binding->param1);

    if (!wm_is_key(binding->param1)) {
        LOG_ERR("windmill のキーではない: %d", binding->param1);
        return ZMK_BEHAVIOR_OPAQUE;
    }

    /* 英数に切り替えるかは、押した時点のレイヤーで決める。送るのは前の文字を
     * 送り終えた後になることがあり、その間にレイヤーが変わりうる */
    return wm_send_output(wm_key_output(binding->param1), wm_is_alpha_output(binding->param1));
}

static int on_wm_char_binding_released(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    /* 押したときに送り切っているので、離したときに送るものは無い */
    LOG_DBG("position %d key %d", event.position, binding->param1);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_wm_char_driver_api = {
    .binding_pressed = on_wm_char_binding_pressed,
    .binding_released = on_wm_char_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};

#define WM_CHAR_INST(n)                                                                            \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_wm_char_driver_api);

DT_INST_FOREACH_STATUS_OKAY(WM_CHAR_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
