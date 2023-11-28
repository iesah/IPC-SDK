#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/proc_fs.h>
#include <soc/base.h>
#include <linux/seq_file.h>
#include <mach/jz_efuse.h>
#include <jz_proc.h>

#define EFUSE_IOBASE	0x134100d0
#define DRV_NAME		"jz-efuse"

#define EFUSE_CTRL				0xc
#define HI_WEN		(0x1 < 1)
#define LO_WEN		(0x1 < 0)

#define CMD_READ		100
#define CMD_WRITE		101


#define CMD_DEBUG_READ		102 //debug use
#define CMD_DEBUG_WRITE		103 //debug use

#define CHIP_ID_ADDR	(0x10)
#define USER_ID_ADDR	(0x20)

struct efuse_wr_info {
	uint32_t seg_id;
	uint32_t data_length;	/* 7 --> 4 , 3 --> 0 / n words */
	uint32_t *buf;
	uint32_t start_pos;		/* 7 ~ 4,3 ~ 0  /words */
};

struct jz_efuse {
	struct jz_efuse_platform_data *pdata;
	struct device *dev;
	struct miscdevice mdev;
	struct efuse_wr_info *wr_info;
	spinlock_t lock;
	void __iomem *iomem;
	int gpio_vddq_en_n;
	struct timer_list vddq_protect_timer;
};

struct jz_efuse *efuse;

static uint32_t efuse_readl(uint32_t reg_off)
{
	return readl(efuse->iomem + reg_off);
}

static void efuse_writel(uint32_t val, uint32_t reg_off)
{
	writel(val, efuse->iomem + reg_off);
}

static int efuse_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int efuse_release(struct inode *inode, struct file *filp)
{
	/*clear configer register */
	/*efuse_writel(0, EFUSE_CFG); */
	return 0;
}

static void efuse_vddq_set(unsigned long is_on)
{
	printk("JZ-EFUSE-V11: vddq_set %d\n", (int)is_on);
	if (is_on) {
		mod_timer(&efuse->vddq_protect_timer, jiffies + HZ);
	}

	if (efuse->gpio_vddq_en_n != -ENODEV) {
		gpio_set_value(efuse->gpio_vddq_en_n, !is_on);
	}
}

static int jz_efuse_read(int seg_id, uint32_t * buf)
{
	int i;

	printk("0--jz_efuse_read\n");

	if(seg_id != USER_ID_ADDR && seg_id != CHIP_ID_ADDR){
		printk("ERROR: seg_id error,check it !!! \n");
		return -1;
	}

	spin_lock(&efuse->lock);

	if (seg_id == CHIP_ID_ADDR) {
		for (i = 0; i < 4; i++)
			*(buf + i) = efuse_readl(CHIP_ID_ADDR + i * 4);
	} else if (seg_id == USER_ID_ADDR) {
		for (i = 0; i < 4; i++)
			*(buf + i) = efuse_readl(USER_ID_ADDR + i * 4);
	}

	spin_unlock(&efuse->lock);
	return 0;
}

void jz_efuse_id_read(int is_chip_id, uint32_t * buf)
{
	if (is_chip_id) {
		jz_efuse_read(CHIP_ID_ADDR, buf);
	} else {
		jz_efuse_read(USER_ID_ADDR, buf);
	}
}

EXPORT_SYMBOL_GPL(jz_efuse_id_rad);

