/*
 * 接続先ごとの設定 (src/host_config.c) の単体テスト。windmill の tests/test_host_os.cpp と
 * tests/test_host_layout.cpp のうち、接続先を切り替えて見るものを移したもの。
 *
 * windmill のテストは windmill_board_host() を差し替えて接続先を切り替える。ここでは
 * src/host.c と同じく、ZMK の endpoint (USB か BLE のプロファイル) から
 * wm_host_from_endpoint() で接続先を引き、その接続先で設定を読み書きする。
 *
 * windmill の EEPROM は先頭の 2bit (旧形式の Android と LED の明るさ) が別の用途で、
 * 接続先ごとの設定はその後ろから並ぶ。こちらは windmill 専用の settings に置くので
 * 0bit 目から並ぶ。旧形式のビットと、配列の設定が入る前のファームウェアからの移行の
 * テスト (HostOsLegacy、HostLayoutUpgrade) は、移す先に当たるものが無いので移さない。
 * 起動し直しても残ること (HostLayoutReboot、HostLayoutBootRead) は settings の読み書きで、
 * native_posix では試せないので移さない
 */

#include <zephyr/ztest.h>

#include <dt-bindings/windmill/host.h>
#include <dt-bindings/zmk/keys.h>

#include <windmill/host_config.h>

/* 設定の格納位置。動かすと、保存してある設定が別の接続先のものとして読まれる */
#define OS_BIT(host) ((WM_SETTING_OS * WM_HOST_COUNT + (host)) * 2)
#define LAYOUT_BIT(host) ((WM_SETTING_LAYOUT * WM_HOST_COUNT + (host)) * 2)

static const struct zmk_endpoint_instance usb = {.transport = ZMK_TRANSPORT_USB};

static struct zmk_endpoint_instance ble(int profile) {
    return (struct zmk_endpoint_instance){
        .transport = ZMK_TRANSPORT_BLE,
        .ble = {.profile_index = profile},
    };
}

/* USB と BLE プロファイル 0〜3 の5つ */
static struct zmk_endpoint_instance endpoints[WM_HOST_COUNT];

static void *setup(void) {
    endpoints[0] = usb;
    for (int i = 0; i < WM_HOST_BLE_COUNT; ++i) {
        endpoints[1 + i] = ble(i);
    }
    return NULL;
}

/* src/host.c と同じ引き方 */
static uint8_t os_on(uint32_t config, struct zmk_endpoint_instance endpoint) {
    return wm_host_config_os(config, wm_host_from_endpoint(endpoint));
}

static uint8_t layout_on(uint32_t config, struct zmk_endpoint_instance endpoint) {
    return wm_host_config_layout(config, wm_host_from_endpoint(endpoint));
}

static uint32_t set_on(uint32_t config, struct zmk_endpoint_instance endpoint, uint8_t setting,
                       uint8_t value) {
    return wm_host_config_set(config, setting, wm_host_from_endpoint(endpoint), value);
}

/*
 * 接続先
 */

ZTEST_SUITE(host_endpoint, NULL, setup, NULL, NULL, NULL);

/* USB と BLE プロファイル 0〜3 が別々の接続先になる。windmill の BLE1〜3 と 2.4G の枠に
 * プロファイル 0〜3 を当てる */
ZTEST(host_endpoint, test_usb_and_four_ble_profiles) {
    zassert_equal(wm_host_from_endpoint(usb), WM_HOST_USB);
    for (int i = 0; i < WM_HOST_BLE_COUNT; ++i) {
        zassert_equal(wm_host_from_endpoint(ble(i)), 1 + i, "profile %d", i);
    }
}

/* 4つより多いプロファイルを使える設定でビルドしても、はみ出したプロファイルは USB の
 * 設定を使い、隣の設定を壊さない */
ZTEST(host_endpoint, test_profile_out_of_range_falls_back_to_usb) {
    zassert_equal(wm_host_from_endpoint(ble(WM_HOST_BLE_COUNT)), WM_HOST_USB);
    zassert_equal(wm_host_from_endpoint(ble(-1)), WM_HOST_USB);
}

/* &wm_out が切り替える先。USB は USB、WM_BLE_n は BLE プロファイル n で、
 * 切り替えた先の設定がそのキーの接続先のものとして読まれる (往復して元に戻る) */
ZTEST(host_endpoint, test_out_key_selects_its_host) {
    zassert_equal(WM_USB, WM_HOST_USB);
    zassert_equal(WM_BLE_0, WM_HOST_BLE_FIRST);
    zassert_equal(WM_BLE_3, WM_HOST_BLE_FIRST + WM_HOST_BLE_COUNT - 1);

    const struct zmk_endpoint_instance out_usb = wm_host_endpoint(WM_USB);
    zassert_equal(out_usb.transport, ZMK_TRANSPORT_USB);

    const uint8_t bles[] = {WM_BLE_0, WM_BLE_1, WM_BLE_2, WM_BLE_3};
    for (int i = 0; i < ARRAY_SIZE(bles); ++i) {
        const struct zmk_endpoint_instance out = wm_host_endpoint(bles[i]);
        zassert_equal(out.transport, ZMK_TRANSPORT_BLE, "profile %d", i);
        zassert_equal(out.ble.profile_index, i);
    }

    for (int host = 0; host < WM_HOST_COUNT; ++host) {
        zassert_equal(wm_host_from_endpoint(wm_host_endpoint(host)), host, "host %d", host);
    }
}

