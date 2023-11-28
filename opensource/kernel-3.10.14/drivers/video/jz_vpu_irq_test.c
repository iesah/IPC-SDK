#include <linux/module.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <soc/base.h>
#include <soc/cpm.h>
//#include <mach/jzcpm_pwc.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <asm/delay.h>

#include "soc_vpu/soc_vpu.h"
#include "soc_vpu/helix/helix.h"
#include "soc_vpu/radix/radix.h"

static struct completion comp;
static struct completion comp_radix;
static struct jz_vpu *vpu = NULL;

static unsigned int vpu_stat = 0;
static unsigned int vpu_stat_radix = 0;

static struct miscdevice vpu_irq_dev;

static int vpuirq;
static int vpuirq_radix;

#define IOCTL_VPUIRQ_GET_STAT 177
#define IOCTL_VPUIRQ_GET_STAT_RADIX 178
#define IOCTL_VPU_HELIX_RESET 179
#define IOCTL_VPU_RADIX_RESET 180
struct jz_vpu {
	struct vpu		vpu;
	char			name[16];
	char			name_radix[16];
	int			irq;//helix
	int			irq_radix;//radix
	void __iomem		*iomem;
	void __iomem		*iomem_radix;
	struct clk		*clk;
	struct clk		*helix_clk_gate;
	struct clk		*radix_clk_gate;
	struct clk		*ahb1_clk_gate;
	enum jz_vpu_status	vpu_status;
	struct completion	done;
	struct completion	done_radix;
	spinlock_t		slock;
	struct mutex		mutex;
	pid_t			owner_pid;
	unsigned int		status;
	unsigned int		bslen;
	void*                   cpm_pwc;
	unsigned int		cmpx;
};

#define radix_readl(vpu, offset)		__raw_readl((vpu)->iomem_radix + offset)
#define radix_writel(vpu, offset, value)	__raw_writel((value), (vpu)->iomem_radix + offset)

#define CLEAR_RADIX_BIT(vpu,offset,bm)				\
	do {							\
		unsigned int stat;				\
		stat = radix_readl(vpu,offset);			\
		radix_writel(vpu,offset,stat & ~(bm));		\
	} while(0)

static irqreturn_t vpu_interrupt_t(int irq, void *dev)
{
	//struct jz_vpu *vpu = dev;

	printk("In vpu_interrupt.\n");
	disable_irq_nosync(vpuirq);
	vpu_stat = vpu_readl(vpu,REG_SCH_STAT);
	printk("In vpu_interrupt : vpu_stat = 0x%08x\n", vpu_stat);
	CLEAR_VPU_BIT(vpu, REG_SCH_GLBC, SCH_INTE_MASK);
	complete(&comp);

	enable_irq(vpuirq);

	return IRQ_HANDLED;
}

static irqreturn_t vpu_interrupt_t_radix(int irq, void *dev)
{
	//struct jz_vpu *vpu = dev;

	printk("In vpu_interrupt. radix\n");
	disable_irq_nosync(vpuirq_radix);
	vpu_stat_radix = radix_readl(vpu, RADIX_CFGC_BASE + RADIX_REG_CFGC_STAT);
	printk("In vpu_interrupt : vpu_stat_radix(%x) = 0x%08x\n", RADIX_CFGC_BASE + RADIX_REG_CFGC_STAT, vpu_stat_radix);
	printk("In vpu_interrupt : glbc(%x) = 0x%08x\n", RADIX_CFGC_BASE + RADIX_REG_CFGC_GLB_CTRL, radix_readl(vpu, RADIX_CFGC_BASE + RADIX_REG_CFGC_GLB_CTRL));
	CLEAR_RADIX_BIT(vpu, 0, RADIX_CFGC_INTE_MASK);
	complete(&comp_radix);

	enable_irq(vpuirq_radix);

	return IRQ_HANDLED;
}

static int vpu_irq_open(struct inode *inode, struct file *file)
{
    clk_enable(vpu->clk);
    clk_enable(vpu->ahb1_clk_gate);
    clk_enable(vpu->helix_clk_gate);
    clk_enable(vpu->radix_clk_gate);
	enable_irq(vpuirq);
	enable_irq(vpuirq_radix);
	printk("%s: vpu_irq open successful!\n", __func__);
	return 0;
}

static int vpu_irq_release(struct inode *inode, struct file *file)
{
	disable_irq_nosync(vpuirq);
	disable_irq_nosync(vpuirq_radix);
	return 0;
}
static ssize_t vpu_irq_read(struct file *file, char __user * buffer, size_t count, loff_t * ppos)
{
	return 0;
}

static ssize_t vpu_irq_write(struct file *file, const char __user * buffer, size_t count, loff_t * ppos)
{
	return 0;
}

#define CPM_VPU_SR(ID)		(31-3*(ID))
#define CPM_VPU_STP(ID)		(30-3*(ID))
#define CPM_VPU_ACK(ID)		(29-3*(ID))

