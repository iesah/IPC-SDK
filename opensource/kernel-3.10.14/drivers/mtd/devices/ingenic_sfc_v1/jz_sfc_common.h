#ifndef JZ_SFC_COMMON_H
#define jz_SFC_COMMON_H
#include <linux/platform_device.h>
#include <mach/sfc_nand.h>
#include <mach/sfc_register.h>

void dump_sfc_reg(struct sfc *sfc);

void sfc_message_init(struct sfc_message *m);
void sfc_message_add_tail(struct sfc_transfer *t, struct sfc_message *m);
void sfc_transfer_del(struct sfc_transfer *t);
int sfc_sync(struct sfc *sfc, struct sfc_message *message);
struct sfc *sfc_res_init(struct platform_device *pdev);
void sfc_res_deinit(struct sfc *sfc);
unsigned int sfc_get_sta_rt(struct sfc *sfc);

int set_flash_timing(struct sfc *sfc, unsigned int t_hold, unsigned int t_setup, unsigned int t_shslrd, unsigned int t_shslwr);

int sfc_nor_get_special_ops(struct sfc_flash *flash);

#endif


