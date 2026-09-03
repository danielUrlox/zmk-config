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
#define GPPUA 0x0C
#define GPPUB 0x0D
#define GPIOB 0x13
#define OLATA 0x14
static const struct device *const bus=DEVICE_DT_GET(DT_BUS(MCP_NODE));
static struct k_work_delayable work;
static bool stable[4][6], prev[4][6]; static uint8_t db[4][6];
static const uint32_t keys[4][6]={{Y,U,I,O,P,BSPC},{H,J,K,L,SEMI,SQT},{N,M,COMMA,DOT,FSLH,RSHFT},{SPACE,ENTER,BSPC,NONE,NONE,NONE}};
static int wr(uint8_t r,uint8_t v){uint8_t d[2]={r,v};return i2c_write(bus,d,2,MCP_ADDR);}
static int rd(uint8_t r,uint8_t *v){return i2c_write_read(bus,MCP_ADDR,&r,1,v,1);}
static void emit(uint32_t k,bool down){if(k!=NONE)raise_zmk_keycode_state_changed_from_encoded(k,down,k_uptime_get());}
static void scan(struct k_work *w){ARG_UNUSED(w);uint8_t rows=0;for(int c=0;c<6;c++){wr(IODIRA,0xFF);wr(OLATA,1U<<c);wr(IODIRA,(uint8_t)~(1U<<c));k_busy_wait(80);if(!rd(GPIOB,&rows)){for(int r=0;r<4;r++){bool x=(rows&(1U<<r))!=0;if(x==prev[r][c]){if(db[r][c]<3)db[r][c]++;}else{prev[r][c]=x;db[r][c]=0;}if(db[r][c]>=2&&x!=stable[r][c]){stable[r][c]=x;emit(keys[r][c],x);}}}}wr(IODIRA,0xFF);wr(OLATA,0);k_work_schedule(&work,K_MSEC(5));}
static int init(void){if(!device_is_ready(bus))return 0;wr(IODIRA,0xFF);wr(IODIRB,0xFF);wr(GPPUA,0);wr(GPPUB,0);wr(OLATA,0);k_work_init_delayable(&work,scan);k_work_schedule(&work,K_MSEC(500));return 0;}SYS_INIT(init,APPLICATION,CONFIG_APPLICATION_INIT_PRIORITY);
