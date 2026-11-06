/*
 * 文字キーの出力表 (windmill.c の「文字キーの出力表」を移したもの)
 */

#include <zephyr/kernel.h>

#include <zmk/keymap.h>

#include <windmill/windmill.h>
#include <windmill/key_outputs.h>

/* key_output_exceptions[] で、どの OS にも当てる行 */
#define OS_ANY 0xFF

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
    uint8_t layout; /* WM_LAYOUT_* */
    uint8_t os;     /* WM_OS_* か OS_ANY */
    uint32_t output;
} key_output_exceptions[] = {
    {KN_LKAKKO, WM_LAYOUT_US, WM_OS_ANDR, LS(RBKT)}, /* 「 */
    {KN_RKAKKO, WM_LAYOUT_US, WM_OS_ANDR, LS(BSLH)}, /* 」 */
    {KN_LKAKKO, WM_LAYOUT_US, OS_ANY, LS(LBKT)},
    {KN_RKAKKO, WM_LAYOUT_US, OS_ANY, LS(RBKT)},
    {KN_HE, WM_LAYOUT_US, OS_ANY, LS(EQUAL)},    /* へ */
    {KN_MU, WM_LAYOUT_US, OS_ANY, BSLH},         /* む */
    {KN_CHOON, WM_LAYOUT_US, OS_ANY, LS(MINUS)}, /* ー */
    {KN_RO, WM_LAYOUT_US, OS_ANY, GRAVE},        /* ろ */
    {SY_AT, WM_LAYOUT_US, OS_ANY, LS(N2)},
    {SY_CIRC, WM_LAYOUT_US, OS_ANY, LS(N6)},
    {SY_AMPR, WM_LAYOUT_US, OS_ANY, LS(N7)},
    {SY_ASTR, WM_LAYOUT_US, OS_ANY, LS(N8)},
    {SY_LPRN, WM_LAYOUT_US, OS_ANY, LS(N9)},
    {SY_RPRN, WM_LAYOUT_US, OS_ANY, LS(N0)},
    {SY_GRV, WM_LAYOUT_US, OS_ANY, GRAVE},
    {SY_EQL, WM_LAYOUT_US, OS_ANY, EQUAL},
    {SY_PLUS, WM_LAYOUT_US, OS_ANY, LS(EQUAL)},
    {SY_UNDS, WM_LAYOUT_US, OS_ANY, LS(MINUS)},
    {SY_LBRC, WM_LAYOUT_US, OS_ANY, LBKT},
    {SY_RBRC, WM_LAYOUT_US, OS_ANY, RBKT},
    {SY_TILD, WM_LAYOUT_US, OS_ANY, LS(GRAVE)},
    {SY_LCBR, WM_LAYOUT_US, OS_ANY, LS(LBKT)},
    {SY_RCBR, WM_LAYOUT_US, OS_ANY, LS(RBKT)},
    {SY_PIPE, WM_LAYOUT_US, OS_ANY, LS(BSLH)},
};

bool wm_is_key(uint32_t key) { return key >= WM_KEY_FIRST && key <= WM_KEY_LAST; }

uint32_t wm_key_output(uint32_t key) {
    /* 接続先を切り替えたら次の文字からその接続先の設定で出すよう、押すたびに引く */
    const uint8_t layout = wm_host_layout();
    const uint8_t os = wm_host_os();

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