static long vpu_reset(int vpuid)
{
	int timeout = 0x7fff;
	unsigned int srbc = cpm_inl(CPM_SRBC);

	cpm_set_bit(CPM_VPU_STP(vpuid), CPM_SRBC);
	while (!(cpm_inl(CPM_SRBC) & (1 << CPM_VPU_ACK(vpuid))) && --timeout) udelay(20);

	if (timeout == 0) {
		printk("wait stop ack timeout\n");
		cpm_outl(srbc, CPM_SRBC);
		return -1;
	} else {
		cpm_outl(srbc | (1 << CPM_VPU_SR(vpuid)), CPM_SRBC);
		cpm_outl(srbc, CPM_SRBC);
	}

	return 0;
}

static long vpu_irq_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp  = (void __user *)arg;
	int ret = 0;

	printk("In vpu irq ioctl.\n");
	switch(cmd) {
	case IOCTL_VPUIRQ_GET_STAT:
		printk("In vpu irq ioctl case.\n");
		ret = wait_for_completion_timeout(&comp, msecs_to_jiffies(10*1000));
		if (ret <= 0)
			printk("wait vpu irq time out: %d\n", ret);
		if (copy_to_user(argp, &vpu_stat, sizeof(unsigned int))) {
			printk("vpu_irq get vpu_stat copy_to_user error!!!\n");
			return -EFAULT;
		}
		break;
	case IOCTL_VPUIRQ_GET_STAT_RADIX:
		printk("In vpu irq ioctl case radix.\n");
		ret = wait_for_completion_timeout(&comp_radix, msecs_to_jiffies(10*1000));
		if (ret <= 0)
			printk("wait vpu irq time out: %d\n", ret);
		if (copy_to_user(argp, &vpu_stat_radix, sizeof(unsigned int))) {
			printk("vpu_irq get vpu_stat_radix copy_to_user error!!!\n");
			return -EFAULT;
		}
		break;
    case IOCTL_VPU_HELIX_RESET:
        return vpu_reset(0);
        break;
    case IOCTL_VPU_RADIX_RESET:
        return vpu_reset(1);
        break;
	}
	return 0;
}

const struct file_operations vpu_irq_fops = {
	.owner = THIS_MODULE,
	.read = vpu_irq_read,
	.write = vpu_irq_write,
	.open = vpu_irq_open,
	.unlocked_ioctl = vpu_irq_ioctl,
	.release = vpu_irq_release,
};

static int vpu_probe(struct platform_device *pdev)
{
	int ret;
	struct resource	*regs;
	struct resource	*regs_radix;
	//struct jz_vpu *vpu;

	vpu = kzalloc(sizeof(struct jz_vpu), GFP_KERNEL);
	if (!vpu) {
		dev_err(&pdev->dev, "kzalloc vpu space failed\n");
		ret = -ENOMEM;
		goto err_kzalloc_vpu;
	}

	vpu->vpu.idx = pdev->id;
	sprintf(vpu->name, "vpu0");
	sprintf(vpu->name_radix, "vpu-radix");

	vpuirq = vpu->irq = platform_get_irq(pdev, 0);
	if(vpu->irq < 0) {
		dev_err(&pdev->dev, "get irq failed\n");
		ret = vpu->irq;
		goto err_get_vpu_irq;
	}
	vpuirq_radix = vpu->irq_radix = platform_get_irq(pdev, 1);
	if(vpu->irq_radix < 0) {
		dev_err(&pdev->dev, "get irq_radix failed\n");
		ret = vpu->irq_radix;
		goto err_get_vpu_irq;
	}

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs) {
		dev_err(&pdev->dev, "No iomem resource\n");
		ret = -ENXIO;
		goto err_get_vpu_resource;
	}
	regs_radix = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!regs_radix) {
		dev_err(&pdev->dev, "No iomem resource\n");
		ret = -ENXIO;
		goto err_get_vpu_resource;
	}

	vpu->iomem = ioremap(regs->start, resource_size(regs));
	if (!vpu->iomem) {
		dev_err(&pdev->dev, "ioremap failed\n");
		ret = -ENXIO;
		goto err_get_vpu_iomem;
	}
	vpu->iomem_radix = ioremap(regs_radix->start, resource_size(regs_radix));
	if (!vpu->iomem_radix) {
		dev_err(&pdev->dev, "ioremap failed\n");
		ret = -ENXIO;
		goto err_get_vpu_iomem;
	}

	vpu->ahb1_clk_gate = clk_get(&pdev->dev, "ahb1");
	if (IS_ERR(vpu->ahb1_clk_gate)) {
		dev_err(&pdev->dev, "ahb1_clk_gate get failed\n");
		ret = PTR_ERR(vpu->ahb1_clk_gate);
		goto err_get_vpu_ahb1_clk_gate;
	}

	vpu->helix_clk_gate = clk_get(&pdev->dev, "helix0");
	if (IS_ERR(vpu->helix_clk_gate)) {
		dev_err(&pdev->dev, "helix_clk_gate get failed\n");
		ret = PTR_ERR(vpu->helix_clk_gate);
		goto err_get_vpu_helix_clk_gate;
	}

	vpu->radix_clk_gate = clk_get(&pdev->dev, "radix0");
	if (IS_ERR(vpu->radix_clk_gate)) {
		dev_err(&pdev->dev, "radix_clk_gate get failed\n");
		ret = PTR_ERR(vpu->radix_clk_gate);
		goto err_get_vpu_radix_clk_gate;
	}

    vpu->clk = clk_get(&pdev->dev,"cgu_vpu");
    if (IS_ERR(vpu->clk)) {
        dev_err(&pdev->dev, "clk get failed\n");
        ret = PTR_ERR(vpu->clk);
        goto err_get_vpu_clk_cgu;
    }
    clk_set_rate(vpu->clk,300000000);

	spin_lock_init(&vpu->slock);
	mutex_init(&vpu->mutex);
	init_completion(&vpu->done);
	init_completion(&vpu->done_radix);

	init_completion(&comp);
	init_completion(&comp_radix);
	ret = request_irq(vpu->irq, vpu_interrupt_t, IRQF_DISABLED, vpu->name, vpu);
	if (ret < 0) {
		dev_err(&pdev->dev, "request_irq failed\n");
		goto err_vpu_helix_request_irq;
	}
	ret = request_irq(vpu->irq_radix, vpu_interrupt_t_radix, IRQF_DISABLED, vpu->name_radix, vpu);
	if (ret < 0) {
		dev_err(&pdev->dev, "request_irq radix failed\n");
		goto err_vpu_radix_request_irq;
	}
	disable_irq_nosync(vpu->irq);
	disable_irq_nosync(vpu->irq_radix);
