#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pocketboard_i2c_probe, LOG_LEVEL_INF);

#define PB_I2C_NODE DT_NODELABEL(i2c0)
#define PB_MCP_ADDR  0x20

static void pocketboard_probe_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(pocketboard_probe_work, pocketboard_probe_handler);

static void pocketboard_probe_handler(struct k_work *work)
{
    const struct device *i2c = DEVICE_DT_GET(PB_I2C_NODE);
    uint8_t reg = 0x00; /* IODIRA on MCP23017 */
    uint8_t value = 0x00;

    if (!device_is_ready(i2c)) {
        LOG_ERR("POCKETBOARD: I2C0 NOT READY");
    } else {
        int rc = i2c_write_read(i2c, PB_MCP_ADDR, &reg, sizeof(reg),
                                &value, sizeof(value));
        if (rc == 0) {
            LOG_INF("POCKETBOARD: MCP23017 FOUND @ 0x20 (IODIRA=0x%02X)", value);
        } else {
            LOG_ERR("POCKETBOARD: NO RESPONSE @ 0x20 (rc=%d)", rc);
        }
    }

    /* Repeat so the result is easy to catch in the USB serial monitor. */
    k_work_schedule(&pocketboard_probe_work, K_SECONDS(3));
}

static int pocketboard_probe_init(void)
{
    /* USB logging starts a few seconds after boot; probe after that. */
    k_work_schedule(&pocketboard_probe_work, K_SECONDS(7));
    return 0;
}

SYS_INIT(pocketboard_probe_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