/* 範囲外の引数は USB へ倒し、無いプロファイルを選ばない */
ZTEST(host_endpoint, test_out_key_out_of_range_is_usb) {
    zassert_equal(wm_host_endpoint(WM_HOST_COUNT).transport, ZMK_TRANSPORT_USB);
    zassert_equal(wm_host_endpoint(0xFF).transport, ZMK_TRANSPORT_USB);
}

/*
 * OS (windmill の HostOs)
 */

ZTEST_SUITE(host_os, NULL, setup, NULL, NULL, NULL);

/* BLE プロファイル 0 で Android にしても、ほかの接続先は Windows のまま。
 * 戻ると Android に戻る (HostOs.android_is_remembered_per_host) */
ZTEST(host_os, test_android_is_remembered_per_host) {
    uint32_t config = 0;

    config = set_on(config, ble(0), WM_SETTING_OS, WM_OS_ANDR);
    zassert_equal(os_on(config, ble(0)), WM_OS_ANDR);

    zassert_equal(os_on(config, usb), WM_OS_WIN);
    for (int i = 1; i < WM_HOST_BLE_COUNT; ++i) {
        zassert_equal(os_on(config, ble(i)), WM_OS_WIN, "profile %d", i);
    }

    zassert_equal(os_on(config, ble(0)), WM_OS_ANDR);
}

/* 接続先ごとに 2bit ずつ並ぶ。4種ぶん確保してあるのは Mac / iOS を足すため
 * (HostOsEeprom.holds_two_bits_per_host) */
ZTEST(host_os, test_holds_two_bits_per_host) {
    const uint32_t config = set_on(0, ble(3), WM_SETTING_OS, WM_OS_ANDR);
    zassert_equal(config, 1u << OS_BIT(4), "config 0x%08x", config);
}

/* 範囲外の接続先は USB として扱い、隣の設定を壊さない
 * (HostOsOutOfRange.falls_back_to_usb) */
ZTEST(host_os, test_out_of_range_falls_back_to_usb) {
    const uint32_t config = wm_host_config_set(0, WM_SETTING_OS, WM_HOST_COUNT, WM_OS_ANDR);

    zassert_equal(os_on(config, usb), WM_OS_ANDR);
    zassert_equal(os_on(config, ble(3)), WM_OS_WIN); /* 最後の接続先の隣 (はみ出した先) も無事 */
    zassert_equal(wm_host_config_os(config, WM_HOST_COUNT), WM_OS_ANDR);
}

/* 全ての接続先を Android にしてから、BLE プロファイル 1 だけ Windows へ戻す
 * (HostOsWinBack.win_on_one_host_keeps_others) */
ZTEST(host_os, test_win_on_one_host_keeps_others) {
    uint32_t config = 0;
    for (int i = 0; i < WM_HOST_COUNT; ++i) {
        config = set_on(config, endpoints[i], WM_SETTING_OS, WM_OS_ANDR);
    }

    config = set_on(config, ble(1), WM_SETTING_OS, WM_OS_WIN);
    zassert_equal(os_on(config, ble(1)), WM_OS_WIN);

    for (int i = 0; i < WM_HOST_COUNT; ++i) {
        if (i == WM_HOST_BLE_FIRST + 1) {
            continue;
        }
        zassert_equal(os_on(config, endpoints[i]), WM_OS_ANDR, "host %d", i);
    }
}

/* 2bit のうち使っていない値 (2, 3。Mac / iOS 用の予約) は Windows として読む */
ZTEST(host_os, test_reserved_value_is_win) {
    const uint32_t config = (2u << OS_BIT(WM_HOST_USB)) | (3u << OS_BIT(1));
    for (int i = 0; i < WM_HOST_COUNT; ++i) {
        zassert_equal(os_on(config, endpoints[i]), WM_OS_WIN, "host %d", i);
    }
}

/*
 * 配列 (windmill の HostLayout)
 */

ZTEST_SUITE(host_layout, NULL, setup, NULL, NULL, NULL);

/* 何も保存されていなければ全ての接続先が JIS。既定のまま JIS を選んでも値は変わらない
 * (HostLayout.default_is_jis) */
ZTEST(host_layout, test_default_is_jis) {
    for (int i = 0; i < WM_HOST_COUNT; ++i) {
        zassert_equal(layout_on(0, endpoints[i]), WM_LAYOUT_JIS, "host %d", i);
    }

    const uint32_t config = set_on(0, ble(0), WM_SETTING_LAYOUT, WM_LAYOUT_JIS);
    zassert_equal(config, 0u);
}

/* BLE プロファイル 0 で US にしても、ほかの接続先は JIS のまま。書き換わるのは
 * その接続先の 2bit だけ。OS とは別々に持つので、同じ接続先で OS を変えても配列は
 * 動かない (HostLayoutPerHost.us_is_remembered_per_host) */
