/* arch/mips/xburst/soc-m200/common/cpu_core_voltage.c
 * CPU core power manager driver for Ingenic's SoC m200.
 *
 * Copyright (C) 2015 Ingenic Semiconductor Co., Ltd.
 *	http://www.ingenic.com
 * Author: Qiu dongsheng <dongsheng.qiu@ingenic.com>, Liu bo <bo.liu@ingenic.com>
 * Modify by: Sun Jiwei <jwsun@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/syscore_ops.h>
#include <linux/regulator/consumer.h>
#include <linux/suspend.h>

#include <jz_notifier.h>
#include "./clk/clk.h"

#define MIN_VOLTAGE	(1025000)
#define MAX_VOLTAGE	(1150000)
#define CPU_CORE_NAME	"cpu_core"

static struct cpu_core_voltage {
	struct workqueue_struct *down_wq;
	struct delayed_work	vol_down_work;
	struct jz_notifier	clk_prechange;
	struct jz_notifier	clkgate_change;
	struct regulator	*core_vcc;
	unsigned int		current_vol;
	unsigned int		target_vol;
	unsigned int		current_rate;
	unsigned int		gpu_adj;
	unsigned int		vpu_adj;
	unsigned int		msc_adj;
	unsigned int		all_adj;
	atomic_t		qwork_atomic;
	int			is_suspend;
	struct mutex		mutex;
} cpu_core_vol;

static struct vol_freq {
	unsigned int u_vol;
	unsigned int k_freq;
} vol_freq[] = {
	{1050000, 300000},
	{1075000, 600000},
	{1100000, 800000},
        {1125000, 1008000},
};

static unsigned int get_vol_from_freq(struct cpu_core_voltage *pcore,
		unsigned int k_freq)
{
	int i;
	int u_vol;
	unsigned int vol_freq_entry = ARRAY_SIZE(vol_freq) - 1;

	u_vol = MAX_VOLTAGE;

	if (k_freq < vol_freq[0].k_freq) {
		u_vol = MIN_VOLTAGE;
	} else if (k_freq == vol_freq[0].k_freq) {
		u_vol = vol_freq[0].u_vol;
	} else {
		for(i = 0; i < vol_freq_entry; i++) {
			if((k_freq <= vol_freq[i + 1].k_freq) &&
					(k_freq > vol_freq[i].k_freq)) {
				u_vol = vol_freq[i + 1].u_vol;
				break;
			}
		}
	}

	u_vol += pcore->all_adj;

	if(u_vol > MAX_VOLTAGE)
		u_vol = MAX_VOLTAGE;

	pr_debug("k_freq:%d, voltage:%d, adj:%d\n",
			k_freq, u_vol, pcore->all_adj);
	return u_vol;
}


static void cpu_core_vol_down(struct work_struct *work)
{
	struct cpu_core_voltage *pcore = container_of(to_delayed_work(work),
			struct cpu_core_voltage, vol_down_work);

#if 0
	/* The step-down voltage way, for example, the current voltage
	 * is 1150mV, and the target voltage is 1100mV, the step-down
	 * voltage way is 1150mV->1125mV->1100mV. This way make the
	 * efficiency of voltage regulation very low, so it is not recommended */
	while(pcore->current_vol > pcore->target_vol) {
		vol = pcore->current_vol - 25000;
		if(vol > pcore->target_vol) {
			regulator_set_voltage(pcore->core_vcc,vol,vol);
		}

		pcore->current_vol = vol;
	}
#endif

	if(pcore->current_vol != pcore->target_vol) {
		regulator_set_voltage(pcore->core_vcc, pcore->target_vol,
				pcore->target_vol);
		pcore->current_vol = pcore->target_vol;
		pr_debug("voltage down:%d mv\n", pcore->current_vol / 1000);
	}
}

