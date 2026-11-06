/*
 * 文字キーの出力表の中身。windmill.c の key_outputs[] を移したもの。
 *
 * ENTRY(キー, 表示名, ホストへ送るキー) の並びで、src/key_output.c が出力表に、
 * &wm_char の ZMK Studio 向けの引数の一覧に展開する。表示名は出る文字で、
 * Studio でキーを選ぶときに見える。
 *
 * 同じ文字でも送るキーは接続先の配列で変わる (「@」は US なら Shift+2、JIS なら単独の
 * キー) ので、キーマップは「どの文字のキーか」だけを持ち、違いはここで吸収する。
 * 基準を Windows + JIS にしたのは、かなの刻印がそのまま JIS 配列の位置だから。
 * ほかの配列・OS との差分は src/key_output.c の key_output_exceptions[] が持つ。
 *
 * 値は ZMK のキーコードで書くが、JIS で出る文字を読めるよう JP_* を通す
 * (QMK の keymap_japanese.h と同じ名前)
 */

#pragma once

#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/windmill/keys.h>

/* JIS 配列で、その文字が出るキー */
#define JP_COMM COMMA
#define JP_DOT DOT
#define JP_SLSH FSLH
#define JP_SCLN SEMI
#define JP_COLN SQT
#define JP_AT LBKT
#define JP_LBRC RBKT
#define JP_RBRC NON_US_HASH
#define JP_CIRC EQUAL
#define JP_MINS MINUS
#define JP_BSLS INT1
#define JP_YEN INT3
#define JP_QUES LS(FSLH)
#define JP_QUOT LS(N7)
#define JP_DQUO LS(N2)
#define JP_EXLM LS(N1)
#define JP_HASH LS(N3)
#define JP_DLR LS(N4)
#define JP_PERC LS(N5)
#define JP_AMPR LS(N6)
#define JP_ASTR LS(SQT)
#define JP_LPRN LS(N8)
#define JP_RPRN LS(N9)
#define JP_GRV LS(LBKT)
#define JP_EQL LS(MINUS)
#define JP_PLUS LS(SEMI)
#define JP_UNDS LS(INT1)
#define JP_TILD LS(EQUAL)
#define JP_LCBR LS(RBKT)
#define JP_RCBR LS(NON_US_HASH)
#define JP_PIPE LS(INT3)

