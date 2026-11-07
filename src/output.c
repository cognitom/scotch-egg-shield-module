/*
 * 文字キーの送り方 (windmill.c の send_output() を移したもの)
 *
 * windmill は Shift の上げ下げとキーの間に wait_ms(IME_WAIT_MS) を挟んで、その場で
 * 送り切る。ZMK でブロックすると、その間ほかのキーの処理もタイマーも止まるので、
 * 送るものを手順 (step) に分け、待ちはワークキューの遅延で作る。
 *
 * ZMK の behavior queue を使わないのは、Shift の打ち消しが behavior では書けないため。
 * 押されている Shift を behavior (&kp) の解放で外すと、ZMK の修飾の参照カウントを
 * 押している本人のぶんまで減らしてしまい、送っている間に本人が離すと数が合わなくなる。
 * ここでは mod-morph と同じく masked modifiers で隠す。隠している間に離されても、
 * 戻すときにはもう無いので、押しっぱなしが残らない
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <dt-bindings/zmk/hid_usage_pages.h>
#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/modifiers.h>

#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>

#include <windmill/windmill.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define SHIFT_MODS (MOD_LSFT | MOD_RSFT)

/*
 * 手順
 */

enum step_type {
    STEP_WAIT,      /* arg ミリ秒待つ */
    STEP_PRESS,     /* arg (エンコード済みキーコード) を押す */
    STEP_RELEASE,   /* arg を離す */
    STEP_HIDE_HELD, /* 押されている Shift (arg) を隠す */
    STEP_SHOW_HELD, /* 隠した Shift を戻す */
};

struct step {
    uint8_t type;
    uint32_t arg;
};

/* 英数に切り替えて送るときがいちばん長く、20 手順 */
#define MAX_STEPS 24

static struct step steps[MAX_STEPS];
static uint8_t step_count;
static uint8_t step_index;
static int64_t started_at;

static void add_step(uint8_t type, uint32_t arg) {
    __ASSERT(step_count < MAX_STEPS, "windmill の送出手順が溢れた");
    steps[step_count++] = (struct step){.type = type, .arg = arg};
}

/* Shift を外す / 足す。押されている Shift (held) があればそれを隠す / 戻し、無ければ
 * 左Shift を押す / 離す。
 *
 * Shift が要るときに押されている Shift をそのまま使うのは、左Shift へ付け替えると
 * 「右Shiftを離して左Shiftを押す」レポートが挟まり、そこだけ Shift が効かない機種が
 * あるため (windmill の issue #18) */
static void add_shift_off(uint8_t held) {
    if (held) {
        add_step(STEP_HIDE_HELD, held);
    } else {
        add_step(STEP_RELEASE, LSHIFT);
    }
}

static void add_shift_on(uint8_t held) {
    if (held) {
        add_step(STEP_SHOW_HELD, held);
    } else {
        add_step(STEP_PRESS, LSHIFT);
    }
}

/* Shift の有無を from から to へ移し、変わるときは前後にウェイトを挟む。挟まないと
 * IME が追い切れず、外した Shift が次のキーに乗ったり、足した Shift が落ちたりする
 * (windmill の issue #87) */
static void add_set_shift(bool from, bool to, uint8_t held) {
    if (from == to) {
        return;
    }
    add_step(STEP_WAIT, IME_WAIT_MS);
    if (from) {
        add_shift_off(held);
    }
    if (to) {
        add_shift_on(held);
    }
    add_step(STEP_WAIT, IME_WAIT_MS);
}

static void add_tap(uint32_t keycode) {
    add_step(STEP_PRESS, keycode);
    add_step(STEP_RELEASE, keycode);
}

/* windmill の send_output() と同じ手順を組む。押されている Shift は、組む時点
 * (前の文字を送り終えた後) のものを見る。
 *
 * alpha なら英数に切り替えて送り、かなに戻す。切り替えのキーに Shift が乗っていると
 * IME が英数/かなキーとして受け取らないので、その間は押されている Shift も外す。
 *
 * wait_first は送り始める前に待つ ms (wm_send_output_after_wait())。送出の手順に
 * 入れておくと、送り始めからの時間として間隔がログに残る */
static void wm_output_begin(uint32_t output, bool alpha, uint8_t wait_first) {
    const uint8_t held = zmk_hid_get_explicit_mods() & SHIFT_MODS;
    const bool needs_shift = SELECT_MODS(output) & MOD_LSFT;
    const uint32_t keycode = STRIP_MODS(output);

    step_count = 0;
    step_index = 0;

    if (wait_first) {
        add_step(STEP_WAIT, wait_first);
    }

    if (alpha) {
        add_set_shift(held, false, held);
        add_tap(LANG2);
        add_step(STEP_WAIT, IME_WAIT_MS); /* モードの切り替えも非同期。待たないと全角のまま入る */
        add_set_shift(false, needs_shift, held);
    } else {
        add_set_shift(held, needs_shift, held);
    }

    add_tap(keycode);

    if (alpha) {
        add_set_shift(needs_shift, false, held);
        add_step(STEP_WAIT, IME_WAIT_MS);
        add_tap(LANG1);
        add_set_shift(false, held, held);
    } else {
        add_set_shift(needs_shift, held, held);
    }

    started_at = k_uptime_get();
    LOG_DBG("output 0x%08X alpha %d held 0x%02X", output, alpha, held);
}

/* windmill 自身がキーのイベントを出している最中 */
static bool raising;

/* 1 手順を実行する。テストで送出の間隔を固定するため、送り始めからの時間と、
 * そのときホストへ出るレポートの修飾を残す */
