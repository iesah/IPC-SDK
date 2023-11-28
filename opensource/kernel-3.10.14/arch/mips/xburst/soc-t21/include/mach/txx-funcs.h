#ifndef __TXX_FUNCS_H__
#define __TXX_FUNCS_H__
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/pwm.h>
#include <linux/file.h>
#include <linux/gpio.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/mfd/core.h>
#include <linux/mempolicy.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <soc/irq.h>
#include <soc/base.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <soc/gpio.h>
#include <mach/platform.h>
#include <jz_proc.h>

struct jz_driver_common_interfaces {
	unsigned int flags_0;			// The flags must be checked.
	/* platform interface */
	int (*priv_platform_driver_register)(struct platform_driver *drv);
	void (*priv_platform_driver_unregister)(struct platform_driver *drv);
	void (*priv_platform_set_drvdata)(struct platform_device *pdev, void *data);
	void *(*priv_platform_get_drvdata)(const struct platform_device *pdev);
	int (*priv_platform_device_register)(struct platform_device *pdev);
	void (*priv_platform_device_unregister)(struct platform_device *pdev);
	struct resource *(*priv_platform_get_resource)(struct platform_device *dev,
				       unsigned int type, unsigned int num);
	int (*priv_dev_set_drvdata)(struct device *dev, void *data);
	void* (*priv_dev_get_drvdata)(const struct device *dev);
	int (*priv_platform_get_irq)(struct platform_device *dev, unsigned int num);
	struct resource * (*tx_request_mem_region)(resource_size_t start, resource_size_t n,
				   const char *name);
	void (*tx_release_mem_region)(resource_size_t start, resource_size_t n);
	void __iomem *(*tx_ioremap)(phys_addr_t offset, unsigned long size);
	void (*priv_iounmap)(const volatile void __iomem *addr);
	unsigned int reserve_platform[8];

	/* interrupt interface */
	int (*priv_request_threaded_irq)(unsigned int irq, irq_handler_t handler,
			 irq_handler_t thread_fn, unsigned long irqflags,
			 const char *devname, void *dev_id);
	void (*priv_enable_irq)(unsigned int irq);
	void (*priv_disable_irq)(unsigned int irq);
	void (*priv_free_irq)(unsigned int irq, void *dev_id);

	/* lock and mutex interface */
	void (*tx_spin_lock_irqsave)(spinlock_t *lock, unsigned long *flags);
	void (*priv_spin_unlock_irqrestore)(spinlock_t *lock, unsigned long flags);
	void (*tx_spin_lock_init)(spinlock_t *lock);
	void (*priv_mutex_lock)(struct mutex *lock);
	void (*priv_mutex_unlock)(struct mutex *lock);
	void (*tx_mutex_init)(struct mutex *mutex);

	/* clock interfaces */
	struct clk *(*priv_clk_get)(struct device *dev, const char *id);
	int (*priv_clk_enable)(struct clk *clk);
	int (*priv_clk_is_enabled)(struct clk *clk);
	void (*priv_clk_disable)(struct clk *clk);
	unsigned long (*priv_clk_get_rate)(struct clk *clk);
	void(*priv_clk_put)(struct clk *clk);
	int (*priv_clk_set_rate)(struct clk *clk, unsigned long rate);
	unsigned int reserve_clk[8];

	/* i2c interfaces */
	struct i2c_adapter* (*priv_i2c_get_adapter)(int nr);
	void (*priv_i2c_put_adapter)(struct i2c_adapter *adap);
	int (*priv_i2c_transfer)(struct i2c_adapter *adap, struct i2c_msg *msgs, int num);
	int (*priv_i2c_register_driver)(struct module *, struct i2c_driver *);
	void (*priv_i2c_del_driver)(struct i2c_driver *);

	struct i2c_client *(*priv_i2c_new_device)(struct i2c_adapter *adap, struct i2c_board_info const *info);
	void *(*priv_i2c_get_clientdata)(const struct i2c_client *dev);
	void (*priv_i2c_set_clientdata)(struct i2c_client *dev, void *data);
	void (*priv_i2c_unregister_device)(struct i2c_client *client);

	unsigned int reserver_i2c[8];

	/* gpio interfaces */
	int (*priv_gpio_request)(unsigned gpio, const char *label);
	void (*priv_gpio_free)(unsigned gpio);
	int (*priv_gpio_direction_output)(unsigned gpio, int value);
	int (*priv_gpio_direction_input)(unsigned gpio);
	int (*priv_gpio_set_debounce)(unsigned gpio, unsigned debounce);
	int (*priv_jzgpio_set_func)(enum gpio_port port, enum gpio_function func,unsigned long pins);
	int (*priv_jzgpio_ctrl_pull)(enum gpio_port port, int enable_pull,unsigned long pins);

	/* system interface */
	void (*priv_msleep)(unsigned int msecs);
	bool (*priv_capable)(int cap);
	unsigned long long (*priv_sched_clock)(void);
	bool (*priv_try_module_get)(struct module *module);
	int (*tx_request_module)(bool wait, const char *fmt, char *s);
	void (*priv_module_put)(struct module *module);

	/* wait */
	void (*priv_init_completion)(struct completion *x);
	void (*priv_complete)(struct completion *x);
	int (*priv_wait_for_completion_interruptible)(struct completion *x);

	/* misc */
	int (*priv_misc_register)(struct miscdevice *mdev);
	int (*priv_misc_deregister)(struct miscdevice *mdev);
	struct proc_dir_entry *(*priv_proc_create_data)(const char *name, umode_t mode,
					struct proc_dir_entry *parent,
					const struct file_operations *proc_fops,
					void *data);
	/* proc */
	ssize_t (*priv_seq_read)(struct file *file, char __user *buf, size_t size, loff_t *ppos);
	loff_t (*priv_seq_lseek)(struct file *file, loff_t offset, int whence);
	int (*priv_single_release)(struct inode *inode, struct file *file);
	int (*priv_single_open_size)(struct file *file, int (*show)(struct seq_file *, void *),
		void *data, size_t size);
	struct proc_dir_entry* (*priv_jz_proc_mkdir)(char *s);
	/* isp driver interface */
	void (*get_isp_priv_mem)(unsigned int *phyaddr, unsigned int *size);
	unsigned int flags_1;			// The flags must be checked.
};

#endif /*__TXX_FUNCS_H__*/
