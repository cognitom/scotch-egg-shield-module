/*
 * かなレイヤー最下段のタップ・ホールド (&wm_bottom)。windmill.c の process_bottom_row()
 * 一式を移したもの。親指の組 (こ / み の半角スペース) は 7/9 で足す。
 *
 * 引数は dt-bindings/windmill/keys.h の KB_*。かなレイヤーには つ さ そ ひ こ み も ね
 * (KB_TSU 〜 KB_NE)、Shift 面には っ 空 ？ 、 (KB_XTSU 〜 KB_TOUTEN) を置く。
 *
 * ZMK の hold-tap はキーごとに独立して決まり、未判定のキーを複数抱えたまま押した順に
 * 出す規則を表せないので、windmill と同じく判定を自前で持つ。
 *
 * - 押してから BOTTOM_HOLD_MS たてばホールド
 * - BOTTOM_INTERRUPT_MS 以上たっていて、後から最下段以外のキーが押されればホールド
 * - 後から押した最下段のキーがタップに決まった時点で、BOTTOM_INTERRUPT_MS 以上たって
 *   いればホールド (decide_tap())
 * - 未判定のまま離せばタップ。境界はどれも「以上」
 *
 * 未判定のキーより後の position イベントは、押した順を保つため hold-tap と同じく
 * listener で捕まえて待たせ、決まったら放つ。放ったイベントは keymap がその時点の
 * レイヤーで引き直す (windmill の bottom_replay())。Shift 面の最下段は同じ位置の
 * かなレイヤーのキーと役割が同じなので、状態はかなレイヤーのキーの番号で持つ
 * (bottom_index())。
 *
 * 押下を決めるのは binding ではなく listener。押した時点のレイヤーで最下段のキーに
 * 当たる位置のイベントは、keymap へ渡さずにここで持ち、解放も位置で見分けてここで持つ。
 * keymap からこの behavior が呼ばれるのは、放ったイベントが最下段のキーに引かれた
 * ときだけで、押下を見ていないので何もしない (windmill の process_record_kb() と同じ)
 *
 * listener は hold-tap (MY_LCTL) より後、keymap より前に置く (CMakeLists.txt)。
 * windmill では QMK の tap-hold が判定を済ませたイベントだけが process_record_kb() に
 * 届くので、MY_LCTL を押してすぐ最下段のキーを押すと、MY_LCTL が先にホールドに
 * 決まり、最下段のキーは英数レイヤーで引かれる。それに揃えるため
 */

#define DT_DRV_COMPAT zmk_behavior_wm_bottom

#include <string.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/modifiers.h>

#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/keymap.h>

#include <windmill/windmill.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 1,
             "&wm_bottom は1つだけ。状態を behavior のインスタンスごとに持っていない");

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

#define WM_BOTTOM_PARAM(key, label)                                                                \
    {.display_name = label, .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = (key)}

static const struct behavior_parameter_value_metadata param_values[] = {
    WM_BOTTOM_PARAM(KB_TSU, "つ / Win"),   WM_BOTTOM_PARAM(KB_SA, "さ / Alt"),
    WM_BOTTOM_PARAM(KB_SO, "そ / Fn"),     WM_BOTTOM_PARAM(KB_HI, "ひ / 記号"),
    WM_BOTTOM_PARAM(KB_KO, "こ / Shift"),  WM_BOTTOM_PARAM(KB_MI, "み / Shift"),
    WM_BOTTOM_PARAM(KB_MO, "も / 記号"),   WM_BOTTOM_PARAM(KB_NE, "ね / Fn"),
    WM_BOTTOM_PARAM(KB_XTSU, "っ (Shift 面)"), WM_BOTTOM_PARAM(KB_SPC, "空 (Shift 面)"),
    WM_BOTTOM_PARAM(KB_QUES, "？ (Shift 面)"), WM_BOTTOM_PARAM(KB_TOUTEN, "、 (Shift 面)"),
};

static const struct behavior_parameter_metadata_set param_metadata_set[] = {{
    .param1_values = param_values,
    .param1_values_len = ARRAY_SIZE(param_values),
}};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(param_metadata_set),
    .sets = param_metadata_set,
};

#endif

