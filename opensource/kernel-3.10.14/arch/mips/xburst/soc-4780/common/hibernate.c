#include <linux/kernel.h>
#include <linux/reboot.h>               //extern void machine_power_off(void);
#include <linux/suspend.h>				//extern void hibernation_set_ops(const struct platform_hibernation_ops *ops);


//linux platform hibernation functions
//some of those follow functions may do nothing ,but they shouldn't be NULL
static int begin(void){
  return 0;
}
static void end(void){
  return ;
}
static int pre_snapshot(void){
  return 0;
}
static void finish(void){
  return ;
}
static int prepare(void){
  return 0;
}
static int enter(void){
  machine_power_off();  //����soc-4780/common/reset.c�е�jz_hibernate()����
  return -1;
}
static void leave(void){
  return ;
}
static int pre_restore(void){
  return 0;
}
static void restore_cleanup(void){
  return ;
}
static void recover(void){
  return ;
}


const struct platform_hibernation_ops my_hibernation_ops = {
  .begin = begin,
  .end = end,
  .pre_snapshot = pre_snapshot,
  .finish = finish,
  .prepare = prepare,
  .enter = enter,
  .leave = leave,
  .pre_restore = pre_restore,
  .restore_cleanup = restore_cleanup,
  .recover = recover,
};

  int __init my_hibernation_init(void){
	hibernation_set_ops(&my_hibernation_ops);
	return 0;
  }

arch_initcall(my_hibernation_init);