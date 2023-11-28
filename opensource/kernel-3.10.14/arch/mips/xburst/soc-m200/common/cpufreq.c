/* arch/mips/xburst/soc-m200/common/cpufreq.c
 *  CPU frequency scaling for Ingeic SOC M200
 *
 * Copyright (C) 2015 Ingenic Semiconductor Co., Ltd.
 *	http://www.ingenic.com
 * Author:	Yan Zhengting <zhengting.yan@ingenic.com>
 *		Qiu Dongsheng <dongsheng.qiu@ingenic.com>
 *		Liu Bo <bo.liu@ingenic.com>
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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/opp.h>
#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>
#include <linux/syscalls.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <jz_notifier.h>
#include "./clk/clk.h"

#define CPUFRQ_MIN  (12000)

/* 40ms for latency. FIXME: what's the actual transition time? */
#define TRANSITION_LATENCY (40 * 1000)

extern struct cpufreq_frequency_table *init_freq_table(unsigned int max_freq,
						       unsigned int min_freq);

static struct jz_cpufreq {
	struct mutex mutex;
	struct clk *cpu_clk;
	struct cpufreq_frequency_table *freq_table;
	struct jz_notifier freq_up_change;
	struct work_struct freq_up_work;
	struct timer_list freq_down_timer;
	struct cpufreq_policy *cur_policy;
} *jz_cpufreq;

static int m200_verify_speed(struct cpufreq_policy *policy)
{
	return cpufreq_frequency_table_verify(policy, jz_cpufreq->freq_table);
}

static unsigned int m200_getspeed(unsigned int cpu)
{
	if (cpu >= NR_CPUS)
		return 0;
	return clk_get_rate(jz_cpufreq->cpu_clk) / 1000;
}
static unsigned int m200_getavg(struct cpufreq_policy *policy,
			   unsigned int cpu)
{
	if (cpu >= NR_CPUS)
		return 0;
	return policy->cur;
}

static int m200_target(struct cpufreq_policy *policy,
			 unsigned int target_freq,
			 unsigned int relation)
{
	int index;
	int ret = 0;
	struct cpufreq_freqs freqs;
	ret = cpufreq_frequency_table_target(policy, jz_cpufreq->freq_table,
			target_freq, relation, &index);
	if (ret) {
		pr_err("%s: cpu%d: no freq match for %d(ret=%d)\n",
		       __func__, policy->cpu, target_freq, ret);
		return ret;
	}

	freqs.new = jz_cpufreq->freq_table[index].frequency;
	freqs.old = m200_getspeed(policy->cpu);
	freqs.cpu = policy->cpu;

	if (freqs.old == freqs.new && policy->cur == freqs.new) {
		return ret;
	}

	mutex_lock(&jz_cpufreq->mutex);
	cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);
	if (!freqs.new) {
		pr_err("%s: cpu%d: no match for freq %d\n", __func__,
		       policy->cpu, target_freq);
		while(1);
	}
	pr_debug("freq.new:%d KHz,old:%d KHz, cur:%d KHz\n",
			freqs.new, freqs.old, policy->cur);
	ret = clk_set_rate(jz_cpufreq->cpu_clk, freqs.new * 1000);
	/* notifiers */
	cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
	mutex_unlock(&jz_cpufreq->mutex);

	return ret;
}

static void freq_down_timer(unsigned long data)
{
	struct cpufreq_policy * policy = jz_cpufreq->cur_policy;
	policy->min = policy->cpuinfo.min_freq;
}

static void freq_up_irq_work(struct work_struct *work)
{
	struct cpufreq_policy * policy = jz_cpufreq->cur_policy;

	del_timer(&jz_cpufreq->freq_down_timer);
	if(policy->min != policy->max){
		policy->min = policy->max;
		policy->governor->governor(policy, CPUFREQ_GOV_LIMITS);
	}
	mod_timer(&jz_cpufreq->freq_down_timer,jiffies +
			msecs_to_jiffies(2000));
}

static int freq_up_change_notify(struct jz_notifier *notify,void *v)
{
	schedule_work(&jz_cpufreq->freq_up_work);
	return NOTIFY_OK;
}

