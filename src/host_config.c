/*
 * 接続先ごとの設定の、ZMK の状態に触らない部分 (include/windmill/host_config.h 参照)。
 * tests/unit/host-config が ZMK 抜きでこのファイルだけをビルドして試すので、
 * ここでは ZMK と Zephyr の関数を呼ばないこと
 */

#include <dt-bindings/zmk/keys.h>

#include <windmill/host_config.h>

#define HOST_BITS 2
#define HOST_MASK ((1u << HOST_BITS) - 1)

/* 設定は 32bit に詰めて保存するので、足すときは収まるかを見る */
_Static_assert(WM_SETTING_COUNT * WM_HOST_COUNT * HOST_BITS <= 32,
               "接続先ごとの設定が 32bit に収まっていない");

uint8_t wm_host_from_endpoint(struct zmk_endpoint_instance endpoint) {
    if (endpoint.transport != ZMK_TRANSPORT_BLE) {
        return WM_HOST_USB;
    }
    /* BLE のプロファイルを WM_HOST_BLE_COUNT より多く使える設定でビルドしても、
     * 隣の設定を壊さないよう USB の設定を使う */
    if (endpoint.ble.profile_index < 0 || endpoint.ble.profile_index >= WM_HOST_BLE_COUNT) {
        return WM_HOST_USB;
    }
    return WM_HOST_BLE_FIRST + endpoint.ble.profile_index;
}

static uint8_t host_shift(uint8_t setting, uint8_t host) {
    if (host >= WM_HOST_COUNT) {
        host = WM_HOST_USB;
    }
    return (setting * WM_HOST_COUNT + host) * HOST_BITS;
}

static uint8_t host_config_get(uint32_t config, uint8_t setting, uint8_t host) {
    return (config >> host_shift(setting, host)) & HOST_MASK;
}

uint8_t wm_host_config_os(uint32_t config, uint8_t host) {
    const uint8_t os = host_config_get(config, WM_SETTING_OS, host);
    return os == WM_OS_ANDR ? WM_OS_ANDR : WM_OS_WIN;
}

uint8_t wm_host_config_layout(uint32_t config, uint8_t host) {
    const uint8_t layout = host_config_get(config, WM_SETTING_LAYOUT, host);
    return layout == WM_LAYOUT_US ? WM_LAYOUT_US : WM_LAYOUT_JIS;
}

uint32_t wm_host_config_set(uint32_t config, uint8_t setting, uint8_t host, uint8_t value) {
    const uint8_t shift = host_shift(setting, host);
    return (config & ~((uint32_t)HOST_MASK << shift)) | ((uint32_t)(value & HOST_MASK) << shift);
}

/* Windows は LANG1 / LANG2 でモードを直接指定する。トグルのキーと違い、IME 側がずれても
 * 押し直せば揃う。
 *
 * Android は日本語へは変換 (INT4)、英語へは無変換 (INT5) を送り、
 * cognitom/windmill-android のユーザー補助サービスが受けて IME を直接指定する
 * (windmill の issue #105)。LANG1 / LANG2 は記号の一時的な英数への切り替え
 * (src/output.c) に使っていて、アプリが横取りするとそれが壊れるので使わない */
uint32_t wm_lang_output(uint8_t os, bool kana) {
    if (os == WM_OS_ANDR) {
        return kana ? INT4 : INT5;
    }
    return kana ? LANG1 : LANG2;
}
