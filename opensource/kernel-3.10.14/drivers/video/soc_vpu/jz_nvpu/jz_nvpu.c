#include <linux/module.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <soc/base.h>
#include <soc/cpm.h>
#include <asm/delay.h>
//#include <mach/jzcpm_pwc.h>
#include <linux/interrupt.h>

#include "../soc_vpu.h"
#include "jz_nvpu.h"
#include "jzm_vpu.h"
#include "jz_ncu.h"

struct jz_vpu {
	struct vpu		vpu;
	char			name[16];
	int			irq;
	void __iomem		*iomem;
	struct clk		*clk;
	struct clk		*clk_gate;
	enum jz_vpu_status	vpu_status;
	struct completion	done;
	spinlock_t		slock;
	struct mutex		mutex;
	pid_t			owner_pid;
	unsigned int		status;
	unsigned int		bslen;
	void*                   cpm_pwc;
	unsigned int		cmpx;
};

//#define CONFIG_BOARD_T10_FPGA

#ifdef CONFIG_BOARD_T10_FPGA
#define CPM_CLKGR1_VPU_BASE       (0)
#define CPM_CLKGR1_VPU(ID)	  (13 * (ID))
int vpu_clk_gate_enable(int id)
{
	cpm_clear_bit(CPM_CLKGR1_VPU(id), CPM_CLKGR1);
	return 0;
}
#endif

static long vpu_reset(struct device *dev)
{
	struct jz_vpu *vpu = dev_get_drvdata(dev);
	int timeout = 0x7fff;
	unsigned int srbc = cpm_inl(CPM_SRBC);

	cpm_set_bit(CPM_VPU_STP(vpu->vpu.idx), CPM_SRBC);
	while (!(cpm_inl(CPM_SRBC) & (1 << CPM_VPU_ACK(vpu->vpu.idx))) && --timeout) udelay(20);

	if (timeout == 0) {
		dev_warn(vpu->vpu.dev, "[%d:%d] wait stop ack timeout\n",
			 current->tgid, current->pid);
		cpm_outl(srbc, CPM_SRBC);
		return -1;
	} else {
		cpm_outl(srbc | (1 << CPM_VPU_SR(vpu->vpu.idx)), CPM_SRBC);
		cpm_outl(srbc, CPM_SRBC);
	}

	return 0;
}


static long vpu_open(struct device *dev)
{
	struct jz_vpu *vpu = dev_get_drvdata(dev);
	unsigned long slock_flag = 0;

	spin_lock_irqsave(&vpu->slock, slock_flag);
	if (cpm_inl(CPM_OPCR) & OPCR_IDLE)
		return -EBUSY;

    clk_enable(vpu->clk);
#ifdef CONFIG_BOARD_T10_FPGA
	vpu_clk_gate_enable(vpu->vpu.idx);
#else
	clk_enable(vpu->clk_gate);
#endif
	//cpm_pwc_enable(vpu->cpm_pwc);

	__asm__ __volatile__ (
			"mfc0  $2, $16,  7   \n\t"
			"ori   $2, $2, 0x340 \n\t"
			"andi  $2, $2, 0x3ff \n\t"
			"mtc0  $2, $16,  7  \n\t"
			"nop                  \n\t");

	vpu_reset(dev);
	enable_irq(vpu->irq);
	vpu->vpu_status = VPU_STATUS_OPEN;
	spin_unlock_irqrestore(&vpu->slock, slock_flag);
	while(try_wait_for_completion(&vpu->done));

	dev_dbg(dev, "[%d:%d] open\n", current->tgid, current->pid);

	return 0;
}

