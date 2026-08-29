/*
 * PocketBoard MCP23017 progressive diagnostic.
 *
 * The normal ZMK kscan scans ONLY the left half.
 * This diagnostic module scans the MCP23017 directly and progressively:
 *
 * Stage 0: A0 x B0
 * Stage 1: A0-A1 x B0
 * Stage 2: A0-A2 x B0
 * Stage 3: A0-A3 x B0
 * Stage 4: A0-A4 x B0
 * Stage 5: A0-A5 x B0
 * Stage 6: A0-A5 x B0-B1
 * Stage 7: A0-A5 x B0-B2
 * Stage 8: A0-A5 x B0-B3
 *
 * Press ANY of the three short-row keys on the left (positions 18,19,20)
 * to advance one stage.
 *
 * Right-side detections are emitted as ONE quick tap on transition only,
 * so a stuck/ghost line will not flood the PC with repeated characters.
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/keycode_state_changed.h>

#include <dt-bindings/zmk/keys.h>

LOG_MODULE_REGISTER(pocketboard_mcp_diag, LOG_LEVEL_INF);

#define MCP_NODE DT_NODELABEL(mcp23017)

#if !DT_NODE_HAS_STATUS(MCP_NODE, okay)
#error "mcp23017 node is not enabled"
#endif

static const struct device *const mcp = DEVICE_DT_GET(MCP_NODE);

static struct k_work_delayable scan_work;

static uint8_t stage;
static bool previous_state[4][6];

/* Easy-to-recognize diagnostic symbols in Notepad. */
static const uint32_t diag_keys[4][6] = {
    {N1, N2, N3, N4, N5, N6},
    {Q,  W,  E,  R,  T,  Y},
    {A,  S,  D,  F,  G,  H},
    {Z,  X,  C,  V,  B,  N},
};

static uint8_t active_cols(void) {
    if(stage <= 5) {
        return stage + 1;
    }
    return 6;
}

static uint8_t active_rows(void) {
    if(stage <= 5) {
        return 1;
    }
    return stage - 4; /* stage 6=>2, 7=>3, 8=>4 */
}

static void release_all_columns(void) {
    for(int c = 0; c < 6; c++) {
        gpio_pin_set(mcp, c, 0);
    }
}

static void tap_diag_key(uint32_t encoded) {
    int64_t now = k_uptime_get();
    raise_zmk_keycode_state_changed_from_encoded(encoded, true, now);
    raise_zmk_keycode_state_changed_from_encoded(encoded, false, now + 1);
}

static void scan_handler(struct k_work *work) {
    ARG_UNUSED(work);

    if(!device_is_ready(mcp)) {
        k_work_reschedule(&scan_work, K_MSEC(100));
        return;
    }

    uint8_t cols = active_cols();
    uint8_t rows = active_rows();

    /* Lines outside the current stage are deliberately ignored. */
    for(int r = 0; r < 4; r++) {
        for(int c = 0; c < 6; c++) {
            if(r >= rows || c >= cols) {
                previous_state[r][c] = false;
            }
        }
    }

    for(int c = 0; c < cols; c++) {
        release_all_columns();
        gpio_pin_set(mcp, c, 1);
        k_busy_wait(100);

        for(int r = 0; r < rows; r++) {
            int value = gpio_pin_get(mcp, 8 + r);
            bool pressed = value > 0;

            if(pressed && !previous_state[r][c]) {
                LOG_INF("STAGE %u detected A%u x B%u", stage, c, r);
                tap_diag_key(diag_keys[r][c]);
            }

            previous_state[r][c] = pressed;
        }
    }

    release_all_columns();
    k_work_reschedule(&scan_work, K_MSEC(20));
}

static void advance_stage(void) {
    if(stage < 8) {
        stage++;
        memset(previous_state, 0, sizeof(previous_state));
        LOG_INF("Diagnostic stage -> %u (cols=%u rows=%u)",
                stage, active_cols(), active_rows());
    } else {
        LOG_INF("Diagnostic already at full 6x4 matrix");
    }
}

static int position_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);

    if(!ev) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    /* The three keys in the short fourth row of the left half. */
    if(ev->state && ev->position >= 18 && ev->position <= 20) {
        advance_stage();
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(pocketboard_diag_listener, position_listener);
ZMK_SUBSCRIPTION(pocketboard_diag_listener, zmk_position_state_changed);

static int pocketboard_diag_init(void) {
    if(!device_is_ready(mcp)) {
        LOG_ERR("MCP23017 not ready");
        return -ENODEV;
    }

    /* GPA0..GPA5 = matrix columns, always controlled by this tester. */
    for(int c = 0; c < 6; c++) {
        int ret = gpio_pin_configure(mcp, c, GPIO_OUTPUT_INACTIVE);
        if(ret < 0) {
            LOG_ERR("Failed to configure A%d: %d", c, ret);
            return ret;
        }
    }

    /*
     * GPB0..GPB3 = matrix rows.
     * External 10 kOhm pull-down resistors are expected from B0..B3 to GND.
     */
    for(int r = 0; r < 4; r++) {
        int ret = gpio_pin_configure(mcp, 8 + r, GPIO_INPUT);
        if(ret < 0) {
            LOG_ERR("Failed to configure B%d: %d", r, ret);
            return ret;
        }
    }

    stage = 0;
    memset(previous_state, 0, sizeof(previous_state));

    k_work_init_delayable(&scan_work, scan_handler);
    k_work_schedule(&scan_work, K_MSEC(300));

    LOG_INF("PocketBoard progressive MCP diagnostic started: A0 x B0");
    return 0;
}

SYS_INIT(pocketboard_diag_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
