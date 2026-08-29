/* PocketBoard: decisive B0 direction test.
 * Drives MCP23017 B0 LOW as an output and reads it, then releases B0
 * as an input with MCP pull-up disabled and reads it again.
 * Types one of: "OUT LOW IN LOW", "OUT LOW IN HIGH",
 * "OUT HIGH IN LOW", "OUT HIGH IN HIGH", or "MCP ERROR".
 */
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <dt-bindings/zmk/keys.h>

#define MCP_NODE DT_NODELABEL(mcp23017)
#define MCP_ADDR 0x20
#define IODIRB 0x01
#define GPPUB  0x0D
#define GPIOB  0x13
#define OLATB  0x15

static const struct device *const i2c_bus = DEVICE_DT_GET(DT_BUS(MCP_NODE));
static struct k_work_delayable test_work;

static void tap(uint32_t k) {
    raise_zmk_keycode_state_changed_from_encoded(k, true, k_uptime_get());
    k_msleep(18);
    raise_zmk_keycode_state_changed_from_encoded(k, false, k_uptime_get());
    k_msleep(22);
}
static void word_out(void){ tap(O);tap(U);tap(T); }
static void word_in(void){ tap(I);tap(N); }
static void word_low(void){ tap(L);tap(O);tap(W); }
static void word_high(void){ tap(H);tap(I);tap(G);tap(H); }
static void err(void){ tap(M);tap(C);tap(P);tap(SPACE);tap(E);tap(R);tap(R);tap(O);tap(R); }

static int wr(uint8_t r,uint8_t v){ uint8_t d[2]={r,v}; return i2c_write(i2c_bus,d,2,MCP_ADDR); }
static int rd(uint8_t r,uint8_t *v){ return i2c_write_read(i2c_bus,MCP_ADDR,&r,1,v,1); }

static void test_handler(struct k_work *work) {
    ARG_UNUSED(work);
    uint8_t v=0;
    if (!device_is_ready(i2c_bus)) { err(); return; }

    /* Disable all MCP pull-ups. */
    if (wr(GPPUB,0x00)) { err(); return; }

    /* Latch B0 LOW, then make B0 an output (B1..B7 remain inputs). */
    if (wr(OLATB,0x00) || wr(IODIRB,0xFE)) { err(); return; }
    k_msleep(20);
    if (rd(GPIOB,&v)) { err(); return; }
    bool out_high = (v & 1) != 0;

    /* Release B0 back to input, still with pull-up disabled. */
    if (wr(IODIRB,0xFF)) { err(); return; }
    k_msleep(20);
    if (rd(GPIOB,&v)) { err(); return; }
    bool in_high = (v & 1) != 0;

    word_out(); tap(SPACE); out_high ? word_high() : word_low();
    tap(SPACE); word_in(); tap(SPACE); in_high ? word_high() : word_low();
}
static int init_diag(void){
    k_work_init_delayable(&test_work,test_handler);
    k_work_schedule(&test_work,K_SECONDS(3));
    return 0;
}
SYS_INIT(init_diag, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