ZTEST(host_layout, test_us_is_remembered_per_host) {
    uint32_t config = set_on(0, ble(0), WM_SETTING_LAYOUT, WM_LAYOUT_US);

    zassert_equal(config, (uint32_t)WM_LAYOUT_US << LAYOUT_BIT(1), "config 0x%08x", config);
    zassert_equal(layout_on(config, ble(0)), WM_LAYOUT_US);
    zassert_equal(layout_on(config, usb), WM_LAYOUT_JIS);
    for (int i = 1; i < WM_HOST_BLE_COUNT; ++i) {
        zassert_equal(layout_on(config, ble(i)), WM_LAYOUT_JIS, "profile %d", i);
    }

    config = set_on(config, ble(0), WM_SETTING_OS, WM_OS_ANDR);
    zassert_equal(config, ((uint32_t)WM_LAYOUT_US << LAYOUT_BIT(1)) | (1u << OS_BIT(1)),
                  "config 0x%08x", config);
    zassert_equal(layout_on(config, ble(0)), WM_LAYOUT_US);
}

/* 全ての接続先を US にしてから、BLE プロファイル 1 だけ JIS へ戻す
 * (HostLayoutJisBack.jis_on_one_host_keeps_others) */
ZTEST(host_layout, test_jis_on_one_host_keeps_others) {
    uint32_t config = 0;
    for (int i = 0; i < WM_HOST_COUNT; ++i) {
        config = set_on(config, endpoints[i], WM_SETTING_LAYOUT, WM_LAYOUT_US);
    }
    for (int i = 0; i < WM_HOST_COUNT; ++i) {
        zassert_equal(layout_on(config, endpoints[i]), WM_LAYOUT_US, "host %d", i);
    }

    config = set_on(config, ble(1), WM_SETTING_LAYOUT, WM_LAYOUT_JIS);
    zassert_equal(layout_on(config, ble(1)), WM_LAYOUT_JIS);
    for (int i = 0; i < WM_HOST_COUNT; ++i) {
        if (i == WM_HOST_BLE_FIRST + 1) {
            continue;
        }
        zassert_equal(layout_on(config, endpoints[i]), WM_LAYOUT_US, "host %d", i);
    }
}

/* 範囲外の接続先は USB として扱い、隣の設定を壊さない
 * (HostLayoutOutOfRange.falls_back_to_usb) */
ZTEST(host_layout, test_out_of_range_falls_back_to_usb) {
    const uint32_t config =
        wm_host_config_set(0, WM_SETTING_LAYOUT, WM_HOST_COUNT, WM_LAYOUT_US);

    zassert_equal(config, (uint32_t)WM_LAYOUT_US << LAYOUT_BIT(WM_HOST_USB), "config 0x%08x",
                  config);
    zassert_equal(layout_on(config, usb), WM_LAYOUT_US);
    for (int i = 0; i < WM_HOST_BLE_COUNT; ++i) {
        zassert_equal(layout_on(config, ble(i)), WM_LAYOUT_JIS, "profile %d", i);
    }
}

/* 2bit のうち使っていない値 (2, 3) が入っていても、既定の JIS として読む
 * (HostLayoutReserved.unknown_value_is_jis) */
ZTEST(host_layout, test_unknown_value_is_jis) {
    const uint32_t config = (2u << LAYOUT_BIT(WM_HOST_USB)) | (3u << LAYOUT_BIT(1));
    for (int i = 0; i < WM_HOST_COUNT; ++i) {
        zassert_equal(layout_on(config, endpoints[i]), WM_LAYOUT_JIS, "host %d", i);
    }
}

/*
 * 言語切替のキー (windmill の HostOs が MY_LCTL のタップで見ているもの)
 */

ZTEST_SUITE(host_lang, NULL, setup, NULL, NULL, NULL);

/* Windows は LANG1 / LANG2、Android は変換 (INT4) / 無変換 (INT5) */
ZTEST(host_lang, test_lang_output_by_os) {
    zassert_equal(wm_lang_output(WM_OS_WIN, true), LANG1);
    zassert_equal(wm_lang_output(WM_OS_WIN, false), LANG2);
    zassert_equal(wm_lang_output(WM_OS_ANDR, true), INT4);
    zassert_equal(wm_lang_output(WM_OS_ANDR, false), INT5);
}

/* 言語切替のキーは、いまの接続先の OS で引く。BLE プロファイル 0 だけ Android にすると、
 * そこでだけ INT4 になる (HostOs.android_is_remembered_per_host の言語切替) */
ZTEST(host_lang, test_lang_output_follows_host) {
    const uint32_t config = set_on(0, ble(0), WM_SETTING_OS, WM_OS_ANDR);

    zassert_equal(wm_lang_output(os_on(config, ble(0)), true), INT4);
    for (int i = 0; i < WM_HOST_COUNT; ++i) {
        if (i == WM_HOST_BLE_FIRST) {
            continue;
        }
        zassert_equal(wm_lang_output(os_on(config, endpoints[i]), true), LANG1, "host %d", i);
    }
}
