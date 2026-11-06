/*
 * 接続先ごとの設定 (windmill.c の「接続先ごとの設定」を移したもの) の、ZMK に繋ぐ部分。
 * 中身の読み書きは src/host_config.c。
 *
 * windmill は機種フック windmill_board_host() で接続先を引く。ここでは ZMK の endpoints が
 * 選んでいる送り先 (USB か、BLE のアクティブなプロファイル) をそのまま接続先にする
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#include <zmk/ble.h>
#include <zmk/endpoints.h>

#include <windmill/host_config.h>
#include <windmill/windmill.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* プロファイルが WM_HOST_BLE_COUNT より多いと、はみ出したぶんが USB の設定を共有して
 * しまう。シールドの .conf で CONFIG_BT_MAX_CONN / CONFIG_BT_MAX_PAIRED を絞ること */
#if IS_ENABLED(CONFIG_ZMK_BLE)
BUILD_ASSERT(ZMK_BLE_PROFILE_COUNT <= WM_HOST_BLE_COUNT,
             "BLE のプロファイルが windmill の接続先の数より多い");
#endif

/* 全ての接続先・設定ぶんを 32bit に詰めたもの (src/host_config.c)。0 が既定 */
static uint32_t host_config;

/* 接続先の切り替えを追わずに済むよう、キャッシュせず毎回引く */
static uint8_t current_host(void) { return wm_host_from_endpoint(zmk_endpoints_selected()); }

uint8_t wm_host_os(void) { return wm_host_config_os(host_config, current_host()); }

uint8_t wm_host_layout(void) { return wm_host_config_layout(host_config, current_host()); }

uint32_t wm_lang_key(bool kana) { return wm_lang_output(wm_host_os(), kana); }

/*
 * 保存
 *
 * windmill は EEPROM (eeconfig_kb) に書く。ここでは Zephyr settings の "windmill/host" に
 * 32bit のまま書く。書き込みは ZMK のほかの設定と同じく CONFIG_ZMK_SETTINGS_SAVE_DEBOUNCE
 * だけ遅らせてまとめ、続けて切り替えてもフラッシュを何度も書かない
 */

#if IS_ENABLED(CONFIG_SETTINGS)

static int host_settings_load(const char *name, size_t len, settings_read_cb read_cb,
                              void *cb_arg) {
    const char *next;
    if (!settings_name_steq(name, "host", &next) || next) {
        return -ENOENT;
    }
    /* 大きさの違うものは別の版の形なので読まず、既定のまま始める */
    if (len != sizeof(host_config)) {
        LOG_WRN("windmill/host の大きさが合わない (%zu)。既定のまま始める", len);
        return -EINVAL;
    }
    const int rc = read_cb(cb_arg, &host_config, sizeof(host_config));
    LOG_DBG("windmill/host 0x%08X", host_config);
    return MIN(rc, 0);
}

SETTINGS_STATIC_HANDLER_DEFINE(windmill, "windmill", NULL, host_settings_load, NULL, NULL);

static void host_settings_save_work(struct k_work *work) {
    const int rc = settings_save_one("windmill/host", &host_config, sizeof(host_config));
    if (rc) {
        LOG_ERR("windmill/host を保存できない: %d", rc);
    }
}

static K_WORK_DELAYABLE_DEFINE(host_save_work, host_settings_save_work);

static void host_settings_save(void) {
    k_work_reschedule(&host_save_work, K_MSEC(CONFIG_ZMK_SETTINGS_SAVE_DEBOUNCE));
}

#else

static void host_settings_save(void) {}

#endif /* IS_ENABLED(CONFIG_SETTINGS) */

/* いま繋がっている接続先のぶんだけ書き換える */
void wm_host_set(uint8_t setting, uint8_t value) {
    const uint8_t host = current_host();
    const uint32_t next = wm_host_config_set(host_config, setting, host, value);

    LOG_DBG("host %d setting %d value %d config 0x%08X", host, setting, value, next);
    if (next == host_config) {
        return; /* 無変更なら書かない */
    }
    host_config = next;
    host_settings_save();
}
