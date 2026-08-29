/* PocketBoard execution test: type "POCKETBOARD OK" once after boot.
 * This deliberately does NOT access the MCP23017 or the right matrix.
 */

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <dt-bindings/zmk/keys.h>

LOG_MODULE_REGISTER(pocketboard_exec_test, LOG_LEVEL_INF);

static struct k_work_delayable test_work;

static void tap(uint32_t keycode) {
    raise_zmk_keycode_state_changed_from_encoded(keycode, true, k_uptime_get());
    k_msleep(35);
    raise_zmk_keycode_state_changed_from_encoded(keycode, false, k_uptime_get());
    k_msleep(35);
}

static void test_handler(struct k_work *work) {
    ARG_UNUSED(work);

    const uint32_t text[] = {
        P, O, C, K, E, T, B, O, A, R, D,
        SPACE,
        O, K
    };

    for (size_t i = 0; i < ARRAY_SIZE(text); i++) {
        tap(text[i]);
    }

    LOG_INF("PocketBoard execution test completed");
}

static int pocketboard_exec_test_init(void) {
    k_work_init_delayable(&test_work, test_handler);
    k_work_schedule(&test_work, K_SECONDS(3));
    LOG_INF("PocketBoard execution test armed");
    return 0;
}

SYS_INIT(pocketboard_exec_test_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