static long vpu_release(struct device *dev)
{
	struct jz_vpu *vpu = dev_get_drvdata(dev);
	unsigned long slock_flag = 0;

	spin_lock_irqsave(&vpu->slock, slock_flag);
	disable_irq_nosync(vpu->irq);

	__asm__ __volatile__ (
			"mfc0  $2, $16,  7   \n\t"
			"andi  $2, $2, 0xbf \n\t"
			"mtc0  $2, $16,  7  \n\t"
			"nop                  \n\t");

	cpm_clear_bit(CPM_VPU_SR(vpu->vpu.idx),CPM_OPCR);
    clk_disable(vpu->clk);
	clk_disable(vpu->clk_gate);
	//cpm_pwc_disable(vpu->cpm_pwc);
	/* Clear completion use_count here to avoid a unhandled irq after vpu off */
	vpu->done.done = 0;
	vpu->vpu_status = VPU_STATUS_CLOSE;
	spin_unlock_irqrestore(&vpu->slock, slock_flag);

	dev_dbg(dev, "[%d:%d] close\n", current->tgid, current->pid);

	return 0;
}

void dump_vpu_registers(struct jz_vpu *vpu)
{
	printk("REG_SCH_GLBC = %x\n", vpu_readl(vpu, REG_SCH_GLBC));
	printk("REG_SCH_TLBC = %x\n", vpu_readl(vpu, REG_SCH_TLBC));
	printk("REG_SCH_TLBV = %x\n", vpu_readl(vpu, REG_SCH_TLBV));
	printk("REG_SCH_TLBA = %x\n", vpu_readl(vpu, REG_SCH_TLBA));
	printk("REG_VDMA_TASKRG = %x\n", vpu_readl(vpu, REG_VDMA_TASKRG));
	printk("REG_EFE_GEOM = %x\n", vpu_readl(vpu, REG_EFE_GEOM));
	printk("REG_EFE_RAWV_SBA = %x\n", vpu_readl(vpu, REG_EFE_RAWV_SBA));
	printk("REG_MCE_CH1_RLUT+4 = %x\n", vpu_readl(vpu, REG_MCE_CH1_RLUT+4));
	printk("REG_MCE_CH2_RLUT+4 = %x\n", vpu_readl(vpu, REG_MCE_CH2_RLUT+4));
	printk("REG_DBLK_GPIC_YA = %x\n", vpu_readl(vpu, REG_DBLK_GPIC_YA));
	printk("REG_DBLK_GPIC_CA = %x\n", vpu_readl(vpu, REG_DBLK_GPIC_CA));
	printk("REG_EFE_RAWY_SBA = %x\n", vpu_readl(vpu, REG_EFE_RAWY_SBA));
	printk("REG_EFE_RAWU_SBA = %x\n", vpu_readl(vpu, REG_EFE_RAWU_SBA));
}

/**** tlb test ****/
int dump_tlb_info(unsigned int tlb_base)
{
	unsigned int pg1_index, pg2_index;
	unsigned int *pg1_vaddr, *pg2_vaddr;
	if(!tlb_base)
		return 0;
	pg1_vaddr = phys_to_virt(tlb_base);
	for (pg1_index=0; pg1_index<1024; pg1_index++) {
		if (!(pg1_vaddr[pg1_index] & 0x1)) {
			printk("pgd dir invalid!\n");
			continue;
		}
		printk("pg1_index:%d, content:0x%08x\n", pg1_index, pg1_vaddr[pg1_index]);
		pg2_vaddr = phys_to_virt(pg1_vaddr[pg1_index] & PAGE_MASK);
		for (pg2_index=0; pg2_index<1024; pg2_index++){
			if((pg2_index%4) == 0)
				printk("\n    pg2_index:%d",pg2_index);
			printk("  0x%08x[0x%08x]",(pg1_index << 22)|(pg2_index << 12), pg2_vaddr[pg2_index]);
		}
	}
	return 0;
}
/**** tlb test end ****/
void reset_ncu(void)
{
	ncu_write(NCU_STATUS, ncu_read( NCU_STATUS) & ~(1 << 0));
	ncu_write(NCU_START, ncu_read( NCU_START) | NCU_RST);//rst
}

static void vpu_start_ncu(struct jz_vpu *vpu,void *ncu_addr)
{
	ncu_reg_info_t reg[50];
	unsigned int i;

	copy_from_user(reg, ncu_addr, ARRAY_SIZE(reg) * sizeof(ncu_reg_info_t));
	for (i = 0; i < 50; i++) {
		if ((reg + i)->addr == 0 || (reg + i)->addr > 0x200)
			break;
		else
			ncu_write((reg + i)->addr, (reg + i)->val);
	}

	ncu_write(NCU_START, 0x1);
}

