#ifndef __TCU_TIMER_H__
#define __TCU_TIMER_H__


#define TCU_TIMER_MS	(30)


extern unsigned long ms_to_count(unsigned long ms);
extern unsigned int tcu_timer_mod(unsigned long timer_cnt);
extern void tcu_timer_request(int tcu_chan);
extern void tcu_timer_release(int tcu_chan);
extern void tcu_timer_handler(void);
extern void tcu_timer_del(void);

#endif
