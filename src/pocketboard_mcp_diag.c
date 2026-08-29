/* PocketBoard: isolated MCP23017 B0 electrical-state test.
 *
 * No matrix scanning. No A pin is driven HIGH.
 * The test configures port A as inputs, port B as inputs,
 * disables MCP internal pull-ups on B, then reads GPIOB once.
 * It types exactly one result: "B0 LOW", "B0 HIGH", or "MCP ERROR".
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <dt-bindings/zmk/keys.h>

LOG_MODULE_REGISTER(pocketboard_mcp_diag, LOG_LEVEL_INF);

#define MCP_NODE   DT_NODELABEL(mcp23017)
#define MCP_ADDR   0x20
#define IODIRA     0x00
#define IODIRB     0x01
#define GPPUB      0x0D
#define GPIOB      0x13

#if !DT_NODE_HAS_STATUS(MCP_NODE, okay)
#error "mcp23017 node is not enabled"
#endif

static const struct device *const i2c_bus = DEVICE_DT_GET(DT_BUS(MCP_NODE));
static struct k_work_delayable test_work;

static void tap_key(uint32_t keycode) {
    raise_zmk_keycode_state_changed_from_encoded(keycode, true, k_uptime_get());
    k_msleep(18);
    raise_zmk_keycode_state_changed_from_encoded(keycode, false, k_uptime_get());
    k_msleep(22);
}

static void type_b0_low(void) {
    tap_key(B); tap_key(N0); tap_key(SPACE); tap_key(L); tap_key(O); tap_key(W);
}

static void type_b0_high(void) {
    tap_key(B); tap_key(N0); tap_key(SPACE); tap_key(H); tap_key(I); tap_key(G); tap_key(H);
}

static void type_mcp_error(void) {
    tap_key(M); tap_key(C); tap_key(P); tap_key(SPACE);
    tap_key(E); tap_key(R); tap_key(R); tap_key(O); tap_key(R);
}

static int write_reg(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return i2c_write(i2c_bus, data, sizeof(data), MCP_ADDR);
}

static int read_reg(uint8_t reg, uint8_t *value) {
    return i2c_write_read(i2c_bus, MCP_ADDR, &reg, 1, value, 1);
}

static void test_handler(struct k_work *work) {
    ARG_UNUSED(work);

    if (!device_is_ready(i2c_bus)) {
        type_mcp_error();
        return;
    }

    /* Keep every A line high-impedance: nothing on port A is driven. */
    if (write_reg(IODIRA, 0xFF) != 0 ||
        write_reg(IODIRB, 0xFF) != 0 ||
        write_reg(GPPUB,  0x00) != 0) {
        type_mcp_error();
        return;
    }

    k_msleep(20);

    uint8_t gpio_b = 0;
    if (read_reg(GPIOB, &gpio_b) != 0) {
        type_mcp_error();
        return;
    }

    LOG_INF("GPIOB=0x%02x, B0=%d", gpio_b, gpio_b & 0x01 ? 1 : 0);

    if (gpio_b & 0x01) {
        type_b0_high();
    } else {
        type_b0_low();
    }
}

static int pocketboard_diag_init(void) {
    k_work_init_delayable(&test_work, test_handler);
    k_work_schedule(&test_work, K_SECONDS(3));
    return 0;
}

SYS_INIT(pocketboard_diag_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
