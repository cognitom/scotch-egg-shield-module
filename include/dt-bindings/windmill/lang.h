/*
 * &wm_lang の引数。windmill の MY_LCTL のタップと MY_IME にあたる。
 * キーマップ (devicetree) からも読むので enum ではなく数値で書く
 */

#pragma once

#define MY_LANG 0 /* かな / 英数のベースを入れ替え、ホストの IME もそちらへ切り替える */
#define MY_IME 1  /* ホストの IME だけを今のベースの側へ切り替える (ベースは動かさない) */
