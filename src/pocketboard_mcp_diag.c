/*
 * PocketBoard MCP23017 one-shot electrical diagnostic.
 *
 * Goal: determine objectively what B0 does when A0 is LOW vs HIGH,
 * without continuously scanning the matrix.
 *
 * Result is emitted ONCE after boot:
 *   1 = B0 LOW  with A0 LOW, and LOW  with A0 HIGH
 *   2 = B0 LOW  with A0 LOW, and HIGH with A0 HIGH
 *   3 = B0 HIGH with A0 LOW, and HIGH with A0 HIGH
 *   4 = B0 HIGH with A0 LOW, and LOW  with A0 HIGH
 *
 * After that, this module does nothing.
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <errno.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <dt-bindings/zmk/keys.h>

LOG_MODULE_REGISTER(pocketboard_mcp_diag, LOG_LEVEL_INF);

#define MCP_NODE DT_NODELABEL(mcp23017)

#if !DT_NODE_HAS_STATUS(MCP_NODE, okay)
#error "mcp23017 node is not enabled"
#endif

static const struct device *const mcp = DEVICE_DT_GET(MCP_NODE);
static struct k_work_delayable test_work;

static void send_result_once(uint32_t encoded) {
    int64_t now = k_uptime_get();
    raise_zmk_keycode_state_changed_from_encoded(encoded, true, now);
    k_msleep(30);
    raise_zmk_keycode_state_changed_from_encoded(encoded, false, k_uptime_get());
}

static void test_handler(struct k_work *work) {
    ARG_UNUSED(work);

    if (!device_is_ready(mcp)) {
        LOG_ERR("MCP23017 not ready");
        return;
    }

    /* Keep every matrix column LOW first. */
    for (int c = 0; c < 6; c++) {
        gpio_pin_set(mcp, c, 0);
    }

    k_msleep(20);
    int b0_when_a0_low = gpio_pin_get(mcp, 8) > 0;

    /* Drive ONLY A0 HIGH. */
    gpio_pin_set(mcp, 0, 1);
    k_msleep(20);
    int b0_when_a0_high = gpio_pin_get(mcp, 8) > 0;

    /* Return A0 LOW and stop. */
    gpio_pin_set(mcp, 0, 0);

    uint32_t result_key;
    if (!b0_when_a0_low && !b0_when_a0_high) {
        result_key = N1;
    } else if (!b0_when_a0_low && b0_when_a0_high) {
        result_key = N2;
    } else if (b0_when_a0_low && b0_when_a0_high) {
        result_key = N3;
    } else {
        result_key = N4;
    }

    LOG_INF("ONE-SHOT A0/B0: A0_LOW->B0=%d, A0_HIGH->B0=%d",
            b0_when_a0_low, b0_when_a0_high);

    send_result_once(result_key);
}

static int pocketboard_diag_init(void) {
    if (!device_is_ready(mcp)) {
        LOG_ERR("MCP23017 not ready");
        return -ENODEV;
    }

    /* GPA0..GPA5 are outputs and start LOW. */
    for (int c = 0; c < 6; c++) {
        int ret = gpio_pin_configure(mcp, c, GPIO_OUTPUT_INACTIVE);
        if (ret < 0) {
            LOG_ERR("Failed to configure A%d: %d", c, ret);
            return ret;
        }
    }

    /* GPB0 only. The external 10 kOhm pull-down remains connected. */
    int ret = gpio_pin_configure(mcp, 8, GPIO_INPUT);
    if (ret < 0) {
        LOG_ERR("Failed to configure B0: %d", ret);
        return ret;
    }

    k_work_init_delayable(&test_work, test_handler);
    k_work_schedule(&test_work, K_SECONDS(2));

    LOG_INF("PocketBoard one-shot A0/B0 diagnostic armed");
    return 0;
}

SYS_INIT(pocketboard_diag_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