static int __cpuinit m200_cpu_init(struct cpufreq_policy *policy)
{
	unsigned int i, max_freq;

	jz_cpufreq = (struct jz_cpufreq *)kzalloc(sizeof(struct jz_cpufreq),
			GFP_KERNEL);
	if(!jz_cpufreq) {
		pr_err("malloc jz_cpufreq fail!!!\n");
		return -1;
	}

	jz_cpufreq->cpu_clk = clk_get(NULL, CLK_NAME_CCLK);
	if (IS_ERR(jz_cpufreq->cpu_clk))
		goto cpu_clk_err;

	max_freq = clk_get_rate(jz_cpufreq->cpu_clk) / 1000;
	if(max_freq <= 0) {
		pr_err("get cclk max freq fail %d\n", max_freq);
		goto freq_table_err;
	}

	jz_cpufreq->freq_table = init_freq_table(max_freq, CPUFRQ_MIN);
	if(!jz_cpufreq->freq_table) {
		pr_err("get freq table error!!\n");
		goto freq_table_err;
	}

	pr_info("freq table is:");
	for(i = 0; jz_cpufreq->freq_table[i].frequency != CPUFREQ_TABLE_END; i++)
		pr_info(" %d", jz_cpufreq->freq_table[i].frequency);
	pr_info("\n");

	if(cpufreq_frequency_table_cpuinfo(policy, jz_cpufreq->freq_table))
		goto freq_table_err;

	cpufreq_frequency_table_get_attr(jz_cpufreq->freq_table, policy->cpu);
	policy->min = policy->cpuinfo.min_freq;
	policy->max = policy->cpuinfo.max_freq;
	pr_debug("policy min %d max %d\n",policy->min, policy->max);
	policy->cur = m200_getspeed(policy->cpu);
	/*
	 * On JZ47XX SMP configuartion, both processors share the voltage
	 * and clock. So both CPUs needs to be scaled together and hence
	 * needs software co-ordination. Use cpufreq affected_cpus
	 * interface to handle this scenario.
	 */
	policy->shared_type = CPUFREQ_SHARED_TYPE_ANY;
	cpumask_setall(policy->cpus);

	policy->cpuinfo.transition_latency = TRANSITION_LATENCY;

	mutex_init(&jz_cpufreq->mutex);
	INIT_WORK(&jz_cpufreq->freq_up_work, freq_up_irq_work);

	setup_timer(&jz_cpufreq->freq_down_timer,freq_down_timer,0);

	jz_cpufreq->freq_up_change.jz_notify = freq_up_change_notify;
	jz_cpufreq->freq_up_change.level = NOTEFY_PROI_NORMAL;
	jz_cpufreq->freq_up_change.msg = JZ_CLK_CHANGING;
	jz_cpufreq->cur_policy = policy;
	jz_notifier_register(&jz_cpufreq->freq_up_change, NOTEFY_PROI_NORMAL);

	pr_debug("cpu freq init ok!\n");
	return 0;
freq_table_err:
	pr_err("init freq_table_err fail!\n");
	clk_put(jz_cpufreq->cpu_clk);
cpu_clk_err:
	pr_err("init cpu_clk_err fail!\n");
	kfree(jz_cpufreq);
	return -1;
}

static int m200_cpu_suspend(struct cpufreq_policy *policy)
{
	return 0;
}

static int m200_cpu_resume(struct cpufreq_policy *policy)
{
	return 0;
}

static struct freq_attr *m200_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver m200_driver = {
	.name		= "m200",
	.flags		= CPUFREQ_STICKY,
	.verify		= m200_verify_speed,
	.target		= m200_target,
	.getavg         = m200_getavg,
	.get		= m200_getspeed,
	.init		= m200_cpu_init,
	.suspend	= m200_cpu_suspend,
	.resume		= m200_cpu_resume,
	.attr		= m200_cpufreq_attr,
};

static int __init m200_cpufreq_init(void)
{
	return cpufreq_register_driver(&m200_driver);
}
module_init(m200_cpufreq_init);

MODULE_AUTHOR("ztyan<ztyan@ingenic.cn>");
MODULE_DESCRIPTION("cpufreq driver for JZ47XX SoCs");
MODULE_LICENSE("GPL");