#define BOTTOM_INTERRUPT_MS 150 /* これ以上たっていれば、別のキーの押下でホールドになる */
#define BOTTOM_HOLD_MS 250      /* 押したままこれだけたてばホールドになる */

#define BOTTOM_COUNT (KB_NE - KB_TSU + 1)
#define BOTTOM(key) ((key) - KB_TSU)
#define NO_BOTTOM 0xFF

static const struct device *const wm_bottom_dev = DEVICE_DT_INST_GET(0);

/*
 * ホールドの役割 (windmill の bottom_keys[])
 */

enum {
    HOLD_OS_MOD, /* Win / Alt。ホールド中は英数レイヤー */
    HOLD_FN,     /* Fn (wm_fn_set()) */
    HOLD_SYM,    /* 記号レイヤー (wm_sym_set()) */
    HOLD_SHIFT,  /* Shift。ホールド中は Shift 面 */
};

struct bottom_key {
    uint8_t hold;     /* HOLD_* */
    uint32_t keycode; /* HOLD_OS_MOD / HOLD_SHIFT: 押す修飾のキー */
    uint8_t arg;      /* HOLD_OS_MOD: 英数レイヤーを上げる MOD_*。HOLD_FN / HOLD_SYM: WM_SIDE_* */
};

static const struct bottom_key bottom_keys[] = {
    [BOTTOM(KB_TSU)] = {HOLD_OS_MOD, LGUI, MOD_LGUI},
    [BOTTOM(KB_SA)] = {HOLD_OS_MOD, LALT, MOD_LALT},
    [BOTTOM(KB_SO)] = {HOLD_FN, 0, WM_SIDE_L},
    [BOTTOM(KB_HI)] = {HOLD_SYM, 0, WM_SIDE_L},
    [BOTTOM(KB_KO)] = {HOLD_SHIFT, LSHIFT, 0},
    [BOTTOM(KB_MI)] = {HOLD_SHIFT, RSHIFT, 0},
    [BOTTOM(KB_MO)] = {HOLD_SYM, 0, WM_SIDE_R},
    [BOTTOM(KB_NE)] = {HOLD_FN, 0, WM_SIDE_R},
};
BUILD_ASSERT(ARRAY_SIZE(bottom_keys) == BOTTOM_COUNT,
             "bottom_keys[] の行が最下段のキーの数と合っていない");

/* ログ (テストの snapshot) で読めるように */
static const char *const bottom_names[] = {"つ", "さ", "そ", "ひ", "こ", "み", "も", "ね"};
BUILD_ASSERT(ARRAY_SIZE(bottom_names) == BOTTOM_COUNT);

/*
 * 状態
 */

/* 判定の結果 */
enum {
    BOTTOM_UNDECIDED,
    BOTTOM_TAP,
    BOTTOM_HOLD,
};

static const char *const decision_names[] = {"undecided", "tap", "hold"};

/* キーごとの、いちばん新しい押下の状態 */
struct bottom_state {
    uint32_t position;
    int64_t pressed_at;
    uint8_t seq;      /* 押下の通し番号。押した順の比較と、待たせている項目との対応に使う */
    uint8_t decision; /* BOTTOM_* */
    bool pressed;     /* 物理的に押されている */
    bool interrupted; /* 押してから、最下段以外のキーが押された */
};

static struct bottom_state bottom_states[BOTTOM_COUNT];
static uint8_t bottom_seq;
static uint8_t bottom_held; /* ホールドを出しているキー (1 << 番号) */

/* 待たせているイベント。押した順に並ぶ */
enum {
    ENTRY_PRESS,   /* 最下段のキーの押下。判定が決まるまで先頭で止まる */
    ENTRY_RELEASE, /* 最下段のキーの解放。ホールドを下ろす */
    ENTRY_EVENT,   /* それ以外のキーのイベント */
};

struct bottom_entry {
    uint8_t type;
    uint8_t index;    /* 最下段のキーの番号 (ENTRY_PRESS / ENTRY_RELEASE) */
    uint8_t seq;      /* ENTRY_PRESS: 押下の通し番号 */
    uint8_t decision; /* ENTRY_PRESS: 同じキーが押し直された後の、この押下の結果 */
    /* 捕まえたイベント。ENTRY_PRESS では、タップを別のキーとして出し直すときの元にする */
    struct zmk_position_state_changed_event event;
};

