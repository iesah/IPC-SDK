#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/module.h>
//#include <linux/fs.h>
//#include <linux/string.h>
//
//#include "nand_api.h"
//#include "nm_interface.h"
//#include "nand_char.h"

extern int NM_open(void);
extern void NM_close(int handler);
extern int NM_ptInstall(int handler, char *ptname);

static int __init install_nand_partitions(void)
{
	int context = 0;
	int ret = 0;
   
	context = NM_open();
	if (!context) {
		printk("NM_open failed\n");
		return -1;
	}

	ret = NM_ptInstall(context, NULL);
	if (ret) {
		printk("NM_ptInstall failed\n");
	}
	printk("install_nand_partitions ok#############################################\n");

	NM_close(context);

	return 0;
}
module_init(install_nand_partitions);

//module_exit(nand_char_exit);
MODULE_LICENSE("GPL v2");
