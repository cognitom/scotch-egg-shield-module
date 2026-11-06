/*
 * 文字キーの出力表 (windmill.c の「文字キーの出力表」を移したもの)
 */

#include <zephyr/kernel.h>

#include <zmk/keymap.h>

#include <windmill/windmill.h>
#include <windmill/key_outputs.h>

/*
 * 接続先の設定
 *
 * 接続先ごとの OS と配列は 3/9 で入れる。それまでは windmill の既定 (Windows + JIS) に
 * 固定して引く。例外の表 (key_output_exceptions[]) は先に移しておき、引き方だけ
 * 後から差し替える
 */

#define OS_WIN 0  /* Windows/デスクトップ */
#define OS_ANDR 1 /* Android */
#define OS_ANY 0xFF

#define LAYOUT_JIS 0 /* 日本語 (JIS) */
#define LAYOUT_US 1  /* English (US) */

static uint8_t host_os(void) { return OS_WIN; }

static uint8_t host_layout(void) { return LAYOUT_JIS; }

/*
 * 出力表
 */

#define WM_KEY_OUTPUT_ROW(key, label, output) [(key) - WM_KEY_FIRST] = (output),

static const uint32_t key_outputs[] = {WM_KEY_OUTPUTS(WM_KEY_OUTPUT_ROW)};

/* 行の書き漏らしはコンパイルを通ってしまい、そのキーが何も出さなくなるだけなので、
 * せめて数を見る (中身は tests/ が全キーぶん見ている) */
BUILD_ASSERT(ARRAY_SIZE(key_outputs) == WM_KEY_COUNT,
             "key_outputs[] の行が windmill のキーの数と合っていない");

/* key_outputs[] の例外。上から見て、キー・配列・OSが当たった最初の行を使う。
 * US 配列でもかなは JISかな配列の位置で出るので、JIS と位置が違う記号のキーに
 * 載っているかなだけ行がある。「」は US のとき、Microsoft IME は [ ] の Shift、
 * Gboard は ] \ の Shift で出る */
static const struct {
    uint8_t key;
    uint8_t layout; /* LAYOUT_* */
    uint8_t os;     /* OS_* か OS_ANY */
    uint32_t output;
} key_output_exceptions[] = {
    {KN_LKAKKO, LAYOUT_US, OS_ANDR, LS(RBKT)}, /* 「 */
    {KN_RKAKKO, LAYOUT_US, OS_ANDR, LS(BSLH)}, /* 」 */
    {KN_LKAKKO, LAYOUT_US, OS_ANY, LS(LBKT)},
    {KN_RKAKKO, LAYOUT_US, OS_ANY, LS(RBKT)},
    {KN_HE, LAYOUT_US, OS_ANY, LS(EQUAL)},   /* へ */
    {KN_MU, LAYOUT_US, OS_ANY, BSLH},        /* む */
    {KN_CHOON, LAYOUT_US, OS_ANY, LS(MINUS)}, /* ー */
    {KN_RO, LAYOUT_US, OS_ANY, GRAVE},       /* ろ */
    {SY_AT, LAYOUT_US, OS_ANY, LS(N2)},
    {SY_CIRC, LAYOUT_US, OS_ANY, LS(N6)},
    {SY_AMPR, LAYOUT_US, OS_ANY, LS(N7)},
    {SY_ASTR, LAYOUT_US, OS_ANY, LS(N8)},
    {SY_LPRN, LAYOUT_US, OS_ANY, LS(N9)},
    {SY_RPRN, LAYOUT_US, OS_ANY, LS(N0)},
    {SY_GRV, LAYOUT_US, OS_ANY, GRAVE},
    {SY_EQL, LAYOUT_US, OS_ANY, EQUAL},
    {SY_PLUS, LAYOUT_US, OS_ANY, LS(EQUAL)},
    {SY_UNDS, LAYOUT_US, OS_ANY, LS(MINUS)},
    {SY_LBRC, LAYOUT_US, OS_ANY, LBKT},
    {SY_RBRC, LAYOUT_US, OS_ANY, RBKT},
    {SY_TILD, LAYOUT_US, OS_ANY, LS(GRAVE)},
    {SY_LCBR, LAYOUT_US, OS_ANY, LS(LBKT)},
    {SY_RCBR, LAYOUT_US, OS_ANY, LS(RBKT)},
    {SY_PIPE, LAYOUT_US, OS_ANY, LS(BSLH)},
};

bool wm_is_key(uint32_t key) { return key >= WM_KEY_FIRST && key <= WM_KEY_LAST; }

uint32_t wm_key_output(uint32_t key) {
    const uint8_t layout = host_layout();
    const uint8_t os = host_os();

    for (size_t i = 0; i < ARRAY_SIZE(key_output_exceptions); ++i) {
        if (key_output_exceptions[i].key != key || key_output_exceptions[i].layout != layout) {
            continue;
        }
        if (key_output_exceptions[i].os != OS_ANY && key_output_exceptions[i].os != os) {
            continue;
        }
        return key_output_exceptions[i].output;
    }
    return key_outputs[key - WM_KEY_FIRST];
}

/* windmill は「ベースレイヤーがかなか」を default_layer_state で見る。ZMK ではかなが
 * いちばん下のレイヤーで、英数はその上に重ねるので、英数が上がっていなければかなと
 * みなす。かな/英数の切り替えは 4/9 で入れる。
 * 4/9 では Ctrl / Win / Alt のホールド中にも英数レイヤーを上げるので、この判定では
 * かなベースのまま英数とみなしてしまう。そこで windmill と同じく、ベースがかなか英数かを
 * 状態として持つ形に置き換えること */
static bool is_kana_base(void) { return !zmk_keymap_layer_active(LAYER_ALPHA); }

bool wm_is_alpha_output(uint32_t key) {
    if (!is_kana_base()) {
        return false;
    }
    return (key >= SY_1 && key <= SY_QUES) || key == KB_QUES;
}