/* 待たせるのは未判定のキーを押してから BOTTOM_HOLD_MS の間のイベントだけなので、
 * 両手で打っても収まる。溢れたら先頭を決めて空ける */
#define BOTTOM_QUEUE_SIZE 16

static struct bottom_entry bottom_queue[BOTTOM_QUEUE_SIZE];
static uint8_t bottom_queue_len;

static void bottom_timeout(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(bottom_timer, bottom_timeout);

/*
 * キーマップの引き方
 */

/* position に今のレイヤーで当たる binding。透過は下のレイヤーへ落とす (keymap と同じ) */
static const struct zmk_behavior_binding *binding_at(uint32_t position) {
#if DT_NODE_HAS_STATUS(DT_NODELABEL(trans), okay)
    const struct device *const trans = DEVICE_DT_GET(DT_NODELABEL(trans));
#else
    const struct device *const trans = NULL;
#endif

    for (int index = ZMK_KEYMAP_LAYERS_LEN - 1; index >= 0; index--) {
        const zmk_keymap_layer_id_t layer = zmk_keymap_layer_index_to_id(index);
        if (layer == ZMK_KEYMAP_LAYER_ID_INVAL || !zmk_keymap_layer_active(layer)) {
            continue;
        }
        const struct zmk_behavior_binding *binding =
            zmk_keymap_get_layer_binding_at_idx(layer, position);
        if (!binding) {
            continue;
        }
        const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
        if (!dev || dev == trans) {
            continue;
        }
        return binding;
    }
    return NULL;
}

static bool is_bottom_binding(const struct zmk_behavior_binding *binding) {
    return binding && zmk_behavior_get_binding(binding->behavior_dev) == wm_bottom_dev;
}

/* binding が最下段のキーなら、その番号。Shift 面の 空 は こ と み の両方の位置に
 * あるので、引数だけでは決まらない。かなレイヤーの同じ位置で引き直す。
 * 最下段のキーでなければ NO_BOTTOM */
static uint8_t bottom_index(const struct zmk_behavior_binding *binding, uint32_t position) {
    if (!is_bottom_binding(binding)) {
        return NO_BOTTOM;
    }
    uint32_t key = binding->param1;
    if (key > KB_TOUTEN) {
        return NO_BOTTOM;
    }
    if (key > KB_NE) {
        const struct zmk_behavior_binding *kana =
            zmk_keymap_get_layer_binding_at_idx(LAYER_KANA, position);
        if (!is_bottom_binding(kana)) {
            return NO_BOTTOM;
        }
        key = kana->param1;
    }
    return key >= KB_TSU && key <= KB_NE ? BOTTOM(key) : NO_BOTTOM;
}

/*
 * 判定
 */

static int64_t bottom_age(const struct bottom_state *state, int64_t now) {
    return now - state->pressed_at;
}

/* 判定を決める。テストで境界を固定するため、押してからの時間を残す */
static void wm_bottom_decide(uint8_t i, uint8_t decision, int64_t now) {
    bottom_states[i].decision = decision;
    LOG_DBG("%s %s age %d", bottom_names[i], decision_names[decision],
            (int)bottom_age(&bottom_states[i], now));
}

static struct bottom_entry *find_press_entry(uint8_t i) {
    for (uint8_t n = 0; n < bottom_queue_len; ++n) {
        struct bottom_entry *entry = &bottom_queue[n];
        if (entry->type == ENTRY_PRESS && entry->index == i &&
            entry->seq == bottom_states[i].seq) {
            return entry;
        }
    }
    return NULL;
}

static uint8_t find_pressed_bottom(uint32_t position) {
    for (uint8_t i = 0; i < BOTTOM_COUNT; ++i) {
        if (bottom_states[i].pressed && bottom_states[i].position == position) {
            return i;
        }
    }
    return NO_BOTTOM;
}

/* 押し直されていなければ今の状態、押し直されていれば項目へ移した結果 */
static uint8_t entry_decision(const struct bottom_entry *entry) {
    const struct bottom_state *state = &bottom_states[entry->index];
    return state->seq == entry->seq ? state->decision : entry->decision;
}

/* 未判定のキーをタップに決め、先に押されて BOTTOM_INTERRUPT_MS 以上たった未判定の
 * キーをホールドにする。後のキーはそのホールドのレイヤーで出る (ひ を押しながら こ を
 * タップすると、記号レイヤーの「|」) */
static void decide_tap(uint8_t i, int64_t now) {
    const struct bottom_state *state = &bottom_states[i];
    wm_bottom_decide(i, BOTTOM_TAP, now);

    for (uint8_t k = 0; k < BOTTOM_COUNT; ++k) {
        const struct bottom_state *earlier = &bottom_states[k];
        if (k == i || !earlier->pressed || earlier->decision != BOTTOM_UNDECIDED) {
            continue;
        }
        if ((int8_t)(earlier->seq - state->seq) >= 0) {
            continue; /* 後から押されたもの */
        }
        if (bottom_age(earlier, now) < BOTTOM_INTERRUPT_MS) {
            continue;
        }
        wm_bottom_decide(k, BOTTOM_HOLD, now);
    }
}

/* 未判定のキーを今すぐ決める。待たせる列が溢れそうなときと、同じキーが押し直された
 * ときに使う。押していればホールド、離していればタップ */
static void force_decision(uint8_t i, int64_t now) {
    if (bottom_states[i].decision != BOTTOM_UNDECIDED) {
        return;
    }
    if (bottom_states[i].pressed) {
        wm_bottom_decide(i, BOTTOM_HOLD, now);
    } else {
        decide_tap(i, now);
    }
}

/*
 * ホールドとタップの送出
 */

static void raise_mod(uint32_t keycode, bool pressed) {
    raise_zmk_keycode_state_changed_from_encoded(keycode, pressed, k_uptime_get());
}

static void apply_hold(uint8_t i) {
    const struct bottom_key *key = &bottom_keys[i];
    switch (key->hold) {
    case HOLD_OS_MOD:
        raise_mod(key->keycode, true);
        /* &wm_mod と同じく、かなベースのときだけ英数レイヤーを上げる。最下段のキーは
         * かなレイヤーにしか無いので、いまは常にかなベース */
        if (wm_is_kana_base()) {
            wm_hold_layer_on(key->arg);
        }
        break;
    case HOLD_FN:
        wm_fn_set(key->arg, true);
        break;
    case HOLD_SYM:
        wm_sym_set(key->arg, true);
        break;
    case HOLD_SHIFT:
        raise_mod(key->keycode, true);
        zmk_keymap_layer_activate(LAYER_KANA_SHIFT);
        break;
    }
    bottom_held |= BIT(i);
}

/* Shift 面を上げている (こ / み のどちらかがホールド中) */
#define SHIFT_HELD (BIT(BOTTOM(KB_KO)) | BIT(BOTTOM(KB_MI)))

static void release_hold(uint8_t i) {
    if (!(bottom_held & BIT(i))) {
        return; /* タップだった */
    }
    bottom_held &= ~BIT(i);

    const struct bottom_key *key = &bottom_keys[i];
    switch (key->hold) {
    case HOLD_OS_MOD:
        raise_mod(key->keycode, false);
        wm_hold_layer_off(key->arg);
        break;
    case HOLD_FN:
        wm_fn_set(key->arg, false);
        break;
    case HOLD_SYM:
        wm_sym_set(key->arg, false);
        break;
    case HOLD_SHIFT:
        raise_mod(key->keycode, false);
        if (!(bottom_held & SHIFT_HELD)) {
            zmk_keymap_layer_deactivate(LAYER_KANA_SHIFT);
        }
        break;
    }
}

/* 捕まえたイベントを、この listener の次から流し直す。keymap はその時点のレイヤーで
 * 引き直す。こちらへは戻ってこない */
static void release_event(struct zmk_position_state_changed_event *event) {
    LOG_DBG("position %d state %d", event->data.position, event->data.state);
    ZMK_EVENT_RELEASE(*event);
}

/* タップを、出す時点のレイヤーで引き直して出す。記号レイヤーなど自分以外のキーに
 * 引かれたら、その位置の押下と解放を keymap へ流し、通常のキーと同じ扱いにする。
 * 空 は親指の組が成り立ったときのものなので、組にならないタップは こ / み として出す */
static void send_tap(uint8_t i, const struct bottom_entry *entry) {
    const uint32_t position = entry->event.data.position;
    const struct zmk_behavior_binding *binding = binding_at(position);

    if (bottom_index(binding, position) != i) {
        struct zmk_position_state_changed_event event = entry->event;
        event.data.state = true;
        event.data.timestamp = k_uptime_get();
        release_event(&event);

        event = entry->event;
        event.data.state = false;
        event.data.timestamp = k_uptime_get();
        release_event(&event);
        return;
    }

    uint32_t key = binding->param1;
    if (key == KB_SPC) {
        key = KB_TSU + i;
    }
    wm_send_output(wm_key_output(key), wm_is_alpha_output(key));
}

/* 先頭から、決まっているものを順に出す。未判定の押下に当たったら止まる */
static void bottom_flush(void) {
    while (bottom_queue_len) {
        struct bottom_entry entry = bottom_queue[0];
        const uint8_t decision =
            entry.type == ENTRY_PRESS ? entry_decision(&entry) : BOTTOM_UNDECIDED;
        if (entry.type == ENTRY_PRESS && decision == BOTTOM_UNDECIDED) {
            return;
        }

        --bottom_queue_len;
        memmove(&bottom_queue[0], &bottom_queue[1], bottom_queue_len * sizeof(bottom_queue[0]));

        switch (entry.type) {
        case ENTRY_PRESS:
            if (decision == BOTTOM_TAP) {
                send_tap(entry.index, &entry);
            } else {
                apply_hold(entry.index);
            }
            break;
        case ENTRY_RELEASE:
            release_hold(entry.index);
            break;
        case ENTRY_EVENT:
            release_event(&entry.event);
            break;
        }
    }
}

static void bottom_push(uint8_t type, uint8_t index,
                        const struct zmk_position_state_changed *event) {
    while (bottom_queue_len >= BOTTOM_QUEUE_SIZE) {
        /* 先頭は未判定の押下 (決まっていれば bottom_flush() が出している) */
        force_decision(bottom_queue[0].index, event->timestamp);
        bottom_flush();
    }

    struct bottom_entry *entry = &bottom_queue[bottom_queue_len++];
    *entry = (struct bottom_entry){
        .type = type,
        .index = index,
        .seq = index == NO_BOTTOM ? 0 : bottom_states[index].seq,
        .decision = BOTTOM_UNDECIDED,
        .event = copy_raised_zmk_position_state_changed(event),
    };
}

/*
 * イベントごとの判定
 */

/* 時間で決まるものを決めて、出せるものを出す。windmill はスキャンごとに回すが、
 * ここでは次に決まる時刻にタイマーを掛け (schedule_tick())、イベントの入口でも回す */
static void bottom_tick(int64_t now) {
    for (uint8_t i = 0; i < BOTTOM_COUNT; ++i) {
        const struct bottom_state *state = &bottom_states[i];
        if (!state->pressed || state->decision != BOTTOM_UNDECIDED) {
            continue;
        }

        const int64_t age = bottom_age(state, now);
        if (age >= BOTTOM_HOLD_MS || (age >= BOTTOM_INTERRUPT_MS && state->interrupted)) {
            wm_bottom_decide(i, BOTTOM_HOLD, now);
        }
    }
    bottom_flush();
}

static void schedule_tick(void) {
    int64_t next = INT64_MAX;
    for (uint8_t i = 0; i < BOTTOM_COUNT; ++i) {
        const struct bottom_state *state = &bottom_states[i];
        if (!state->pressed || state->decision != BOTTOM_UNDECIDED) {
            continue;
        }
        const int64_t at =
            state->pressed_at + (state->interrupted ? BOTTOM_INTERRUPT_MS : BOTTOM_HOLD_MS);
        next = MIN(next, at);
    }

    if (next == INT64_MAX) {
        k_work_cancel_delayable(&bottom_timer);
        return;
    }
    /* 時刻で指定する。K_MSEC() の相対指定は1ティック余分に待つので、境界ちょうどで
     * 決まらなくなる */
    k_work_reschedule(&bottom_timer, K_TIMEOUT_ABS_MS(next));
}

static void bottom_timeout(struct k_work *work) {
    bottom_tick(k_uptime_get());
    schedule_tick();
}

static void bottom_press(uint8_t i, const struct zmk_position_state_changed *event) {
    const int64_t now = event->timestamp;
    struct bottom_state *state = &bottom_states[i];

    /* 前の押下がまだ待っていれば、その結果を項目へ移してから押し直す */
    struct bottom_entry *previous = find_press_entry(i);
    if (previous) {
        force_decision(i, now);
        previous->decision = state->decision;
    }

    *state = (struct bottom_state){
        .position = event->position,
        .pressed_at = now,
        .seq = ++bottom_seq,
        .decision = BOTTOM_UNDECIDED,
        .pressed = true,
    };
    LOG_DBG("%s position %d", bottom_names[i], event->position);

    bottom_push(ENTRY_PRESS, i, event);
}

static void bottom_release(uint8_t i, const struct zmk_position_state_changed *event) {
    const int64_t now = event->timestamp;
    struct bottom_state *state = &bottom_states[i];
    state->pressed = false;

    if (state->decision == BOTTOM_UNDECIDED) {
        decide_tap(i, now);
    }

    if (bottom_queue_len) {
        bottom_push(ENTRY_RELEASE, i, event); /* 先に押されたキーより後で下ろす */
    } else {
        release_hold(i);
    }
}

/* 最下段以外のキーの押下 */
static void bottom_interrupt(int64_t now) {
    for (uint8_t i = 0; i < BOTTOM_COUNT; ++i) {
        struct bottom_state *state = &bottom_states[i];
        if (!state->pressed || state->decision != BOTTOM_UNDECIDED) {
            continue;
        }

        const int64_t age = bottom_age(state, now);
        if (age < 0) {
            continue; /* このキーより前の押下 (hold-tap で遅れて届いた MY_LCTL など) */
        }
        if (age >= BOTTOM_INTERRUPT_MS) {
            wm_bottom_decide(i, BOTTOM_HOLD, now);
        } else {
            state->interrupted = true;
        }
    }
}

/* 全ての position イベントの入口 */
static int wm_bottom_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *event = as_zmk_position_state_changed(eh);
    if (!event) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    const int64_t now = event->timestamp;
    bottom_tick(now);

    bool consumed = true;
    if (event->state) {
        const struct zmk_behavior_binding *binding = binding_at(event->position);
        if (is_bottom_binding(binding)) {
            const uint8_t i = bottom_index(binding, event->position);
            if (i != NO_BOTTOM) {
                bottom_press(i, event);
            } else {
                /* キーマップの誤り (Shift 面の最下段の下に、かなの最下段のキーが無い) */
                LOG_ERR("最下段のキーとして引けない: position %d", event->position);
            }
        } else {
            bottom_interrupt(now);
            consumed = false;
        }
    } else {
        /* 解放は位置で見分ける。押下の後でレイヤーが変わっていても、押したキーの
         * 解放として扱う */
        const uint8_t i = find_pressed_bottom(event->position);
        if (i != NO_BOTTOM) {
            bottom_release(i, event);
        } else {
            consumed = false;
        }
    }
    bottom_flush();
    schedule_tick();

    if (consumed) {
        return ZMK_EV_EVENT_HANDLED;
    }
    if (bottom_queue_len) {
        LOG_DBG("position %d state %d", event->position, event->state);
        bottom_push(ENTRY_EVENT, NO_BOTTOM, event);
        return ZMK_EV_EVENT_CAPTURED;
    }
    /* ここで流せば、keymap はホールドを出した後のレイヤーで引く */
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(behavior_wm_bottom, wm_bottom_listener);
ZMK_SUBSCRIPTION(behavior_wm_bottom, zmk_position_state_changed);

/*
 * behavior
 */

/* listener が押下を持っていくので、ここへ来るのは放ったイベントが最下段のキーに
 * 引かれたときだけ (windmill の process_record_kb() の「流し直しで最下段のキーに
 * 引かれた」)。押下を見ていないので何もしない */
static int on_wm_bottom_binding_pressed(struct zmk_behavior_binding *binding,
                                        struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d key %d", event.position, binding->param1);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_wm_bottom_binding_released(struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d key %d", event.position, binding->param1);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_wm_bottom_driver_api = {
    .binding_pressed = on_wm_bottom_binding_pressed,
    .binding_released = on_wm_bottom_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};

#define WM_BOTTOM_INST(n)                                                                          \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_wm_bottom_driver_api);

DT_INST_FOREACH_STATUS_OKAY(WM_BOTTOM_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
