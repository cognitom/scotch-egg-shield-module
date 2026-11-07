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
#define LAYER_CONF 5       /* 設定。左右の Fn を両方ホールドしている間だけ */

/* &wm_mo の引数。Fn と記号レイヤーは左右に1つずつあり、どちらを押しているかを
 * 1ビットずつ覚える (windmill の set_fn() / set_sym())。ZMK の &mo はレイヤーの
 * 参照カウントを持たず、先に離したほうがまだ押している側のぶんまで落とすため */
#define WM_FN_L 0  /* 左の Fn */
#define WM_FN_R 1  /* 右の Fn */
#define WM_SYM_L 2 /* 左の記号 */
#define WM_SYM_R 3 /* 右の記号 */
