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