static long vpu_start_vpu(struct device *dev, const struct channel_node * const cnode)
{
#if 1
	struct jz_vpu *vpu = dev_get_drvdata(dev);
	struct channel_list *clist = list_entry(cnode->clist, struct channel_list, list);

	if (cnode->n_flag == 1) {
		vpu_start_ncu(vpu,cnode->ncu_addr);
		vpu_writel(vpu, 0x40070, 0x1);
	}

#ifdef DUMP_VPU_REG
	dev_info(vpu->vpu.dev, "-------------------dump start------------\n");
	dump_vpu_registers(vpu);
	dev_info(vpu->vpu.dev, "-------------------dump end------------\n");
#endif
	if (clist->tlb_flag == true || clist->private_tlb_flag == true) {
#ifdef DUMP_VPU_TLB
		dump_tlb_info(clist->tlb_pidmanager->tlbbase);
#endif
		if(clist->tlb_flag == true) {
			vpu_writel(vpu, REG_SCH_GLBC, SCH_GLBC_HIAXI | SCH_TLBE_JPGC | SCH_TLBE_DBLK
					| SCH_TLBE_SDE | SCH_TLBE_EFE | SCH_TLBE_VDMA | SCH_TLBE_MCE
					| SCH_INTE_ACFGERR | SCH_INTE_TLBERR | SCH_INTE_BSERR
					| SCH_INTE_ENDF);
			vpu_writel(vpu, REG_SCH_TLBC, SCH_TLBC_INVLD | SCH_TLBC_RETRY);
			vpu_writel(vpu, REG_SCH_TLBV, SCH_TLBV_RCI_MC | SCH_TLBV_RCI_EFE |
					SCH_TLBV_CNM(0x00) | SCH_TLBV_GCN(0x00));
			vpu_writel(vpu, REG_SCH_TLBA, clist->tlb_pidmanager->tlbbase);
		} else if(clist->private_tlb_flag == true) {
			vpu_writel(vpu, REG_SCH_GLBC, SCH_GLBC_HIAXI
					| SCH_TLBE_EFE
					| SCH_INTE_ACFGERR | SCH_INTE_TLBERR | SCH_INTE_BSERR
					| SCH_INTE_ENDF);
			vpu_writel(vpu, REG_SCH_TLBC, SCH_TLBC_INVLD | SCH_TLBC_RETRY);
			vpu_writel(vpu, REG_SCH_TLBV, SCH_TLBV_RCI_MC | SCH_TLBV_RCI_EFE |
					SCH_TLBV_CNM(0x00) | SCH_TLBV_GCN(0x00));
			if(clist->tlb_pidmanager) {
				if(clist->tlb_pidmanager->private_tlbbase) {
					vpu_writel(vpu, REG_SCH_TLBA, clist->tlb_pidmanager->private_tlbbase);
				}
			}
		}
	} else {
		vpu_writel(vpu, REG_SCH_GLBC, SCH_GLBC_HIAXI | SCH_INTE_ACFGERR
				| SCH_INTE_BSERR | SCH_INTE_ENDF);
	}

	if (cnode->thread_id >= 0) {
		SET_VPU_BIT(vpu, REG_EFE_DCS, 1 << (cnode->thread_id & 0xf));
	}
	vpu_writel(vpu, REG_VDMA_TASKRG, VDMA_ACFG_DHA(cnode->dma_addr)
			| VDMA_ACFG_RUN);

	//printk("cnode->dma_addr = %x, vpu->iomem = %p, clist->tlb_flag = %d\n", cnode->dma_addr, vpu->iomem, clist->tlb_flag);
#ifdef DUMP_VPU_REG
	dev_info(vpu->vpu.dev, "-------------------dump start------------\n");
	dump_vpu_registers(vpu);
	dev_info(vpu->vpu.dev, "--------------------dump end-------------\n");
#endif
	dev_dbg(vpu->vpu.dev, "[%d:%d] start vpu\n", current->tgid, current->pid);
#endif

	return 0;
}

