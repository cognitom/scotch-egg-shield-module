/*
 * &wm_host の引数。windmill.h の MY_WIN / MY_ANDR / MY_JIS / MY_US にあたる。
 *
 * いまの接続先 (USB か、BLE のアクティブなプロファイル) の OS か配列を選ぶ。
 * キーマップ (devicetree) からも読むので enum ではなく数値で書く
 */

#pragma once

#define MY_WIN 0  /* 言語切替と「」の出し方を Windows/デスクトップ向けに */
#define MY_ANDR 1 /* 同じく Android 向けに */
#define MY_JIS 2  /* 接続先の配列を日本語 (JIS) に */
#define MY_US 3   /* 接続先の配列を English (US) に */

/* &wm_out の引数。押した接続先へ送り先を切り替える (geonix41 の MD_USB / MD_BLE1〜3 /
 * MD_24G の位置)。値は接続先の番号 (windmill/host_config.h の WM_HOST_*) と同じ */
#define WM_USB 0   /* USB */
#define WM_BLE_0 1 /* BLE プロファイル 0 */
#define WM_BLE_1 2 /* BLE プロファイル 1 */
#define WM_BLE_2 3 /* BLE プロファイル 2 */
#define WM_BLE_3 4 /* BLE プロファイル 3 */