#if 0
	vpu->cpm_pwc = cpm_pwc_get(PWC_VPU);
	if(!vpu->cpm_pwc) {
		dev_err(&pdev->dev, "get %s fail!\n",PWC_VPU);
		goto err_vpu_request_power;
	}
#endif

	vpu_irq_dev.minor = MISC_DYNAMIC_MINOR;
	vpu_irq_dev.fops = &vpu_irq_fops;
	vpu_irq_dev.name = "vpu_irq";

	ret = misc_register(&vpu_irq_dev);
	if(ret) {
		dev_err(&pdev->dev,"request vpu_irq misc device failed!\n");
        goto err_vpu_register;
	}
	platform_set_drvdata(pdev, vpu);

	return 0;

err_vpu_register:
#if 0
	cpm_pwc_put(vpu->cpm_pwc);
err_vpu_request_power:
#endif
	free_irq(vpu->irq, vpu);
err_vpu_radix_request_irq:
	free_irq(vpu->irq_radix, vpu);
err_vpu_helix_request_irq:
    clk_put(vpu->clk);
err_get_vpu_clk_cgu:
	clk_put(vpu->radix_clk_gate);
err_get_vpu_radix_clk_gate:
	clk_put(vpu->helix_clk_gate);
err_get_vpu_helix_clk_gate:
	clk_put(vpu->ahb1_clk_gate);
err_get_vpu_ahb1_clk_gate:
	iounmap(vpu->iomem);
err_get_vpu_iomem:
err_get_vpu_resource:
err_get_vpu_irq:
	kfree(vpu);
err_kzalloc_vpu:
	return ret;
}

static int vpu_remove(struct platform_device *dev)
{
	//struct jz_vpu *vpu = platform_get_drvdata(dev);

	/*vpu_unregister(&vpu->vpu.vlist);*/
	//cpm_pwc_put(vpu->cpm_pwc);

	free_irq(vpu->irq, vpu);
	free_irq(vpu->irq_radix, vpu);
    clk_put(vpu->clk);
	clk_put(vpu->radix_clk_gate);
	clk_put(vpu->helix_clk_gate);
	clk_put(vpu->ahb1_clk_gate);
	iounmap(vpu->iomem);
	kfree(vpu);

	return 0;
}


static struct platform_driver jz_vpu_irq_driver = {
	.probe		= vpu_probe,
	.remove		= vpu_remove,
	.driver		= {
		.name	= "jz-vpu-irq",
	},
};

static int __init vpu_init(void)
{
	return platform_driver_register(&jz_vpu_irq_driver);
}

static void __exit vpu_exit(void)
{
	platform_driver_unregister(&jz_vpu_irq_driver);
}

module_init(vpu_init);
module_exit(vpu_exit);

MODULE_DESCRIPTION("JZ T30 VPU IRQ driver");
MODULE_AUTHOR("Elvis <huan.wang@ingenic.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("20160524");
