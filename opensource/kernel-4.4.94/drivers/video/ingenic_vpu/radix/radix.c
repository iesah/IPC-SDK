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
#include "radix.h"
//#include "radix_hevc_enc.h"

//#define DUMP_RADIX_REG

struct jz_vpu_radix {
	struct vpu          vpu;
	char                name[16];
	int                 irq;
	void __iomem        *iomem;
	struct clk          *clk;
	struct clk          *clk_gate;
	struct clk          *ahb1_gate;
	enum jz_vpu_status  vpu_status;
	struct completion   done;
	spinlock_t          slock;
	struct mutex        mutex;
	pid_t               owner_pid;
	unsigned int        status;
	unsigned int        bslen;
	void*               cpm_pwc;
	unsigned int        cmpx;
};

static long vpu_reset(struct device *dev)
{
	struct jz_vpu_radix *vpu = dev_get_drvdata(dev);
	int timeout = 0x7fff;
	unsigned int srbc = cpm_inl(CPM_SRBC);

	cpm_set_bit(CPM_RADIX_STP(vpu->vpu.idx), CPM_SRBC);
	while (!(cpm_inl(CPM_SRBC) & (1 << CPM_RADIX_ACK(vpu->vpu.idx))) && --timeout) udelay(20);

	if (timeout == 0) {
		dev_warn(vpu->vpu.dev, "[%d:%d] wait stop ack timeout\n",
			 current->tgid, current->pid);
		cpm_outl(srbc, CPM_SRBC);
		return -1;
	} else {
		cpm_outl(srbc | (1 << CPM_RADIX_SR(vpu->vpu.idx)), CPM_SRBC);
		cpm_outl(srbc, CPM_SRBC);
	}

	return 0;
}

static long vpu_open(struct device *dev)
{
	struct jz_vpu_radix *vpu = dev_get_drvdata(dev);
	unsigned long slock_flag = 0;

	spin_lock_irqsave(&vpu->slock, slock_flag);
	if (cpm_inl(CPM_OPCR) & OPCR_IDLE)
		return -EBUSY;

    clk_enable(vpu->clk);
	clk_enable(vpu->ahb1_gate);
	clk_enable(vpu->clk_gate);
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
	struct jz_vpu_radix *vpu = dev_get_drvdata(dev);
	unsigned long slock_flag = 0;

	spin_lock_irqsave(&vpu->slock, slock_flag);
	disable_irq_nosync(vpu->irq);

	__asm__ __volatile__ (
			"mfc0  $2, $16,  7   \n\t"
			"andi  $2, $2, 0xbf \n\t"
			"mtc0  $2, $16,  7  \n\t"
			"nop                  \n\t");

	cpm_clear_bit(CPM_RADIX_SR(vpu->vpu.idx),CPM_OPCR);

    clk_disable(vpu->clk);
	clk_disable(vpu->clk_gate);
	clk_disable(vpu->ahb1_gate);
	//cpm_pwc_disable(vpu->cpm_pwc);
	/* Clear completion use_count here to avoid a unhandled irq after vpu off */
	vpu->done.done = 0;
	vpu->vpu_status = VPU_STATUS_CLOSE;
	spin_unlock_irqrestore(&vpu->slock, slock_flag);

	dev_dbg(dev, "[%d:%d] close\n", current->tgid, current->pid);

	return 0;
}

