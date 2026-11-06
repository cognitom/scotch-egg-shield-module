/*
 * windmill のキー。&wm_char の引数に書く。
 *
 * windmill.h の独自キーコード (enum windmill_keycodes) のうち、出力表
 * (key_outputs[]) で引くもの (KB_TSU 〜 SY_QUES) を、同じ並びで 0 から数え直したもの。
 * キーマップ (devicetree) からも読むので enum ではなく数値で書く。
 *
 * キーマップには「どの文字のキーか」だけを書き、ホストへ送るキーは src/key_output.c の
 * 出力表が決める。足すときは出力表にも行を足すこと。
 */

#pragma once

/* かなレイヤー最下段のタップ。タップ・ホールドの判定は 6/9 で入れる。
 * いまは出力表の行だけで、キーマップの最下段は仮の素のキー */
#define KB_TSU 0 /* つ */
#define KB_SA 1  /* さ */
#define KB_SO 2  /* そ */
#define KB_HI 3  /* ひ */
#define KB_KO 4  /* こ */
#define KB_MI 5  /* み */
#define KB_MO 6  /* も */
#define KB_NE 7  /* ね */

/* Shift 面の最下段のタップ */
#define KB_XTSU 8    /* っ */
#define KB_SPC 9     /* 空 親指の組の半角スペース (7/9) */
#define KB_QUES 10   /* ？ 半角。英数に切り替えて送り、かなに戻す */
#define KB_TOUTEN 11 /* 、 */

/* かなレイヤー。名前は出るかな */
#define KN_NU 12   /* ぬ */
#define KN_FU 13   /* ふ */
#define KN_A 14    /* あ */
#define KN_U 15    /* う */
#define KN_E 16    /* え */
#define KN_O 17    /* お */
#define KN_YA 18   /* や */
#define KN_YU 19   /* ゆ */
#define KN_YO 20   /* よ */
#define KN_WA 21   /* わ */
#define KN_TA 22   /* た */
#define KN_TE 23   /* て */
#define KN_I 24    /* い */
#define KN_SU 25   /* す */
#define KN_KA 26   /* か */
#define KN_N 27    /* ん */
#define KN_NA 28   /* な */
#define KN_NI 29   /* に */
#define KN_RA 30   /* ら */
#define KN_SE 31   /* せ */
#define KN_DAKU 32 /* ゛ */
#define KN_CHI 33  /* ち */
#define KN_TO 34   /* と */
#define KN_SHI 35  /* し */
#define KN_HA 36   /* は */
#define KN_KI 37   /* き */
#define KN_KU 38   /* く */
#define KN_MA 39   /* ま */
#define KN_NO 40   /* の */
#define KN_RI 41   /* り */
#define KN_RE 42   /* れ */
#define KN_KE 43   /* け */
#define KN_RU 44   /* る */
#define KN_ME 45   /* め */
#define KN_RO 46   /* ろ */

/* かなの Shift 面 */
#define KN_XA 47        /* ぁ */
#define KN_XU 48        /* ぅ */
#define KN_XE 49        /* ぇ */
#define KN_XO 50        /* ぉ */
#define KN_XYA 51       /* ゃ */
#define KN_XYU 52       /* ゅ */
#define KN_XYO 53       /* ょ */
#define KN_WO 54        /* を */
#define KN_HE 55        /* へ */
#define KN_XI 56        /* ぃ */
#define KN_MU 57        /* む */
#define KN_HO 58        /* ほ */
#define KN_LKAKKO 59    /* 「 */
#define KN_RKAKKO 60    /* 」 */
#define KN_HANDAKU 61   /* ゜ */
#define KN_XTSU 62      /* っ */
#define KN_TOUTEN 63    /* 、 */
#define KN_KUTEN 64     /* 。 */
#define KN_NAKAGURO 65  /* ・ */
#define KN_CHOON 66     /* ー */

/* 記号レイヤー。英数に切り替えて送るかどうかを SY_1 〜 SY_QUES の範囲で見ているので、
 * 記号レイヤーのキーはこの間に足すこと */
#define SY_1 67
#define SY_2 68
#define SY_3 69
#define SY_4 70
#define SY_5 71
#define SY_6 72
#define SY_7 73
#define SY_8 74
#define SY_9 75
#define SY_0 76
#define SY_EXLM 77 /* ! */
#define SY_AT 78   /* @ */
#define SY_HASH 79 /* # */
#define SY_DLR 80  /* $ */
#define SY_PERC 81 /* % */
#define SY_CIRC 82 /* ^ */
#define SY_AMPR 83 /* & */
#define SY_ASTR 84 /* * */
#define SY_LPRN 85 /* ( */
#define SY_RPRN 86 /* ) */
#define SY_GRV 87  /* ` */
#define SY_EQL 88  /* = */
#define SY_PLUS 89 /* + */
#define SY_MINS 90 /* - */
#define SY_UNDS 91 /* _ */
#define SY_LBRC 92 /* [ */
#define SY_RBRC 93 /* ] */
#define SY_TILD 94 /* ~ */
#define SY_LCBR 95 /* { */
#define SY_RCBR 96 /* } */
#define SY_PIPE 97 /* | */
#define SY_QUES 98 /* ? */

/* 出力表で引くキーの範囲 */
#define WM_KEY_FIRST KB_TSU
#define WM_KEY_LAST SY_QUES
#define WM_KEY_COUNT (WM_KEY_LAST - WM_KEY_FIRST + 1)