static long vpu_wait_complete(struct device *dev, struct channel_node * const cnode)
{
	long ret = 0;
	struct jz_vpu *vpu = dev_get_drvdata(dev);

hard_vpu_wait_restart:
	ret = wait_for_completion_interruptible_timeout(&vpu->done, msecs_to_jiffies(cnode->mdelay));
	if (ret > 0) {
		ret = 0;
		if ((cnode->codecdir & (1 << 16)) == 0) {
			CLEAR_VPU_BIT(vpu, REG_VPU_SDE_STAT, SDE_STAT_BSEND);
			CLEAR_VPU_BIT(vpu, REG_VPU_DBLK_STAT, DBLK_STAT_DOEND);
		} else if (cnode->codecdir == HWJPEGDEC) {
			vpu->bslen = vpu_readl(vpu, REG_JPGC_MCUS);
		}

		if(cnode->n_flag == 1) {
			reset_ncu();
			vpu_writel(vpu, 0x40070, 0x0);
		}

		dev_dbg(vpu->vpu.dev, "[%d:%d] wait complete\n", current->tgid, current->pid);
	} else if (ret == -ERESTARTSYS) {
		dev_dbg(vpu->vpu.dev, "[%d:%d]:fun:%s,line:%d vpu is interrupt\n", current->tgid, current->pid, __func__, __LINE__);
		goto hard_vpu_wait_restart;
	} else {
		dev_warn(dev, "[%d:%d] wait_for_completion timeout\n", current->tgid, current->pid);
		dev_warn(dev, "vpu_stat = %x\n", vpu_readl(vpu,REG_VPU_STAT));
		dev_warn(dev, "aux_stat = %x\n", vpu_readl(vpu,REG_VPU_AUX_STAT));
		dev_warn(dev, "ret = %ld\n", ret);
		if (vpu_reset(dev) < 0) {
			dev_warn(dev, "vpu reset failed\n");
		}
		ret = -1;
	}
	cnode->output_len = vpu->bslen;
	cnode->status = vpu->status;
	cnode->cmpx = vpu->cmpx;

	//dev_info(dev, "[file:%s,fun:%s,line:%d] ret = %ld, status = %x, bslen = %d, cnode->cmpx=%d\n", __FILE__, __func__, __LINE__, ret, cnode->status, cnode->output_len, cnode->cmpx);

	return ret;
}

static long vpu_suspend(struct device *dev)
{
	struct jz_vpu *vpu = dev_get_drvdata(dev);
	int timeout = 0xffffff;
	volatile unsigned int vpulock = 0;
	unsigned long slock_flag = 0;
	enum jz_vpu_status vpu_status = 0;

	spin_lock_irqsave(&vpu->slock, slock_flag);
	vpu_status = vpu->vpu_status;
	spin_unlock_irqrestore(&vpu->slock, slock_flag);
	if (vpu_status == VPU_STATUS_CLOSE) {
		dev_dbg(dev, "%s(%d)vpu_status is closed\n", __func__, __LINE__);
		return 0;
	}

	//vpu_writel(vpu, REG_SCH_TLBA, clist->tlb_pidmanager->tlbbase);
	vpulock = vpu_readl(vpu, REG_VPU_LOCK);
	if ( vpulock & VPU_LOCK_END_FLAG) {
		while(!(vpu_readl(vpu, REG_VPU_STAT) & VPU_STAT_ENDF) && timeout--);
		if (!timeout) {
			dev_warn(dev, "vpu suspend timeout\n");
			return -1;
		}
		SET_VPU_BIT(vpu, REG_VPU_LOCK, VPU_LOCK_WAIT_OK);
		CLEAR_VPU_BIT(vpu, REG_VPU_LOCK, VPU_LOCK_END_FLAG);
	}

    clk_disable(vpu->clk);
	clk_disable(vpu->clk_gate);

	return 0;
}