static void cpu_core_change(struct cpu_core_voltage *pcore,
		unsigned int target_vol)
{
	unsigned int current_vol;

	if (!pcore->is_suspend) {
		if (atomic_read(&pcore->qwork_atomic)) {
			cancel_delayed_work_sync(&pcore->vol_down_work);
			atomic_set(&pcore->qwork_atomic, 0);
		}

		current_vol = pcore->current_vol;
		pcore->target_vol = target_vol;

		if (target_vol > current_vol)
		{
#if 0
			/* The step-up voltage way, for example, the current
			 * voltage is 1100mV, and the target voltage is
			 * 1150mV, the step-up voltage way is
			 * 1100mV->1125mV->1150mV. This way make the efficiency
			 * of voltage regulation very low, so it is not recommended */
			while(target_vol > current_vol) {
				current_vol += 25000;
				if(target_vol >= current_vol) {
					regulator_set_voltage(pcore->core_vcc,
							current_vol, current_vol);
				}
			}
			if(target_vol != current_vol) {
				regulator_set_voltage(pcore->core_vcc,
						target_vol,target_vol);
			}
#endif
			regulator_set_voltage(pcore->core_vcc,
					target_vol, target_vol);
			pcore->current_vol = target_vol;
			pr_debug("voltage up:%d mv\n",
					pcore->current_vol / 1000);
		} else if (target_vol < current_vol) {
			queue_delayed_work(pcore->down_wq,
					&pcore->vol_down_work,
					msecs_to_jiffies(1000));
			atomic_set(&pcore->qwork_atomic, 1);
		}
	}
}

static int clkgate_change_notify(struct jz_notifier *notify,void *v)
{
	unsigned int val = (unsigned int)v;
	unsigned int on = val & 0x80000000;
	unsigned int clk_id = val & (~0x80000000);
	unsigned int target_vol;
	struct cpu_core_voltage *pcore =
		container_of(notify, struct cpu_core_voltage, clkgate_change);

	switch(clk_id) {
	case CLK_ID_GPU:
		mutex_lock(&pcore->mutex);
		pcore->gpu_adj = on ? 100000:0;
		pcore->all_adj = pcore->gpu_adj + pcore->vpu_adj +
			pcore->msc_adj;
		target_vol = get_vol_from_freq(pcore,
				pcore->current_rate / 1000);
		if (target_vol != pcore->target_vol) {
			cpu_core_change(pcore, target_vol);
		}
		mutex_unlock(&pcore->mutex);
		break;
	case CLK_ID_VPU:
		mutex_lock(&pcore->mutex);
		pcore->vpu_adj = on ? 50000:0;
		pcore->all_adj = pcore->gpu_adj + pcore->vpu_adj +
			pcore->msc_adj;
		target_vol = get_vol_from_freq(pcore,
				pcore->current_rate / 1000);
		if (target_vol != pcore->target_vol) {
			cpu_core_change(pcore, target_vol);
		}
		mutex_unlock(&pcore->mutex);
		break;
	case CLK_ID_MSC:
		mutex_lock(&pcore->mutex);
		pcore->msc_adj = on ? 100000:0;
		pcore->all_adj = pcore->gpu_adj + pcore->vpu_adj +
			pcore->msc_adj;
		target_vol = get_vol_from_freq(pcore,
				pcore->current_rate / 1000);
		if (target_vol != pcore->target_vol) {
			cpu_core_change(pcore, target_vol);
		}
		mutex_unlock(&pcore->mutex);
		break;
	}

	return NOTIFY_OK;
}

static int clk_prechange_notify(struct jz_notifier *notify,void *v)
{
	unsigned int target_vol;
	struct cpu_core_voltage *pcore =
		container_of(notify, struct cpu_core_voltage, clk_prechange);
	struct clk_notify_data *clk_data = (struct clk_notify_data *)v;

	mutex_lock(&pcore->mutex);
	target_vol = get_vol_from_freq(pcore, clk_data->target_rate / 1000);
	pcore->current_rate = clk_data->target_rate;
	cpu_core_change(pcore, target_vol);
	mutex_unlock(&pcore->mutex);

	return NOTIFY_OK;
}

