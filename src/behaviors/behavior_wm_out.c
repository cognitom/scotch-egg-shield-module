/*
 * 接続先を選ぶキー (&wm_out)。geonix41 の MD_USB / MD_BLE1〜3 / MD_24G の位置に置く。
 *
 * 引数は dt-bindings/windmill/host.h の WM_USB / WM_BLE_0〜3。送り先の切替
 * (&out OUT_USB / OUT_BLE) とプロファイルの選択 (&bt BT_SEL) を1キーで行う。
 *
 * &bt BT_SEL だけでは、USB が繋がっていると切り替わらない。ZMK は USB と BLE の両方が
 * 使えるとき、優先する送り先 (&out で選ぶ) のほうへ送るため。geonix41 は押した接続先へ
 * そのまま移るので、それに揃えて優先する送り先も一緒に変える。
 * 選んだプロファイルがまだ繋がっていなければ、繋がるまでは USB へ送る (ZMK の挙動のまま)
 */

#define DT_DRV_COMPAT zmk_behavior_wm_out

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>

#include <dt-bindings/windmill/host.h>
#include <windmill/host_config.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

/* キーマップの引数をそのまま接続先の番号として使う */
BUILD_ASSERT(WM_USB == WM_HOST_USB && WM_BLE_0 == WM_HOST_BLE_FIRST &&
                 WM_BLE_3 == WM_HOST_COUNT - 1,
             "&wm_out の引数が接続先の番号とずれている");

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata param_values[] = {
    {.display_name = "USB", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = WM_USB},
    {.display_name = "BLE 0", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = WM_BLE_0},
    {.display_name = "BLE 1", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = WM_BLE_1},
    {.display_name = "BLE 2", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = WM_BLE_2},
    {.display_name = "BLE 3", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = WM_BLE_3},
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

static int on_wm_out_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct zmk_endpoint_instance endpoint = wm_host_endpoint(binding->param1);
    LOG_DBG("position %d param %d transport %d", event.position, binding->param1,
            endpoint.transport);

#if IS_ENABLED(CONFIG_ZMK_BLE)
    /* 先にプロファイルを選ぶ。送り先を先に BLE へ変えると、前のプロファイルが
     * 繋がっていた場合に、そちらへ一瞬送り先が移る */
    if (endpoint.transport == ZMK_TRANSPORT_BLE) {
        const int rc = zmk_ble_prof_select(endpoint.ble.profile_index);
        if (rc < 0) {
            LOG_ERR("BLE プロファイル %d を選べない: %d", endpoint.ble.profile_index, rc);
            return rc;
        }
    }
#endif

    return zmk_endpoints_select_transport(endpoint.transport);
}

static int on_wm_out_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_wm_out_driver_api = {
    .binding_pressed = on_wm_out_binding_pressed,
    .binding_released = on_wm_out_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};

#define WM_OUT_INST(n)                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_wm_out_driver_api);

DT_INST_FOREACH_STATUS_OKAY(WM_OUT_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
