/*
 * レイヤー番号。windmill.h の LAYER_* と同じ並びで、キーマップの layer の並びと
 * 一致させること。
 *
 * かなの Shift 面を英数と記号より下に置くのは、Shift を押しながら英数や記号の
 * レイヤーを上げたとき、そちらのキーが勝つようにするため
 */

#pragma once

#define LAYER_KANA 0       /* かな。ホストの IME を「かな入力」にして使う */
#define LAYER_KANA_SHIFT 1 /* かなの Shift 面 */
#define LAYER_ALPHA 2      /* 英数 */
#define LAYER_SYM 3        /* 数字・記号 */
#define LAYER_FN 4         /* ファンクション・メディア */
#define LAYER_CONF 5       /* 設定 */
