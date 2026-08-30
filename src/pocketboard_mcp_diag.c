/* PocketBoard: read B0..B3 with A0..A7 high impedance and MCP pull-ups OFF.
 * Types once: B0H B1L B2H B3L (example).
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
#define IODIRA 0x00
#define IODIRB 0x01
#define GPPUA  0x0C
#define GPPUB  0x0D
#define GPIOB  0x13

static const struct device *const bus=DEVICE_DT_GET(DT_BUS(MCP_NODE));
static struct k_work_delayable work;

static void tap(uint32_t k){
 raise_zmk_keycode_state_changed_from_encoded(k,true,k_uptime_get()); k_msleep(18);
 raise_zmk_keycode_state_changed_from_encoded(k,false,k_uptime_get()); k_msleep(22);
}
static int wr(uint8_t r,uint8_t v){uint8_t d[2]={r,v};return i2c_write(bus,d,2,MCP_ADDR);}
static int rd(uint8_t r,uint8_t *v){return i2c_write_read(bus,MCP_ADDR,&r,1,v,1);}
static void err(void){tap(M);tap(C);tap(P);tap(SPACE);tap(E);tap(R);tap(R);tap(O);tap(R);}
static void result(int n,bool high){
 tap(B);
 switch(n){case 0:tap(N0);break;case 1:tap(N1);break;case 2:tap(N2);break;default:tap(N3);}
 tap(high?H:L);
}
static void handler(struct k_work *w){
 ARG_UNUSED(w); uint8_t v=0;
 if(!device_is_ready(bus)||wr(IODIRA,0xFF)||wr(IODIRB,0xFF)||wr(GPPUA,0x00)||wr(GPPUB,0x00)){
   err(); return;
 }
 k_msleep(30);
 if(rd(GPIOB,&v)){err();return;}
 for(int i=0;i<4;i++){ if(i)tap(SPACE); result(i,(v&(1<<i))!=0); }
}
static int init_diag(void){k_work_init_delayable(&work,handler);k_work_schedule(&work,K_SECONDS(3));return 0;}
SYS_INIT(init_diag,APPLICATION,CONFIG_APPLICATION_INIT_PRIORITY);