static long vpu_resume(struct device *dev)
{
	struct jz_vpu *vpu = dev_get_drvdata(dev);
	unsigned long slock_flag = 0;
	enum jz_vpu_status vpu_status = 0;

	spin_lock_irqsave(&vpu->slock, slock_flag);
	vpu_status = vpu->vpu_status;
	spin_unlock_irqrestore(&vpu->slock, slock_flag);
	if (vpu_status == VPU_STATUS_CLOSE) {
		dev_dbg(dev, "%s(%d)vpu_status is closed\n", __func__, __LINE__);
		return 0;
	}

    clk_set_rate(vpu->clk,350000000);
    clk_enable(vpu->clk);
#ifdef CONFIG_BOARD_T10_FPGA
	vpu_clk_gate_enable(vpu->vpu.idx);
#else
	clk_enable(vpu->clk_gate);
#endif

	return 0;
}

static struct vpu_ops vpu_ops = {
	.owner		= THIS_MODULE,
	.open		= vpu_open,
	.release	= vpu_release,
	.start_vpu	= vpu_start_vpu,
	.wait_complete	= vpu_wait_complete,
	.reset		= vpu_reset,
	.suspend	= vpu_suspend,
	.resume		= vpu_resume,
};

static irqreturn_t vpu_interrupt(int irq, void *dev)
{
	struct jz_vpu *vpu = dev;
	unsigned int vpu_stat;
	unsigned long vflag = 0;

	vpu_stat = vpu_readl(vpu,REG_VPU_STAT);

	spin_lock_irqsave(&vpu->slock, vflag);
	CLEAR_VPU_BIT(vpu, REG_SCH_GLBC, SCH_INTE_MASK);
	if(vpu_stat) {
		if(vpu_stat & VPU_STAT_ENDF) {
			if(vpu_stat & VPU_STAT_JPGEND) {
				dev_dbg(vpu->vpu.dev, "JPG successfully done!\n");
				vpu->bslen = vpu_readl(vpu, REG_VPU_JPGC_STAT) & 0xffffff;
				vpu->cmpx = 0;
				CLEAR_VPU_BIT(vpu,REG_VPU_JPGC_STAT,JPGC_STAT_ENDF);
			} else {
				dev_dbg(vpu->vpu.dev, "SCH successfully done!\n");
				vpu->bslen = vpu_readl(vpu, REG_VPU_ENC_LEN);
				vpu->cmpx = vpu_readl(vpu, REG_EFE_SSAD);
				CLEAR_VPU_BIT(vpu,REG_VPU_SDE_STAT,SDE_STAT_BSEND);
				CLEAR_VPU_BIT(vpu,REG_VPU_DBLK_STAT,DBLK_STAT_DOEND);
			}
			vpu->status = vpu_stat;
			complete(&vpu->done);
		} else {
			check_vpu_status(VPU_STAT_SLDERR, "SHLD error!\n");
			check_vpu_status(VPU_STAT_TLBERR, "TLB error! Addr is 0x%08x\n",
					 vpu_readl(vpu,REG_VPU_STAT));
			check_vpu_status(VPU_STAT_BSERR, "BS error!\n");
			check_vpu_status(VPU_STAT_ACFGERR, "ACFG error!\n");
			check_vpu_status(VPU_STAT_TIMEOUT, "TIMEOUT error!\n");
			CLEAR_VPU_BIT(vpu,REG_VPU_GLBC,
				      (VPU_INTE_ACFGERR |
				       VPU_INTE_TLBERR |
				       VPU_INTE_BSERR |
				       VPU_INTE_ENDF
					      ));
			vpu->bslen = 0;
			vpu->status = vpu_stat;
			vpu->cmpx = 0;
		}
	} else {
		if(vpu_readl(vpu,REG_VPU_AUX_STAT) & AUX_STAT_MIRQP) {
			dev_dbg(vpu->vpu.dev, "AUX successfully done!\n");
			CLEAR_VPU_BIT(vpu,REG_VPU_AUX_STAT,AUX_STAT_MIRQP);
		} else {
			dev_dbg(vpu->vpu.dev, "illegal interrupt happened!\n");
		}
	}

	spin_unlock_irqrestore(&vpu->slock, vflag);

	return IRQ_HANDLED;
}