static void wm_output_step(const struct step *step) {
    const int elapsed = (int)(k_uptime_get() - started_at);

    switch (step->type) {
    case STEP_PRESS:
    case STEP_RELEASE:
        LOG_DBG("+%dms %s 0x%08X", elapsed, step->type == STEP_PRESS ? "press" : "release",
                step->arg);
        /* 目印。順番待ちの間に外から来たキーを捕まえる listener が、これは通す */
        raising = true;
        raise_zmk_keycode_state_changed_from_encoded(step->arg, step->type == STEP_PRESS,
                                                     k_uptime_get());
        raising = false;
        break;
    case STEP_HIDE_HELD:
    case STEP_SHOW_HELD:
        if (step->type == STEP_HIDE_HELD) {
            zmk_hid_masked_modifiers_set(step->arg);
        } else {
            zmk_hid_masked_modifiers_clear();
        }
        zmk_endpoints_send_report(HID_USAGE_KEY);
        LOG_DBG("+%dms %s 0x%02X mods 0x%02X", elapsed,
                step->type == STEP_HIDE_HELD ? "hide" : "show", step->arg,
                zmk_hid_get_keyboard_report()->body.modifiers);
        break;
    }
}

/*
 * 順番待ち
 *
 * windmill はその場で送り切るので、文字を送っている間に押された素のキー (&kp の
 * Enter、BSpc、矢印、修飾) が文字を追い越すことは無い。ここでは待ちの間にも
 * ほかのキーの処理が進むので、送出待ちがある間に windmill の外から来たキーの
 * イベント (keycode_state_changed) は捕まえて文字と同じ列に並べ、押された順に出す。
 * 捕まえずに通すと、かなを打ってすぐの Enter が先に確定したり、Shift を隠している
 * 間のキーが Shift 抜けで出たりする
 */

enum pending_type {
    PENDING_OUTPUT, /* 文字 */
    PENDING_EVENT,  /* 捕まえたキーのイベント */
};

struct pending {
    uint8_t type;
    union {
        struct {
            uint32_t output;
            bool alpha;
            uint8_t wait_first;
        };
        struct zmk_keycode_state_changed_event event;
    };
};

/* 1文字ぶんは長くても 100ms ほどなので、両手で打っても溢れない */
#define PENDING_SIZE 32

static struct pending pendings[PENDING_SIZE];
static uint8_t pending_head;
static uint8_t pending_len;
static bool sending;

static void run(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(output_work, run);

static struct pending *push_pending(void) {
    if (pending_len >= PENDING_SIZE) {
        return NULL;
    }
    return &pendings[(pending_head + pending_len++) % PENDING_SIZE];
}

/* 捕まえたイベントを、捕まえた listener の次から流し直す。こちらへは戻ってこない */
static void wm_output_release(struct pending *pending) {
    LOG_DBG("keycode 0x%02X state %d", pending->event.data.keycode, pending->event.data.state);
    ZMK_EVENT_RELEASE(pending->event);
}

static void run(struct k_work *work) {
    for (;;) {
        if (!sending) {
            if (!pending_len) {
                return;
            }
            struct pending next = pendings[pending_head];
            pending_head = (pending_head + 1) % PENDING_SIZE;
            pending_len--;

            if (next.type == PENDING_EVENT) {
                wm_output_release(&next);
                continue;
            }
            wm_output_begin(next.output, next.alpha, next.wait_first);
            sending = true;
        }

        while (step_index < step_count) {
            const struct step *step = &steps[step_index++];
            if (step->type == STEP_WAIT) {
                k_work_schedule(&output_work, K_MSEC(step->arg));
                return;
            }
            wm_output_step(step);
        }
        sending = false;
    }
}

int wm_send_output(uint32_t output, bool alpha) {
    return wm_send_output_after_wait(output, alpha, 0);
}

int wm_send_output_after_wait(uint32_t output, bool alpha, uint8_t wait_ms) {
    struct pending *pending = push_pending();
    if (!pending) {
        LOG_WRN("windmill の送出待ちが溢れたので 0x%08X を捨てる", output);
        return -ENOMEM;
    }
    *pending = (struct pending){
        .type = PENDING_OUTPUT,
        .output = output,
        .alpha = alpha,
        .wait_first = wait_ms,
    };

    /* 待ちの途中ならその遅延のまま (k_work_schedule は予約済みなら何もしない)。
     * 止まっていれば今すぐ送り始める */
    k_work_schedule(&output_work, K_NO_WAIT);
    return 0;
}

/* 送出待ちがある間だけ、windmill の外から来たキーのイベントを捕まえる。
 * windmill 自身が出すイベント (raising の間) は通す */
static int wm_output_hold_back(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (!ev || raising || (!sending && !pending_len)) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    struct pending *pending = push_pending();
    if (!pending) {
        /* 捨てると押しっぱなしや離し忘れになるので、順番が崩れても通す */
        LOG_WRN("windmill の送出待ちが溢れたので、キーを待たせずに通す");
        return ZMK_EV_EVENT_BUBBLE;
    }
    pending->type = PENDING_EVENT;
    pending->event = copy_raised_zmk_keycode_state_changed(ev);

    LOG_DBG("keycode 0x%02X state %d", ev->keycode, ev->state);
    return ZMK_EV_EVENT_CAPTURED;
}

ZMK_LISTENER(windmill_output, wm_output_hold_back);
ZMK_SUBSCRIPTION(windmill_output, zmk_keycode_state_changed);
