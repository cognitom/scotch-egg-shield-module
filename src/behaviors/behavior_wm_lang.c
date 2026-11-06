/*
 * 言語切替のキー (&wm_lang)。windmill の MY_LCTL のタップと MY_IME。
 *
 * 引数は dt-bindings/windmill/lang.h の MY_*。MY_LCTL のホールド (Ctrl + 英数レイヤー) は
 * &wm_mod の役目で、タップとホールドの判定は ZMK の hold-tap (&wm_lctl) に任せる
 */

#define DT_DRV_COMPAT zmk_behavior_wm_lang

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>

#include <dt-bindings/windmill/lang.h>
#include <windmill/windmill.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata param_values[] = {
    {.display_name = "Toggle", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = MY_LANG},
    {.display_name = "IME only", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = MY_IME},
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

static int on_wm_lang_binding_pressed(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d param %d", event.position, binding->param1);

    switch (binding->param1) {
    case MY_LANG:
        wm_switch_lang();
        break;
    case MY_IME:
        wm_send_lang();
        break;
    default:
        LOG_ERR("&wm_lang の引数ではない: %d", binding->param1);
        break;
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_wm_lang_binding_released(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    /* 押したときに送り切っているので、離したときに送るものは無い */
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_wm_lang_driver_api = {
    .binding_pressed = on_wm_lang_binding_pressed,
    .binding_released = on_wm_lang_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};

#define WM_LANG_INST(n)                                                                            \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_wm_lang_driver_api);

DT_INST_FOREACH_STATUS_OKAY(WM_LANG_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
