#ifndef __VALUE_ADJSUT_H__
#define __VALUE_ADJUST_H__



#define TRIGGER_CNTS		(2)

extern int thr_table[TRIGGER_CNTS];



extern int adjust_trigger_value(int times_per_unit, int cur_thr);
#endif