#ifdef DUMP_RADIX_REG
static void radix_show_internal_state(struct jz_vpu_radix *vpu)
{
    unsigned int i;

    //md
    unsigned int md_read_reg0  = vpu_readl(vpu, RADIX_MD_BASE + (8 << 2));
    unsigned int md_read_reg1  = vpu_readl(vpu, RADIX_MD_BASE + (24 << 2));
    unsigned int md_read_reg2  = vpu_readl(vpu, RADIX_MD_BASE + (25 << 2));
    unsigned int md_read_reg3  = vpu_readl(vpu, RADIX_MD_BASE + (26 << 2));
    unsigned int md_read_reg4  = vpu_readl(vpu, RADIX_MD_BASE + (27 << 2));
    unsigned int md_read_reg5  = vpu_readl(vpu, RADIX_MD_BASE + (28 << 2));
    unsigned int md_read_reg6  = vpu_readl(vpu, RADIX_MD_BASE + (29 << 2));
    unsigned int md_read_reg7  = vpu_readl(vpu, RADIX_MD_BASE + (30 << 2));

    //bc
    unsigned int bc_read_reg0  = vpu_readl(vpu, RADIX_BC_BASE + 0xc);
    unsigned int bc_read_reg1  = vpu_readl(vpu, RADIX_BC_BASE + 0x10);
    unsigned int bc_read_reg2  = vpu_readl(vpu, RADIX_BC_BASE + 0x14);
    unsigned int bc_read_reg3  = vpu_readl(vpu, RADIX_BC_BASE + 0x18);
    unsigned int bc_read_reg4  = vpu_readl(vpu, RADIX_BC_BASE + 0x1c);
    unsigned int bc_read_reg5  = vpu_readl(vpu, RADIX_BC_BASE + 0x20);
    unsigned int bc_read_reg6  = vpu_readl(vpu, RADIX_BC_BASE + 0x8);

    //sde
    unsigned int sde_read_reg0  = vpu_readl(vpu, RADIX_SDE_BASE + 0x14);
    unsigned int sde_read_reg1  = vpu_readl(vpu, RADIX_SDE_BASE + 0x18);
    unsigned int sde_read_reg2  = vpu_readl(vpu, RADIX_SDE_BASE + 0x1c);
    unsigned int sde_read_reg3  = vpu_readl(vpu, RADIX_SDE_BASE + 0x20);
    unsigned int sde_read_reg4  = vpu_readl(vpu, RADIX_SDE_BASE + 0x24);
    unsigned int sde_read_reg5  = vpu_readl(vpu, RADIX_SDE_BASE + 0x28);
    unsigned int sde_read_reg6  = vpu_readl(vpu, RADIX_SDE_BASE + 0x2c);
    unsigned int sde_read_reg7  = vpu_readl(vpu, RADIX_SDE_BASE + 0x30);
    unsigned int sde_read_reg8  = vpu_readl(vpu, RADIX_SDE_BASE + 0x34);
    unsigned int sde_read_reg9  = vpu_readl(vpu, RADIX_SDE_BASE + 0x38);
    unsigned int sde_read_reg10  = vpu_readl(vpu, RADIX_SDE_BASE + 0x3c);

    //stc
    unsigned int stc_read_reg0  = vpu_readl(vpu, RADIX_STC_BASE + 0x14);
    unsigned int stc_read_reg1  = vpu_readl(vpu, RADIX_STC_BASE + 0x18);
    unsigned int stc_read_reg2  = vpu_readl(vpu, RADIX_STC_BASE + 0x1c);
    unsigned int stc_read_reg3  = vpu_readl(vpu, RADIX_STC_BASE + 0x20);
    unsigned int stc_read_reg4  = vpu_readl(vpu, RADIX_STC_BASE + 0x24);
    unsigned int stc_read_reg5  = vpu_readl(vpu, RADIX_STC_BASE + 0x28);
    unsigned int stc_read_reg6  = vpu_readl(vpu, RADIX_STC_BASE + 0x2c);

    //dt
    unsigned int dt_read_reg0  = vpu_readl(vpu, RADIX_DT_BASE + (9 << 2));
    unsigned int dt_read_reg1  = vpu_readl(vpu, RADIX_DT_BASE + (8 << 2));
    unsigned int dt_read_reg2  = vpu_readl(vpu, RADIX_DT_BASE + (6 << 2));
    unsigned int dt_read_reg3  = vpu_readl(vpu, RADIX_DT_BASE + (5 << 2));
    unsigned int dt_read_reg4  = vpu_readl(vpu, RADIX_DT_BASE + (4 << 2));
    unsigned int dt_read_reg5  = vpu_readl(vpu, RADIX_DT_BASE + (3 << 2));
    unsigned int dt_read_reg6  = vpu_readl(vpu, RADIX_DT_BASE + (2 << 2));
    unsigned int dt_read_reg7  = vpu_readl(vpu, RADIX_DT_BASE + (1 << 2));

    //sao
    unsigned int sao_read_reg0  = vpu_readl(vpu, RADIX_SAO_BASE + (0x3c));
    unsigned int sao_read_reg1  = vpu_readl(vpu, RADIX_SAO_BASE + (0x40));
    unsigned int sao_read_reg2  = vpu_readl(vpu, RADIX_SAO_BASE + (0x44));
    unsigned int sao_read_reg3  = vpu_readl(vpu, RADIX_SAO_BASE + (0x48));

    //ipred
    unsigned int ipred_read_reg12  = vpu_readl(vpu, RADIX_IPRED_BASE + (3 << 2));
    unsigned int ipred_read_reg0  = vpu_readl(vpu, RADIX_IPRED_BASE + (6 << 2));
    unsigned int ipred_read_reg1  = vpu_readl(vpu, RADIX_IPRED_BASE + (7 << 2));
    unsigned int ipred_read_reg2  = vpu_readl(vpu, RADIX_IPRED_BASE + (8 << 2));
    unsigned int ipred_read_reg3  = vpu_readl(vpu, RADIX_IPRED_BASE + (9 << 2));
    unsigned int ipred_read_reg4  = vpu_readl(vpu, RADIX_IPRED_BASE + (0xa << 2));
    unsigned int ipred_read_reg5  = vpu_readl(vpu, RADIX_IPRED_BASE + (0xb << 2));
    unsigned int ipred_read_reg6  = vpu_readl(vpu, RADIX_IPRED_BASE + (0xc << 2));
    unsigned int ipred_read_reg7  = vpu_readl(vpu, RADIX_IPRED_BASE + (0xd << 2));
    unsigned int ipred_read_reg8  = vpu_readl(vpu, RADIX_IPRED_BASE + (0xe << 2));
    unsigned int ipred_read_reg9  = vpu_readl(vpu, RADIX_IPRED_BASE + (0xf << 2));
    unsigned int ipred_read_reg10  = vpu_readl(vpu, RADIX_IPRED_BASE + (0x10 << 2));
    unsigned int ipred_read_reg11  = vpu_readl(vpu, RADIX_IPRED_BASE + (0x11 << 2));

    //tfm
    unsigned int tfm_read_reg0  = vpu_readl(vpu, RADIX_TFM_BASE + (2 << 2));
    unsigned int tfm_read_reg1  = vpu_readl(vpu, RADIX_TFM_BASE + (3 << 2));
    unsigned int tfm_read_reg2  = vpu_readl(vpu, RADIX_TFM_BASE + (4 << 2));
    unsigned int tfm_read_reg3  = vpu_readl(vpu, RADIX_TFM_BASE + (5 << 2));
    unsigned int tfm_read_reg4  = vpu_readl(vpu, RADIX_TFM_BASE + (6 << 2));
    unsigned int tfm_read_reg5  = vpu_readl(vpu, RADIX_TFM_BASE + (7 << 2));
    unsigned int tfm_read_reg6  = vpu_readl(vpu, RADIX_TFM_BASE + (8 << 2));
    unsigned int tfm_read_reg7  = vpu_readl(vpu, RADIX_TFM_BASE + (9 << 2));
    unsigned int tfm_read_reg8  = vpu_readl(vpu, RADIX_TFM_BASE + (10 << 2));
    unsigned int tfm_read_reg9  = vpu_readl(vpu, RADIX_TFM_BASE + (11 << 2));
    unsigned int tfm_read_reg10  = vpu_readl(vpu, RADIX_TFM_BASE + (12 << 2));

    //mce
    unsigned int mce_read_reg0  = vpu_readl(vpu, RADIX_MCE_BASE + (0x20 << 2));
    unsigned int mce_read_reg1  = vpu_readl(vpu, RADIX_MCE_BASE + (0x21 << 2));
    unsigned int mce_read_reg2  = vpu_readl(vpu, RADIX_MCE_BASE + (0x22 << 2));
    unsigned int mce_read_reg3  = vpu_readl(vpu, RADIX_MCE_BASE + (0x23 << 2));
    unsigned int mce_read_reg4  = vpu_readl(vpu, RADIX_MCE_BASE + (0x24 << 2));
    unsigned int mce_read_reg5  = vpu_readl(vpu, RADIX_MCE_BASE + (0x25 << 2));
    unsigned int mce_read_reg6  = vpu_readl(vpu, RADIX_MCE_BASE + (0x26 << 2));
    unsigned int mce_read_reg7  = vpu_readl(vpu, RADIX_MCE_BASE + (0x27 << 2));
    unsigned int mce_read_reg8  = vpu_readl(vpu, RADIX_MCE_BASE + (0x28 << 2));
    unsigned int mce_read_reg9  = vpu_readl(vpu, RADIX_MCE_BASE + (0x29 << 2));
    unsigned int mce_read_reg10  = vpu_readl(vpu, RADIX_MCE_BASE + (0x2a << 2));
    unsigned int mce_read_reg11  = vpu_readl(vpu, RADIX_MCE_BASE + (0x2b << 2));
    unsigned int mce_read_reg12  = vpu_readl(vpu, RADIX_MCE_BASE + (0x2c << 2));
    unsigned int mce_read_reg13  = vpu_readl(vpu, RADIX_MCE_BASE + (0x2d << 2));
    unsigned int mce_read_reg14  = vpu_readl(vpu, RADIX_MCE_BASE + (0x2e << 2));
    unsigned int mce_read_reg15  = vpu_readl(vpu, RADIX_MCE_BASE + (0x2f << 2));
    unsigned int mce_read_reg16  = vpu_readl(vpu, RADIX_MCE_BASE + (0x30 << 2));
    unsigned int mce_read_reg17  = vpu_readl(vpu, RADIX_MCE_BASE + (0x31 << 2));
    unsigned int mce_read_reg18  = vpu_readl(vpu, RADIX_MCE_BASE + (0x32 << 2));
    unsigned int mce_read_reg19  = vpu_readl(vpu, RADIX_MCE_BASE + (0x33 << 2));
    unsigned int mce_read_reg20  = vpu_readl(vpu, RADIX_MCE_BASE + (0x34 << 2));
    unsigned int mce_read_reg21  = vpu_readl(vpu, RADIX_MCE_BASE + (0x35 << 2));
    unsigned int mce_read_reg22  = vpu_readl(vpu, RADIX_MCE_BASE + (0x38 << 2));
    unsigned int mce_read_reg23  = vpu_readl(vpu, RADIX_MCE_BASE + (0x39 << 2));
    unsigned int mce_read_reg24  = vpu_readl(vpu, RADIX_MCE_BASE + (0x40 << 2));
    unsigned int mce_read_reg25  = vpu_readl(vpu, RADIX_MCE_BASE + (0x41 << 2));
    unsigned int mce_read_reg26  = vpu_readl(vpu, RADIX_MCE_BASE + (0x42 << 2));
    unsigned int mce_read_reg27  = vpu_readl(vpu, RADIX_MCE_BASE + (0x43 << 2));
    unsigned int mce_read_reg28  = vpu_readl(vpu, RADIX_MCE_BASE + (0x44 << 2));
    unsigned int mce_read_reg29  = vpu_readl(vpu, RADIX_MCE_BASE + (0x45 << 2));

    //efe
    unsigned int efe_read_reg0  = vpu_readl(vpu, RADIX_EFE_BASE + (102 << 2));
    unsigned int efe_read_reg1  = vpu_readl(vpu, RADIX_EFE_BASE + (103 << 2));
    unsigned int efe_read_reg2  = vpu_readl(vpu, RADIX_EFE_BASE + (104 << 2));
    unsigned int efe_read_reg3  = vpu_readl(vpu, RADIX_EFE_BASE + (105 << 2));
    unsigned int efe_read_reg4  = vpu_readl(vpu, RADIX_EFE_BASE + (106 << 2));

    //dblk
    unsigned int dblk_read_reg0  = vpu_readl(vpu, RADIX_DBLK_BASE + (7 << 2));
    unsigned int dblk_read_reg1  = vpu_readl(vpu, RADIX_DBLK_BASE + (8 << 2));
    unsigned int dblk_read_reg2  = vpu_readl(vpu, RADIX_DBLK_BASE + (9 << 2));
    unsigned int dblk_read_reg3  = vpu_readl(vpu, RADIX_DBLK_BASE + (10 << 2));
    unsigned int dblk_read_reg4  = vpu_readl(vpu, RADIX_DBLK_BASE + (11 << 2));
    //debug
    unsigned int debug_read_reg0  = vpu_readl(vpu, RADIX_CFGC_BASE + (RADIX_REG_CFGC_ACM_CTRL << 2));

    /* dump code */
    //md
    printk("md_read_reg : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            md_read_reg0, md_read_reg1, md_read_reg2, md_read_reg3,
            md_read_reg4, md_read_reg5, md_read_reg6, md_read_reg7);

    //md_new
    printk("md_read_reg_new :\n");
    printk(" 0x%x \t",vpu_readl(vpu, RADIX_MD_BASE + (15 << 2)));
    for(i=0;i<6;i++)
        printk(" 0x%x \t",vpu_readl(vpu, RADIX_MD_BASE + ((18+i) << 2)));
    printk("\n");
    for(i=0;i<15;i++)
        printk(" 0x%x \t",vpu_readl(vpu, RADIX_MD_BASE + ((32+i) << 2)));
    printk("\n");

    //bc
    printk("bc_read_reg : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            bc_read_reg0, bc_read_reg1, bc_read_reg2, bc_read_reg3,
            bc_read_reg4, bc_read_reg5, bc_read_reg6);

    //sde
    printk("sde_read_reg : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            sde_read_reg0, sde_read_reg1, sde_read_reg2, sde_read_reg3,
            sde_read_reg4, sde_read_reg5, sde_read_reg6, sde_read_reg7,
            sde_read_reg8, sde_read_reg9, sde_read_reg10);

    //stc
    printk("stc_read_reg : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            stc_read_reg0, stc_read_reg1, stc_read_reg2, stc_read_reg3,
            stc_read_reg4, stc_read_reg5, stc_read_reg6);

    //dt
    printk("dt_read_reg : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            dt_read_reg0, dt_read_reg1, dt_read_reg2, dt_read_reg3,
            dt_read_reg4, dt_read_reg5, dt_read_reg6, dt_read_reg7);

    //sao
    printk("sao_read_reg : 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            sao_read_reg0, sao_read_reg1, sao_read_reg2, sao_read_reg3);

    //ipred
    printk("ipred_read_reg_new :\n");
    for(i=0;i<10;i++)
        printk(" 0x%x \t",vpu_readl(vpu, RADIX_IPRED_BASE + ((0+i) << 2)));
    printk("\n");
    for(i=0;i<3;i++)
        printk(" 0x%x \t",vpu_readl(vpu, RADIX_IPRED_BASE + ((0x20+i) << 2)));
    printk("\n");

    printk("ipred_read_reg : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            ipred_read_reg0, ipred_read_reg1, ipred_read_reg2, ipred_read_reg3,
            ipred_read_reg4, ipred_read_reg5, ipred_read_reg6, ipred_read_reg7,
            ipred_read_reg8, ipred_read_reg9, ipred_read_reg10, ipred_read_reg11, ipred_read_reg12);

    //tfm
    printk("tfm_read_reg : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            tfm_read_reg0, tfm_read_reg1, tfm_read_reg2, tfm_read_reg3,
            tfm_read_reg4, tfm_read_reg5, tfm_read_reg6, tfm_read_reg7,
            tfm_read_reg8, tfm_read_reg9, tfm_read_reg10);
    //mce
    printk("mce_read_reg_new :\n");
    printk(" 0x%x \n",vpu_readl(vpu, RADIX_MCE_BASE + ((0x0) << 2)));
    printk(" 0x%x \n",vpu_readl(vpu, RADIX_MCE_BASE + ((0x4) << 2)));
    printk(" 0x%x \n",vpu_readl(vpu, RADIX_MCE_BASE + ((0x10) << 2)));
    printk(" 0x%x \n",vpu_readl(vpu, RADIX_MCE_BASE + ((0x11) << 2)));

    for(i=0;i<3;i++)
        printk(" 0x%x \t",vpu_readl(vpu, RADIX_MCE_BASE + ((0x18+i) << 2)));
    printk("\n");

    printk("mce_read_reg : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            mce_read_reg0, mce_read_reg1, mce_read_reg2, mce_read_reg3,
            mce_read_reg4, mce_read_reg5, mce_read_reg6, mce_read_reg7,
            mce_read_reg8, mce_read_reg9);

    printk("mce_read_reg1 : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            mce_read_reg10, mce_read_reg11, mce_read_reg12, mce_read_reg13,
            mce_read_reg14, mce_read_reg15, mce_read_reg16, mce_read_reg17,
            mce_read_reg18, mce_read_reg19);

    printk("mce_read_reg2 : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            mce_read_reg20, mce_read_reg21, mce_read_reg22, mce_read_reg23,
            mce_read_reg24, mce_read_reg25, mce_read_reg26, mce_read_reg27,
            mce_read_reg28, mce_read_reg29);

    printk("mce_read_reg_new :\n");
    for(i=0;i<4;i++)
        printk(" 0x%x \t",vpu_readl(vpu, RADIX_MCE_BASE + ((0x46+i) << 2)));
    printk("\n");
    printk(" 0x%x \t",vpu_readl(vpu, RADIX_MCE_BASE + ((0x50) << 2)));
    printk("\n");

    //efe
    printk("efe_read_reg : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            efe_read_reg0, efe_read_reg1, efe_read_reg2, efe_read_reg3,
            efe_read_reg4);

    printk("efe_read_reg_new0 :\n");
    for(i=0;i<8;i++)
        printk(" 0x%x \t",vpu_readl(vpu, RADIX_EFE_BASE + ((28+i) << 2)));
    printk("\n");

    printk("efe_read_reg_new1 : 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            vpu_readl(vpu, RADIX_EFE_BASE + (36 << 2)), vpu_readl(vpu, RADIX_EFE_BASE + (37 << 2)),
            vpu_readl(vpu, RADIX_EFE_BASE + (38 << 2)), vpu_readl(vpu, RADIX_EFE_BASE + (39 << 2)));
    printk("efe_read_reg_new2 : 0x%x \t 0x%x \t\n",
            vpu_readl(vpu, RADIX_EFE_BASE + (48 << 2)),
            vpu_readl(vpu, RADIX_EFE_BASE + (49 << 2)));
    printk("efe_read_reg_new3 : 0x%x \t 0x%x \t\n",
            vpu_readl(vpu, RADIX_EFE_BASE + 0x44 ),
            vpu_readl(vpu, RADIX_EFE_BASE + 0x48 ));

    //dblk
    printk("dblk_read_reg : 0x%x \t 0x%x \t 0x%x \t 0x%x \t 0x%x \t\n",
            dblk_read_reg0, dblk_read_reg1, dblk_read_reg2, dblk_read_reg3,
            dblk_read_reg4);

    //debug
    printk("debug_read_reg : 0x%x \t\n",debug_read_reg0);
}
#endif

static long vpu_start(struct device *dev, const struct channel_node * const cnode)
{
	struct jz_vpu_radix *vpu = dev_get_drvdata(dev);
	//struct channel_list *clist = list_entry(cnode->clist, struct channel_list, list);

#ifdef DUMP_RADIX_REG
	dev_info(vpu->vpu.dev, "------%s(%d)radix_show_internal_state start------\n", __func__, __LINE__);
	radix_show_internal_state(vpu);
	dev_info(vpu->vpu.dev, "------%s(%d)radix_show_internal_state end------\n", __func__, __LINE__);
#endif

    vpu_writel(vpu, RADIX_CFGC_BASE  + RADIX_REG_CFGC_GLB_CTRL, ((1<<16) - 1) | RADIX_CFGC_INTE_TLBERR
            | RADIX_CFGC_INTE_AMCERR | RADIX_CFGC_INTE_BSERR | RADIX_CFGC_INTE_ENDF);

    vpu_writel(vpu, RADIX_CFGC_BASE + RADIX_REG_CFGC_ACM_CTRL, RADIX_VDMA_ACFG_DHA(cnode->dma_addr) | RADIX_VDMA_ACFG_RUN);

	dev_dbg(vpu->vpu.dev, "[%d:%d] start vpu\n", current->tgid, current->pid);

	return 0;
}

static long vpu_wait_complete(struct device *dev, struct channel_node * const cnode)
{
	long ret = 0;
	struct jz_vpu_radix *vpu = dev_get_drvdata(dev);

hard_vpu_wait_restart:
	ret = wait_for_completion_interruptible_timeout(&vpu->done, msecs_to_jiffies(cnode->mdelay));
	if (ret > 0) {
		dev_dbg(vpu->vpu.dev, "[%d:%d] wait complete finish\n", current->tgid, current->pid);
		ret = 0;
	} else if (ret == -ERESTARTSYS) {
		dev_dbg(vpu->vpu.dev, "[%d:%d]:fun:%s,line:%d vpu is interrupt\n", current->tgid, current->pid, __func__, __LINE__);
		goto hard_vpu_wait_restart;
	} else {
		dev_warn(dev, "[%d:%d] wait_for_completion timeout\n", current->tgid, current->pid);
		dev_warn(dev, "vpu_stat = %x\n", vpu_readl(vpu, RADIX_CFGC_BASE + RADIX_REG_CFGC_STAT));
		dev_warn(dev, "ret = %ld\n", ret);
#ifdef DUMP_RADIX_REG
		dev_info(vpu->vpu.dev, "------%s(%d)radix_show_internal_state start------\n", __func__, __LINE__);
		radix_show_internal_state(vpu);
		dev_info(vpu->vpu.dev, "------%s(%d)radix_show_internal_state end------\n", __func__, __LINE__);
#endif
		if (vpu_reset(dev) < 0) {
			dev_warn(dev, "vpu reset failed\n");
		}
		ret = -1;
	}
	cnode->output_len = vpu->bslen;
	cnode->status = vpu->status;

	//dev_info(dev, "[file:%s,fun:%s,line:%d] ret = %ld, status = %x, bslen = %d\n", __FILE__, __func__, __LINE__, ret, cnode->status, cnode->output_len);

	return ret;
}

static long vpu_suspend(struct device *dev)
{
	return 0;
}

static long vpu_resume(struct device *dev)
{
	return 0;
}

static struct vpu_ops vpu_ops = {
	.owner		= THIS_MODULE,
	.open		= vpu_open,
	.release	= vpu_release,
	.start_vpu	= vpu_start,
	.wait_complete	= vpu_wait_complete,
	.reset		= vpu_reset,
	.suspend	= vpu_suspend,
	.resume		= vpu_resume,
};

static irqreturn_t vpu_interrupt(int irq, void *dev)
{
	struct jz_vpu_radix *vpu = dev;
	unsigned int vpu_stat;
	unsigned long vflag = 0;

	vpu_stat = vpu_readl(vpu, RADIX_CFGC_BASE + RADIX_REG_CFGC_STAT);

	spin_lock_irqsave(&vpu->slock, vflag);
	CLEAR_VPU_BIT(vpu, RADIX_CFGC_BASE + RADIX_REG_CFGC_GLB_CTRL, RADIX_CFGC_INTE_MASK);
	if(vpu_stat) {
		if(vpu_stat & RADIX_CFGC_INTST_RADIX_END) {
            dev_dbg(vpu->vpu.dev, "radix successfully done!\n");
            vpu->bslen = vpu_readl(vpu, RADIX_SDE_BASE + RADIX_REG_SDE_SLBL);
			vpu->status = vpu_stat;
			CLEAR_VPU_BIT(vpu, RADIX_CFGC_BASE + RADIX_REG_CFGC_STAT, RADIX_CFGC_STAT_GLB_END | RADIX_CFGC_INTST_RADIX_END);
			complete(&vpu->done);
		} else {
			check_vpu_status(RADIX_CFGC_INTST_ACM_ERR, "vdma error!\n");
			vpu->bslen = 0;
			vpu->status = vpu_stat;
		}
	}

	spin_unlock_irqrestore(&vpu->slock, vflag);

	return IRQ_HANDLED;
}

static int vpu_probe(struct platform_device *pdev)
{
	int ret;
	struct resource	*regs;
	struct jz_vpu_radix *vpu;

	vpu = kzalloc(sizeof(struct jz_vpu_radix), GFP_KERNEL);
	if (!vpu) {
		dev_err(&pdev->dev, "kzalloc vpu space failed\n");
		ret = -ENOMEM;
		goto err_kzalloc_vpu;
	}

	vpu->vpu.vpu_id = VPU_RADIX_ID | (1 << pdev->id);
	vpu->vpu.idx = pdev->id;
	sprintf(vpu->name, "%s%d", pdev->name, pdev->id);

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

	vpu->ahb1_gate = clk_get(&pdev->dev, "ahb1");
	if (IS_ERR(vpu->ahb1_gate)) {
		dev_err(&pdev->dev, "ahb1_gate get failed\n");
		ret = PTR_ERR(vpu->ahb1_gate);
		goto err_get_ahb1_clk_gate;
	}

	vpu->clk_gate = clk_get(&pdev->dev, vpu->name);
	if (IS_ERR(vpu->clk_gate)) {
		dev_err(&pdev->dev, "clk_gate get failed\n");
		ret = PTR_ERR(vpu->clk_gate);
		goto err_get_vpu_clk_gate;
	}

    vpu->clk = clk_get(&pdev->dev,"cgu_vpu");
    if (IS_ERR(vpu->clk)) {
        dev_err(&pdev->dev, "clk get failed\n");
        ret = PTR_ERR(vpu->clk);
        goto err_get_vpu_clk_cgu;
    }
    clk_set_rate(vpu->clk,350000000);

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
    clk_put(vpu->clk);
err_get_vpu_clk_cgu:
	clk_put(vpu->clk_gate);
err_get_vpu_clk_gate:
	clk_put(vpu->ahb1_gate);
err_get_ahb1_clk_gate:
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
	struct jz_vpu_radix *vpu = platform_get_drvdata(dev);

	vpu_unregister(&vpu->vpu.vlist);
	//cpm_pwc_put(vpu->cpm_pwc);
	free_irq(vpu->irq, vpu);
    clk_put(vpu->clk);
	clk_put(vpu->clk_gate);
	clk_put(vpu->ahb1_gate);
	iounmap(vpu->iomem);
	kfree(vpu);

	return 0;
}

static int vpu_suspend_platform(struct platform_device *dev, pm_message_t state)
{
	struct jz_vpu_radix *vpu = platform_get_drvdata(dev);

	return vpu_suspend(vpu->vpu.dev);
}

static int vpu_resume_platform(struct platform_device *dev)
{
	struct jz_vpu_radix *vpu = platform_get_drvdata(dev);

	return vpu_resume(vpu->vpu.dev);
}

static struct platform_driver jz_vpu_driver = {
	.probe		= vpu_probe,
	.remove		= vpu_remove,
	.driver		= {
		.name	= "radix",
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

MODULE_DESCRIPTION("T30 VPU radix driver");
MODULE_AUTHOR("Justin <ptkang@ingenic.cn>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("20171010");
