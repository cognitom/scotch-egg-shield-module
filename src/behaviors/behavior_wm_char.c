/*
 * windmill の文字キー (&wm_char)。
 *
 * windmill の「1キー1出力」: 押した瞬間に押し離しを送り切り、リピートしない。
 * いまは渡されたキーをそのまま押し離すだけで、出力表の引き当てや Shift の打ち消しは
 * これから windmill.c から移す。
 */

#define DT_DRV_COMPAT zmk_behavior_wm_char

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

/* ZMK Studio から &kp と同じくキーを選べるようにする */
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

static int on_wm_char_binding_pressed(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d keycode 0x%02X", event.position, binding->param1);

    /* 押しっぱなしでもリピートさせないため、離すのを待たずにここで離しまで送る */
    int ret = raise_zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp);
    if (ret < 0) {
        return ret;
    }
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp);
}

static int on_wm_char_binding_released(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    /* 押したときに送り切っているので、離したときに送るものは無い */
    LOG_DBG("position %d keycode 0x%02X", event.position, binding->param1);
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
