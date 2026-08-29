/* PocketBoard: direct I2C presence test for MCP23017 at address 0x20.
 *
 * This test does NOT scan the matrix and does NOT use MCP GPIO pins.
 * It performs one raw I2C register read after boot:
 *   success -> types "MCP OK"
 *   failure -> types "MCP ERROR"
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <errno.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <dt-bindings/zmk/keys.h>

LOG_MODULE_REGISTER(pocketboard_mcp_diag, LOG_LEVEL_INF);

#define MCP_NODE DT_NODELABEL(mcp23017)
#define MCP_ADDR 0x20
#define MCP_IODIRA 0x00

#if !DT_NODE_HAS_STATUS(MCP_NODE, okay)
#error "mcp23017 node is not enabled"
#endif

static const struct device *const i2c_bus = DEVICE_DT_GET(DT_BUS(MCP_NODE));
static struct k_work_delayable test_work;

static void tap_key(uint32_t keycode) {
    int64_t now = k_uptime_get();
    raise_zmk_keycode_state_changed_from_encoded(keycode, true, now);
    k_msleep(18);
    raise_zmk_keycode_state_changed_from_encoded(keycode, false, k_uptime_get());
    k_msleep(22);
}

static void type_mcp_ok(void) {
    tap_key(M);
    tap_key(C);
    tap_key(P);
    tap_key(SPACE);
    tap_key(O);
    tap_key(K);
}

static void type_mcp_error(void) {
    tap_key(M);
    tap_key(C);
    tap_key(P);
    tap_key(SPACE);
    tap_key(E);
    tap_key(R);
    tap_key(R);
    tap_key(O);
    tap_key(R);
}

static void test_handler(struct k_work *work) {
    ARG_UNUSED(work);

    if (!device_is_ready(i2c_bus)) {
        LOG_ERR("I2C bus is not ready");
        type_mcp_error();
        return;
    }

    uint8_t reg = MCP_IODIRA;
    uint8_t value = 0;

    int ret = i2c_write_read(i2c_bus, MCP_ADDR, &reg, 1, &value, 1);

    if (ret == 0) {
        LOG_INF("MCP23017 ACK at 0x20; IODIRA=0x%02x", value);
        type_mcp_ok();
    } else {
        LOG_ERR("MCP23017 no response at 0x20: %d", ret);
        type_mcp_error();
    }
}

static int pocketboard_diag_init(void) {
    k_work_init_delayable(&test_work, test_handler);
    k_work_schedule(&test_work, K_SECONDS(3));
    LOG_INF("PocketBoard MCP23017 raw I2C test armed");
    return 0;
}

SYS_INIT(pocketboard_diag_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