static int jz_efuse_write(int seg_id, uint32_t * buf)
{
	int i;

	printk("1--jz_efuse_write\n");
	if (efuse->gpio_vddq_en_n == -ENODEV) {
		printk("JZ-EFUSE-V11: The VDDQ can't be opened by software!\n");
		return -1;
	}

	if(seg_id != USER_ID && seg_id != CHIP_ID){
		printk("ERROR: seg_id error,check it !!! \n");
		return -1;
	}
	spin_lock(&efuse->lock);

	efuse_vddq_set(1);

	if (seg_id == CHIP_ID_ADDR) {
		for (i = 0; i < 4; i++)
			efuse_writel(CHIP_ID_ADDR + i * 4, *(buf + i));

		efuse_writel(EFUSE_CTRL, LO_WEN);
		while ((efuse_readl(EFUSE_CTRL) & LO_WEN)) ;
	} else if (seg_id == USER_ID_ADDR) {
		for (i = 0; i < 4; i++)
			efuse_writel(USER_ID_ADDR + i * 4, *(buf + i));

		efuse_writel(EFUSE_CTRL, HI_WEN);
		while ((efuse_readl(EFUSE_CTRL) & HI_WEN)) ;
	}
	efuse_vddq_set(0);

	spin_unlock(&efuse->lock);
	return 0;
}
static int jz_efuse_debug_read(struct efuse_wr_info *wr_info)
{
	uint32_t seg_id = wr_info->seg_id;
	int start_pos = wr_info->start_pos;
	int len = wr_info->data_length; /* words */
	int add_off;

	printk(" 2 - jz_efuse debug read \n");

	if(seg_id == CHIP_ID){
		if(len > 4){
			dev_err(efuse->dev, "read segment %d data length %d > 4 words \n", seg_id, len);
			return -1;
		}
		if(start_pos > 3){
			dev_err(efuse->dev, "read segment %d start_pos  %d > (0 ~ 3) \n", seg_id, start_pos);
			return -1;
		}
	}else if(seg_id == USER_ID){
		if(len > 4){
			dev_err(efuse->dev, "read segment %d data length %d > 4 words\n", seg_id, len);
			return -1;
		}
		if(start_pos > 7 || start_pos < 4){
			dev_err(efuse->dev, "read segment %d start_pos  %d < (4 ~ 7) \n", seg_id, start_pos);
			return -1;
		}
	}else{
		dev_err(efuse->dev, "read segment %d error  > 1 ,should check the seg_id  \n", seg_id);
		return -1;
	}

	if(len <= 0){
		dev_err(efuse->dev, "data_length  <= 0 ,don't do anything \n");
		return -1;
	}
	if(len > start_pos + 1){
		dev_warn(efuse->dev,"the read data_length > start_pos + 1,we just make the (start_pos + 1) as the data_length\n");
		len = start_pos + 1;
	}

	if(seg_id == CHIP_ID)
		add_off = CHIP_ID_ADDR;
	else if(seg_id == USER_ID)
		add_off = USER_ID_ADDR;

	spin_lock(&efuse->lock);

	do{
		*(wr_info->buf + (len - 1)) = efuse_readl(add_off + start_pos * 4);
		len--;
		start_pos--;
	}while(start_pos >= 0 && len >= 0);

	spin_unlock(&efuse->lock);

	return 0;
}

static int jz_efuse_debug_write(struct efuse_wr_info *wr_info)
{
	uint32_t seg_id = wr_info->seg_id;
	int start_pos = wr_info->start_pos;
	int len = wr_info->data_length; /* words */
	int add_off;

	printk(" 3 - jz_efuse debug write \n");

	if(seg_id == CHIP_ID){
		if(len > 4){
			dev_err(efuse->dev, "write segment %d data length %d > 4 words \n", seg_id, len);
			return -1;
		}
		if(start_pos > 3){
			dev_err(efuse->dev, "write segment %d start_pos  %d > (0 ~ 3) \n", seg_id,start_pos);
			return -1;
		}
	}else if(seg_id == USER_ID){
		if(len > 4){
			dev_err(efuse->dev, "write segment %d data length %d > 4 words\n", seg_id, len);
			return -1;
		}
		if(start_pos > 7 || start_pos < 4){
			dev_err(efuse->dev, "write segment %d start_pos  %d < (4 ~ 7) \n", seg_id, start_pos);
			return -1;
		}
	}else{
		dev_err(efuse->dev, "write segment %d error  > 1 ,should check the seg_id  \n", seg_id);
		return -1;
	}
	if(len <= 0){
		dev_err(efuse->dev, "data_length  <= 0 ,don't do anything \n");
		return -1;
	}
	if(len > start_pos + 1){
		dev_warn(efuse->dev,"the write data_length > start_pos + 1,we just make the (start_pos + 1) as the data_length\n");
		len = start_pos + 1;
	}

	if(seg_id == CHIP_ID)
		add_off = CHIP_ID_ADDR;
	else if(seg_id == USER_ID)
		add_off = USER_ID_ADDR;

	spin_lock(&efuse->lock);
	efuse_vddq_set(1);

	do{
		efuse_writel(add_off + start_pos * 4, *(wr_info->buf + (len - 1)));
		len--;
		start_pos--;
	}while(start_pos >= 0 && len >= 0);

	if(seg_id == CHIP_ID){
		efuse_writel(EFUSE_CTRL, LO_WEN);
		while ((efuse_readl(EFUSE_CTRL) & LO_WEN)) ;
	}else if(seg_id == USER_ID){
		efuse_writel(EFUSE_CTRL, HI_WEN);
		while ((efuse_readl(EFUSE_CTRL) & HI_WEN)) ;
	}

	efuse_vddq_set(0);
	spin_unlock(&efuse->lock);

	return 0;
}

