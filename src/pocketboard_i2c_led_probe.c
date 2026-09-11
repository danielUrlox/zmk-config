#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

#define PB_MCP_ADDR 0x20
#define PB_I2C_NODE DT_NODELABEL(i2c0)
#define PB_GPIO_NODE DT_NODELABEL(gpio0)
#define PB_BLUE_LED_PIN 15

static struct k_work_delayable pb_probe_work;
static struct k_work_delayable pb_blink_work;
static bool pb_led_on;

static void pb_set_led(bool on)
{
    const struct device *gpio = DEVICE_DT_GET(PB_GPIO_NODE);
    if (!device_is_ready(gpio)) {
        return;
    }
    /* nice!nano blue LED is active-low. */
    gpio_pin_set(gpio, PB_BLUE_LED_PIN, on ? 0 : 1);
    pb_led_on = on;
}

static void pb_blink_handler(struct k_work *work)
{
    ARG_UNUSED(work);
    pb_set_led(!pb_led_on);
    k_work_schedule(&pb_blink_work, K_MSEC(250));
}

static void pb_probe_handler(struct k_work *work)
{
    ARG_UNUSED(work);
    const struct device *i2c = DEVICE_DT_GET(PB_I2C_NODE);
    const struct device *gpio = DEVICE_DT_GET(PB_GPIO_NODE);
    uint8_t reg = 0x00; /* IODIRA */
    uint8_t value;

    if (device_is_ready(gpio)) {
        gpio_pin_configure(gpio, PB_BLUE_LED_PIN, GPIO_OUTPUT_HIGH);
        pb_set_led(false);
    }

    if (device_is_ready(i2c) &&
        i2c_write_read(i2c, PB_MCP_ADDR, &reg, 1, &value, 1) == 0) {
        /* MCP @ 0x20 FOUND: blue LED stays SOLID ON. */
        pb_set_led(true);
    } else {
        /* NO response @ 0x20: blue LED BLINKS rapidly. */
        k_work_schedule(&pb_blink_work, K_NO_WAIT);
    }
}

static int pb_probe_init(void)
{
    k_work_init_delayable(&pb_probe_work, pb_probe_handler);
    k_work_init_delayable(&pb_blink_work, pb_blink_handler);
    /* Let ZMK/USB/I2C finish starting before the one-shot test. */
    k_work_schedule(&pb_probe_work, K_SECONDS(5));
    return 0;
}

SYS_INIT(pb_probe_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
