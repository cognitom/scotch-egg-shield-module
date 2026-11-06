/*
 * 接続先の OS と配列を選ぶキー (&wm_host)。windmill の MY_WIN / MY_ANDR / MY_JIS / MY_US。
 *
 * 引数は dt-bindings/windmill/host.h の MY_*。いま繋がっている接続先 (USB か、BLE の
 * アクティブなプロファイル) のぶんだけを書き換え、ほかの接続先の設定は変えない。
 * ホストへは何も送らない
 */

#define DT_DRV_COMPAT zmk_behavior_wm_host

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>

#include <dt-bindings/windmill/host.h>
#include <windmill/windmill.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata param_values[] = {
    {.display_name = "Windows", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = MY_WIN},
    {.display_name = "Android", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = MY_ANDR},
    {.display_name = "JIS", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = MY_JIS},
    {.display_name = "US", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = MY_US},
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

static int on_wm_host_binding_pressed(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d param %d", event.position, binding->param1);

    switch (binding->param1) {
    case MY_WIN:
        wm_host_set(WM_SETTING_OS, WM_OS_WIN);
        break;
    case MY_ANDR:
        wm_host_set(WM_SETTING_OS, WM_OS_ANDR);
        break;
    case MY_JIS:
        wm_host_set(WM_SETTING_LAYOUT, WM_LAYOUT_JIS);
        break;
    case MY_US:
        wm_host_set(WM_SETTING_LAYOUT, WM_LAYOUT_US);
        break;
    default:
        LOG_ERR("&wm_host の引数ではない: %d", binding->param1);
        break;
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_wm_host_binding_released(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_wm_host_driver_api = {
    .binding_pressed = on_wm_host_binding_pressed,
    .binding_released = on_wm_host_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};

#define WM_HOST_INST(n)                                                                            \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_wm_host_driver_api);

DT_INST_FOREACH_STATUS_OKAY(WM_HOST_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