static long efuse_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct miscdevice *dev = filp->private_data;
	struct jz_efuse *efuse = container_of(dev, struct jz_efuse, mdev);
	unsigned int *arg_r;
	int ret = 0;

	arg_r = (unsigned int *)arg;
	efuse->wr_info = (struct efuse_wr_info *)arg_r;
	//copy_from_user(efuse->wr_info,arg,sizeof(struct efuse_wr_info));
	switch (cmd) {
	case CMD_READ:
		ret = jz_efuse_read(efuse->wr_info->seg_id, efuse->wr_info->buf);
		break;
	case CMD_WRITE:
		ret = jz_efuse_write(efuse->wr_info->seg_id, efuse->wr_info->buf);
		break;
	case CMD_DEBUG_READ:
		ret = jz_efuse_debug_read(efuse->wr_info);
		//copy_to_user(arg,efuse->wr_info,sizeof(struct efuse_wr_info));
		break;
	case CMD_DEBUG_WRITE:
		ret = jz_efuse_debug_write(efuse->wr_info);
		break;
	default:
		ret = -1;
		printk("no support other cmd\n");
	}
	return ret;
}

static struct file_operations efuse_misc_fops = {
	.open = efuse_open,
	.release = efuse_release,
	.unlocked_ioctl = efuse_ioctl,
};

static int efuse_read_chip_id_proc(struct seq_file *m, void *v)
{
	int len = 0;
	uint32_t buf[4];
	jz_efuse_read(CHIP_ID_ADDR, buf);
	len =
	    seq_printf(m,"--------> chip id: %08x-%08x-%08x-%08x\n", buf[0], buf[1],
		    buf[2], buf[3]);
	return len;
}

static int efuse_read_user_id_proc(struct seq_file *m, void *v)
{
	int len = 0;
	uint32_t buf[4];
	jz_efuse_read(USER_ID_ADDR, buf);
	len =
	    seq_printf(m, "--------> user id: %08x-%08x-%08x-%08x\n", buf[0], buf[1],
		    buf[2], buf[3]);

	return len;
}

static int efuse_read_chipID_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, efuse_read_chip_id_proc, PDE_DATA(inode));
}
static int efuse_read_userID_proc_open(struct inode *inode,struct file *file)
{
	return single_open(file, efuse_read_user_id_proc, PDE_DATA(inode));
}

static const struct file_operations efuse_proc_read_chipID_fops ={
	.read = seq_read,
	.open = efuse_read_chipID_proc_open,
	.write = NULL,
	.llseek = seq_lseek,
	.release = single_release,
};
static const struct file_operations efuse_proc_read_userID_fops ={
	.read = seq_read,
	.open = efuse_read_userID_proc_open,
	.write = NULL,
	.llseek = seq_lseek,
	.release = single_release,
};