static int vpu_probe(struct platform_device *pdev)
{
	int ret;
	struct resource	*regs;
	struct jz_vpu *vpu;

	vpu = kzalloc(sizeof(struct jz_vpu), GFP_KERNEL);
	if (!vpu) {
		dev_err(&pdev->dev, "kzalloc vpu space failed\n");
		ret = -ENOMEM;
		goto err_kzalloc_vpu;
	}

	vpu->vpu.vpu_id = JZ_NVPU_ID | (1 << pdev->id);
	vpu->vpu.idx = pdev->id;
	sprintf(vpu->name, "vpu%d", pdev->id);

	vpu->irq = platform_get_irq(pdev, 0);
	if(vpu->irq < 0) {
		dev_err(&pdev->dev, "get irq failed\n");
		ret = vpu->irq;
		goto err_get_vpu_irq;
	}

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs) {
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

	vpu->clk_gate = clk_get(&pdev->dev, vpu->name);
	if (IS_ERR(vpu->clk_gate)) {
		dev_err(&pdev->dev, "clk_gate get failed\n");
		ret = PTR_ERR(vpu->clk_gate);
		goto err_get_vpu_clk_gate;
	}

	if (pdev->id == 0) {
		vpu->clk = clk_get(&pdev->dev,"cgu_vpu");
		if (IS_ERR(vpu->clk)) {
			dev_err(&pdev->dev, "clk get failed\n");
			ret = PTR_ERR(vpu->clk);
			goto err_get_vpu_clk_cgu;
		}
		clk_set_rate(vpu->clk,350000000);
	}

	spin_lock_init(&vpu->slock);
	mutex_init(&vpu->mutex);
	init_completion(&vpu->done);

	ret = request_irq(vpu->irq, vpu_interrupt, IRQF_DISABLED, vpu->name, vpu);
	if (ret < 0) {
		dev_err(&pdev->dev, "request_irq failed\n");
		goto err_vpu_request_irq;
	}
	disable_irq_nosync(vpu->irq);
#if 0
	vpu->cpm_pwc = cpm_pwc_get(PWC_VPU);
	if(!vpu->cpm_pwc) {
		dev_err(&pdev->dev, "get %s fail!\n",PWC_VPU);
		goto err_vpu_request_power;
	}
#endif

	vpu->vpu_status = VPU_STATUS_CLOSE;
	vpu->vpu.dev = &pdev->dev;
	vpu->vpu.ops = &vpu_ops;

	if ((ret = vpu_register(&vpu->vpu.vlist)) < 0) {
		dev_err(&pdev->dev, "vpu_register failed\n");
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
err_vpu_request_irq:
	if (pdev->id == 0) {
		clk_put(vpu->clk);
	}
err_get_vpu_clk_cgu:
	clk_put(vpu->clk_gate);
err_get_vpu_clk_gate:
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
	struct jz_vpu *vpu = platform_get_drvdata(dev);

	vpu_unregister(&vpu->vpu.vlist);
	//cpm_pwc_put(vpu->cpm_pwc);
	free_irq(vpu->irq, vpu);
    clk_put(vpu->clk);
	clk_put(vpu->clk_gate);
	iounmap(vpu->iomem);
	kfree(vpu);

	return 0;
}

static int vpu_suspend_platform(struct platform_device *dev, pm_message_t state)
{
	struct jz_vpu *vpu = platform_get_drvdata(dev);

	return vpu_suspend(vpu->vpu.dev);
}

static int vpu_resume_platform(struct platform_device *dev)
{
	struct jz_vpu *vpu = platform_get_drvdata(dev);

	return vpu_resume(vpu->vpu.dev);
}

static struct platform_driver jz_vpu_driver = {
	.probe		= vpu_probe,
	.remove		= vpu_remove,
	.driver		= {
		.name	= "jz-vpu",
	},
	.suspend	= vpu_suspend_platform,
	.resume		= vpu_resume_platform,
};

static int __init vpu_init(void)
{
	return platform_driver_register(&jz_vpu_driver);
}

static void __exit vpu_exit(void)
{
	platform_driver_unregister(&jz_vpu_driver);
}

module_init(vpu_init);
module_exit(vpu_exit);

MODULE_DESCRIPTION("JZ4780 VPU driver");
MODULE_AUTHOR("Justin <ptkang@ingenic.cn>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("20140830");