static int cpu_core_sleep_pm_callback(struct notifier_block *nfb,unsigned long action,void *ignored)
{
	struct cpu_core_voltage *pcore = &cpu_core_vol;
	unsigned int max_vol = MAX_VOLTAGE;

	switch (action) {
	case PM_SUSPEND_PREPARE:
		pcore->is_suspend = 1;
		cancel_delayed_work_sync(&pcore->vol_down_work);
		mutex_lock(&pcore->mutex);
		regulator_set_voltage(pcore->core_vcc, max_vol, max_vol);
		regulator_put(pcore->core_vcc);
		mutex_unlock(&pcore->mutex);
		break;
	case PM_POST_SUSPEND:
		mutex_lock(&pcore->mutex);
		pcore->is_suspend = 0;
		pcore->core_vcc = regulator_get(NULL, CPU_CORE_NAME);
		mutex_unlock(&pcore->mutex);
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block cpu_core_sleep_pm_notifier = {
	.notifier_call = cpu_core_sleep_pm_callback,
	.priority = 0,
};

static int __init cpu_core_voltage_init(void)
{
	struct clk *cpu_clk;
	struct cpu_core_voltage *pcore = &cpu_core_vol;

	pcore->down_wq = alloc_workqueue("voltage_down", 0, 1);
	if (!pcore->down_wq)
		return -ENOMEM;
	INIT_DELAYED_WORK(&pcore->vol_down_work,
			cpu_core_vol_down);

	pcore->core_vcc = regulator_get(NULL, CPU_CORE_NAME);
	if(IS_ERR(pcore->core_vcc))
		return -1;

	cpu_clk = clk_get(NULL, CLK_NAME_CCLK);
	if (IS_ERR(cpu_clk))
		return -1;
	pcore->current_rate = clk_get_rate(cpu_clk);
	clk_put(cpu_clk);
	if(pcore->current_rate <= 0) {
		return -1;
	}

	pcore->msc_adj = 0;
	pcore->gpu_adj = 0;
	pcore->vpu_adj = 0;
	pcore->all_adj = 0;

	atomic_set(&pcore->qwork_atomic, 0);

	mutex_init(&pcore->mutex);
	pcore->current_vol = regulator_get_voltage(pcore->core_vcc);
	pcore->target_vol = pcore->current_vol;
	pcore->clk_prechange.jz_notify = clk_prechange_notify;
	pcore->clk_prechange.level = NOTEFY_PROI_HIGH;
	pcore->clk_prechange.msg = JZ_CLK_PRECHANGE;
	jz_notifier_register(&pcore->clk_prechange, NOTEFY_PROI_HIGH);

	pcore->clkgate_change.jz_notify = clkgate_change_notify;
	pcore->clkgate_change.level = NOTEFY_PROI_HIGH;
	pcore->clkgate_change.msg = JZ_CLKGATE_CHANGE;
	jz_notifier_register(&pcore->clkgate_change, NOTEFY_PROI_HIGH);

	register_pm_notifier(&cpu_core_sleep_pm_notifier);

	return 0;
}

static void __exit cpu_core_voltage__exit(void)
{
	struct cpu_core_voltage *pcore = &cpu_core_vol;
	destroy_workqueue(pcore->down_wq);
	jz_notifier_unregister(&pcore->clkgate_change, NOTEFY_PROI_HIGH);
	jz_notifier_unregister(&pcore->clk_prechange, NOTEFY_PROI_HIGH);
}

module_init(cpu_core_voltage_init);
module_exit(cpu_core_voltage__exit);
MODULE_AUTHOR("Qiu Dongsheng<dongsheng.qiu@ingenic.com>");
MODULE_DESCRIPTION("cpufreq voltage driver for Ingenic SoC M200");
MODULE_LICENSE("GPL");