static int efuse_probe(struct platform_device *pdev)
{
	int ret = 0;
	//unsigned long rate;
	//uint32_t val, ns;
	struct proc_dir_entry *res, *p;

	efuse = kzalloc(sizeof(struct jz_efuse), GFP_KERNEL);
	if (!efuse) {
		printk("efuse:malloc faile\n");
		return -ENOMEM;
	}

	efuse->pdata = pdev->dev.platform_data;
	if (!efuse->pdata) {
		dev_err(&pdev->dev, "No platform data\n");
		ret = -1;
		goto fail_free_efuse;
	}

	efuse->dev = &pdev->dev;

	efuse->gpio_vddq_en_n = efuse->pdata->gpio_vddq_en_n;

	efuse->iomem = ioremap(EFUSE_IOBASE, 0x100);
	if (!efuse->iomem) {
		dev_err(efuse->dev, "ioremap failed!\n");
		ret = -EBUSY;
		goto fail_free_efuse;
	}

	if (efuse->gpio_vddq_en_n != -ENODEV) {
		ret = gpio_request(efuse->gpio_vddq_en_n, dev_name(efuse->dev));
		if (ret) {
			dev_err(efuse->dev, "Failed to request gpio pin: %d\n",
				ret);
			goto fail_free_io;
		}
		ret = gpio_direction_output(efuse->gpio_vddq_en_n, 1);	/* power off by default */
		if (ret) {
			dev_err(efuse->dev,
				"Failed to set gpio as output: %d\n", ret);
			goto fail_free_gpio;
		}
	}

	dev_info(efuse->dev, "setup vddq_protect_timer!\n");
	setup_timer(&efuse->vddq_protect_timer, efuse_vddq_set, 0);
	add_timer(&efuse->vddq_protect_timer);

	spin_lock_init(&efuse->lock);

	efuse->mdev.minor = MISC_DYNAMIC_MINOR;
	efuse->mdev.name = DRV_NAME;
	efuse->mdev.fops = &efuse_misc_fops;

	spin_lock_init(&efuse->lock);

	ret = misc_register(&efuse->mdev);
	if (ret < 0) {
		dev_err(efuse->dev, "misc_register failed\n");
		goto fail_free_gpio;
	}
	platform_set_drvdata(pdev, efuse);

	efuse->wr_info = NULL;

	p = jz_proc_mkdir("efuse");
	if (!p) {
		pr_warning("create_proc_entry for common efuse failed.\n");
		return -ENODEV;
	}

	res = proc_create("efuse_chip_id", 0444,p,&efuse_proc_read_chipID_fops);
	if(!res){
		pr_err("create proc of efuse_chip_id error!!!!\n");
	}
	res = proc_create("efuse_user_id",0444,p,&efuse_proc_read_userID_fops);
	if(!res){
		pr_err("create proc of efuse_user_id error!!!!\n");
	}

	dev_info(efuse->dev,
		 "ingenic efuse interface module registered success.\n");
	return 0;
fail_free_efuse:
	kfree(efuse);
fail_free_gpio:
	gpio_free(efuse->gpio_vddq_en_n);
fail_free_io:
	iounmap(efuse->iomem);

	return ret;
}

static int  efuse_remove(struct platform_device *dev)
{
	struct jz_efuse *efuse = platform_get_drvdata(dev);

	misc_deregister(&efuse->mdev);
	if (efuse->gpio_vddq_en_n != -ENODEV) {
		gpio_free(efuse->gpio_vddq_en_n);
		dev_info(efuse->dev, "del vddq_protect_timer!\n");
		del_timer(&efuse->vddq_protect_timer);
	}
	iounmap(efuse->iomem);
	kfree(efuse);

	return 0;
}

static struct platform_driver efuse_driver = {
	.driver.name = "jz-efuse",
	.driver.owner = THIS_MODULE,
	.probe = efuse_probe,
	.remove = efuse_remove,
};

static int __init efuse_init(void)
{
	return platform_driver_register(&efuse_driver);
}

static void __exit efuse_exit(void)
{
	platform_driver_unregister(&efuse_driver);
}

module_init(efuse_init);
module_exit(efuse_exit);
MODULE_LICENSE("GPL");
