/*
 * 左右の Fn と記号レイヤーのキー (&wm_mo)。windmill の英数レイヤーの MO(LAYER_FN) と、
 * かなレイヤーの そ / ね / ひ / も のホールド。
 *
 * 引数は dt-bindings/windmill/layers.h の WM_FN_L / WM_FN_R / WM_SYM_L / WM_SYM_R。
 * windmill は左右を matrix の列で見分ける (FN_L_COL / FN_R_COL) が、ここではキーマップに
 * 左右を書く。押した位置の都合を behavior に持ち込まずに済む。
 * 上げ下げは src/layer.c
 */

#define DT_DRV_COMPAT zmk_behavior_wm_mo

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>

#include <windmill/windmill.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata param_values[] = {
    {.display_name = "Fn (Left)", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = WM_FN_L},
    {.display_name = "Fn (Right)", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = WM_FN_R},
    {.display_name = "Symbol (Left)",
     .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = WM_SYM_L},
    {.display_name = "Symbol (Right)",
     .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = WM_SYM_R},
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

static void wm_mo_set(uint32_t param, bool on) {
    switch (param) {
    case WM_FN_L:
        wm_fn_set(WM_SIDE_L, on);
        break;
    case WM_FN_R:
        wm_fn_set(WM_SIDE_R, on);
        break;
    case WM_SYM_L:
        wm_sym_set(WM_SIDE_L, on);
        break;
    case WM_SYM_R:
        wm_sym_set(WM_SIDE_R, on);
        break;
    default:
        LOG_ERR("&wm_mo の引数ではない: %d", param);
        break;
    }
}

static int on_wm_mo_binding_pressed(struct zmk_behavior_binding *binding,
                                    struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d param %d", event.position, binding->param1);
    wm_mo_set(binding->param1, true);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_wm_mo_binding_released(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d param %d", event.position, binding->param1);
    wm_mo_set(binding->param1, false);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_wm_mo_driver_api = {
    .binding_pressed = on_wm_mo_binding_pressed,
    .binding_released = on_wm_mo_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};

#define WM_MO_INST(n)                                                                              \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_wm_mo_driver_api);

DT_INST_FOREACH_STATUS_OKAY(WM_MO_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