/* clang-format off */
#define WM_KEY_OUTPUTS(ENTRY)                                                                      \
    /* かなレイヤー最下段のタップ */                                                               \
    ENTRY(KB_TSU,      "つ",   Z)                                                                      \
    ENTRY(KB_SA,       "さ",   X)                                                                      \
    ENTRY(KB_SO,       "そ",   C)                                                                      \
    ENTRY(KB_HI,       "ひ",   V)                                                                      \
    ENTRY(KB_KO,       "こ",   B)                                                                      \
    ENTRY(KB_MI,       "み",   N)                                                                      \
    ENTRY(KB_MO,       "も",   M)                                                                      \
    ENTRY(KB_NE,       "ね",   JP_COMM)                                                                \
    ENTRY(KB_XTSU,     "っ",   LS(Z))                                                                  \
    ENTRY(KB_SPC,      "空",   SPACE)                                                                  \
    ENTRY(KB_QUES,     "？",   JP_QUES)                                                                \
    ENTRY(KB_TOUTEN,   "、",   LS(JP_COMM))                                                            \
    /* かな */                                                                                     \
    ENTRY(KN_NU,       "ぬ",   N1)                                                                     \
    ENTRY(KN_FU,       "ふ",   N2)                                                                     \
    ENTRY(KN_A,        "あ",   N3)                                                                     \
    ENTRY(KN_U,        "う",   N4)                                                                     \
    ENTRY(KN_E,        "え",   N5)                                                                     \
    ENTRY(KN_O,        "お",   N6)                                                                     \
    ENTRY(KN_YA,       "や",   N7)                                                                     \
    ENTRY(KN_YU,       "ゆ",   N8)                                                                     \
    ENTRY(KN_YO,       "よ",   N9)                                                                     \
    ENTRY(KN_WA,       "わ",   N0)                                                                     \
    ENTRY(KN_TA,       "た",   Q)                                                                      \
    ENTRY(KN_TE,       "て",   W)                                                                      \
    ENTRY(KN_I,        "い",   E)                                                                      \
    ENTRY(KN_SU,       "す",   R)                                                                      \
    ENTRY(KN_KA,       "か",   T)                                                                      \
    ENTRY(KN_N,        "ん",   Y)                                                                      \
    ENTRY(KN_NA,       "な",   U)                                                                      \
    ENTRY(KN_NI,       "に",   I)                                                                      \
    ENTRY(KN_RA,       "ら",   O)                                                                      \
    ENTRY(KN_SE,       "せ",   P)                                                                      \
    ENTRY(KN_DAKU,     "゛",   JP_AT)                                                                  \
    ENTRY(KN_CHI,      "ち",   A)                                                                      \
    ENTRY(KN_TO,       "と",   S)                                                                      \
    ENTRY(KN_SHI,      "し",   D)                                                                      \
    ENTRY(KN_HA,       "は",   F)                                                                      \
    ENTRY(KN_KI,       "き",   G)                                                                      \
    ENTRY(KN_KU,       "く",   H)                                                                      \
    ENTRY(KN_MA,       "ま",   J)                                                                      \
    ENTRY(KN_NO,       "の",   K)                                                                      \
    ENTRY(KN_RI,       "り",   L)                                                                      \
    ENTRY(KN_RE,       "れ",   JP_SCLN)                                                                \
    ENTRY(KN_KE,       "け",   JP_COLN)                                                                \
    ENTRY(KN_RU,       "る",   JP_DOT)                                                                 \
    ENTRY(KN_ME,       "め",   JP_SLSH)                                                                \
    ENTRY(KN_RO,       "ろ",   JP_BSLS)                                                                \
    /* かなの Shift 面。IME が同じキーの Shift で出すもの (ぁ、を など) は LS() 付き */           \
    ENTRY(KN_XA,       "ぁ",   LS(N3))                                                                 \
    ENTRY(KN_XU,       "ぅ",   LS(N4))                                                                 \
    ENTRY(KN_XE,       "ぇ",   LS(N5))                                                                 \
    ENTRY(KN_XO,       "ぉ",   LS(N6))                                                                 \
    ENTRY(KN_XYA,      "ゃ",   LS(N7))                                                                 \
    ENTRY(KN_XYU,      "ゅ",   LS(N8))                                                                 \
    ENTRY(KN_XYO,      "ょ",   LS(N9))                                                                 \
    ENTRY(KN_WO,       "を",   LS(N0))                                                                 \
    ENTRY(KN_HE,       "へ",   JP_CIRC)                                                                \
    ENTRY(KN_XI,       "ぃ",   LS(E))                                                                  \
    ENTRY(KN_MU,       "む",   JP_RBRC)                                                                \
    ENTRY(KN_HO,       "ほ",   JP_MINS)                                                                \
    ENTRY(KN_LKAKKO,   "「",   LS(JP_LBRC))                                                            \
    ENTRY(KN_RKAKKO,   "」",   LS(JP_RBRC))                                                            \
    ENTRY(KN_HANDAKU,  "゜",   JP_LBRC)                                                                \
    ENTRY(KN_XTSU,     "っ",   LS(Z))                                                                  \
    ENTRY(KN_TOUTEN,   "、",   LS(JP_COMM))                                                            \
    ENTRY(KN_KUTEN,    "。",   LS(JP_DOT))                                                             \
    ENTRY(KN_NAKAGURO, "・",   LS(JP_SLSH))                                                            \
    ENTRY(KN_CHOON,    "ー",   JP_YEN)                                                                 \
    /* 記号レイヤー */                                                                             \
    ENTRY(SY_1,        "1",    N1)                                                                     \
    ENTRY(SY_2,        "2",    N2)                                                                     \
    ENTRY(SY_3,        "3",    N3)                                                                     \
    ENTRY(SY_4,        "4",    N4)                                                                     \
    ENTRY(SY_5,        "5",    N5)                                                                     \
    ENTRY(SY_6,        "6",    N6)                                                                     \
    ENTRY(SY_7,        "7",    N7)                                                                     \
    ENTRY(SY_8,        "8",    N8)                                                                     \
    ENTRY(SY_9,        "9",    N9)                                                                     \
    ENTRY(SY_0,        "0",    N0)                                                                     \
    ENTRY(SY_EXLM,     "!",    JP_EXLM)                                                                \
    ENTRY(SY_AT,       "@",    JP_AT)                                                                  \
    ENTRY(SY_HASH,     "#",    JP_HASH)                                                                \
    ENTRY(SY_DLR,      "$",    JP_DLR)                                                                 \
    ENTRY(SY_PERC,     "%",    JP_PERC)                                                                \
    ENTRY(SY_CIRC,     "^",    JP_CIRC)                                                                \
    ENTRY(SY_AMPR,     "&",    JP_AMPR)                                                                \
    ENTRY(SY_ASTR,     "*",    JP_ASTR)                                                                \
    ENTRY(SY_LPRN,     "(",    JP_LPRN)                                                                \
    ENTRY(SY_RPRN,     ")",    JP_RPRN)                                                                \
    ENTRY(SY_GRV,      "`",    JP_GRV)                                                                 \
    ENTRY(SY_EQL,      "=",    JP_EQL)                                                                 \
    ENTRY(SY_PLUS,     "+",    JP_PLUS)                                                                \
    ENTRY(SY_MINS,     "-",    JP_MINS)                                                                \
    ENTRY(SY_UNDS,     "_",    JP_UNDS)                                                                \
    ENTRY(SY_LBRC,     "[",    JP_LBRC)                                                                \
    ENTRY(SY_RBRC,     "]",    JP_RBRC)                                                                \
    ENTRY(SY_TILD,     "~",    JP_TILD)                                                                \
    ENTRY(SY_LCBR,     "{",    JP_LCBR)                                                                \
    ENTRY(SY_RCBR,     "}",    JP_RCBR)                                                                \
    ENTRY(SY_PIPE,     "|",    JP_PIPE)                                                                \
    ENTRY(SY_QUES,     "?",    JP_QUES)
/* clang-format on */
