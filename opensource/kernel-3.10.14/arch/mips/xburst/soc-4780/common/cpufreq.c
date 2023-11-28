/*
 *  CPU frequency scaling for JZ47XX SOCS
 *
 *  Copyright (C) 2012 Ingenic Corporation
 *  Written by ztyan<ztyan@ingenic.cn>
 *
 *  Based on cpu-sa1110.c, Copyright (C) 2001 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* #define DEBUG */
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

#include <asm/cpu.h>

#define CPUFREQ_NR	20
#define MIN_FREQ	200000
#define MIN_VOLT	1200000
#define MAX_MPLL_DIV	6			/* Max CPU MPLL div */
#ifndef CONFIG_TVOL_L
#define tVOL_L		30			/* ms of voltage latency */
#else
#define tVOL_L		CONFIG_TVOL_L
#endif
#ifndef CONFIG_TVOL_H
#define tVOL_H		500			/* ms of voltage hold time */
#else
#define tVOL_H		CONFIG_TVOL_H
#endif
#ifndef CONFIG_DETE_PERIOD
#define DETE_PERIOD	30000			/* freq detect period */
#else
#define DETE_PERIOD	CONFIG_DETE_PERIOD
#endif
#ifndef CONFIG_HIGH_THRESHOLD
#define HIGH_THRESHOLD	30			/* percent of high load threshold */
#else
#define HIGH_THRESHOLD	CONFIG_HIGH_THRESHOLD
#endif
#ifndef CONFIG_HIFREQ_MINUTE
#define HIFREQ_MINUTE	30			/* high freq last time */
#else
#define HIFREQ_MINUTE	CONFIG_HIFREQ_MINUTE
#endif
#ifndef CONFIG_MAX_APLL_FREQ
#define MAX_APLL_FREQ	1728000			/* max cpufreq from APLL */
#else
#define MAX_APLL_FREQ	CONFIG_MAX_APLL_FREQ
#endif
#ifndef CONFIG_LOW_APLL_FREQ
#define LOW_APLL_FREQ	700000			/* low cpufreq from APLL */
#else
#define LOW_APLL_FREQ	CONFIG_LOW_APLL_FREQ
#endif
#ifndef CONFIG_APLL_FREQ_STEP
#define APLL_FREQ_STEP	96000			/* step of APLL freq */
#else
#define APLL_FREQ_STEP	CONFIG_APLL_FREQ_STEP
#endif

struct cpufreq_config {
	int apll_on;
	int nocpufreq;
};
#ifdef CONFIG_SMP
struct lpj_info {
	unsigned long	ref;
	unsigned int	freq;
};

static DEFINE_PER_CPU(struct lpj_info, lpj_ref);
static struct lpj_info global_lpj_ref;
#endif

static struct clk *cpu_clk;
static struct regulator *cpu_regulator;
static spinlock_t freq_lock;
static struct delayed_work vol_work;
static int vol_v;
static unsigned int freq_gate;
static unsigned int freq_high;
static struct task_struct *freq_thread;
static unsigned long long timer_start = 0;
static unsigned long long timer_end = 0;
static unsigned long long radical_time = 0;
static int radical_cnt = 0;
struct cpufreq_frequency_table __attribute__((weak)) freq_table[CPUFREQ_NR] = {{0,0}};
static struct cpufreq_config freq_config = {
	.apll_on	= 0,
	.nocpufreq	= 0,
};

static int __init cpufreq_config_setup(char *str)
{
	if (!strcmp(str, "apll_on"))
		freq_config.apll_on = 1;
	if (!strcmp(str, "nocpufreq"))
		freq_config.nocpufreq = 1;

	return 1;
}
__setup("cpufreq_config=", cpufreq_config_setup);

unsigned long __attribute__((weak)) core_reg_table[12][2] = {
	{ 1584000,1400000 }, // >= 1.548 GHz - 1.40V
	{ 1300000,1350000 }, // >= 1.300 GHz - 1.35V
	{ 1200000,1200000 }, // >= 1.200 GHz - 1.20V
	{MIN_FREQ,MIN_VOLT},
};

unsigned long regulator_find_voltage(int freqs)
{
	int i = 0;

	while(core_reg_table[i][0] && core_reg_table[i][1]) {
		if(freqs >= core_reg_table[i][0])
			return core_reg_table[i][1];
		i++;
	}
	return 0;
}

static int freq_table_prepare(void)
{
	struct clk *sclka, *mpll, *apll, *cparent;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int sclka_rate, mpll_rate, apll_rate;
	unsigned int max_rate = 0;
	unsigned int talbe_tmp[CPUFREQ_NR];

	sclka = clk_get(NULL,"sclka");
	if (IS_ERR(sclka)) {
		return -EINVAL;
	}

	mpll = clk_get(NULL,"mpll");
	if (IS_ERR(mpll)) {
		goto mpll_err;
	}

	apll = clk_get(NULL,"apll");
	if (IS_ERR(apll)) {
		goto apll_err;
	}

	sclka_rate = clk_get_rate(sclka) / 1000;
	apll_rate = clk_get_rate(apll) / 1000;
	mpll_rate = clk_get_rate(mpll) / 1000;
	cparent = clk_get_parent(cpu_clk);

	freq_gate = mpll_rate;
	freq_high = mpll_rate;

	if (freq_table[0].frequency != 0)
		goto done;

	if (clk_is_enabled(apll) && freq_config.apll_on) {
		freq_high = apll_rate;
		max_rate = MAX_APLL_FREQ;
		while (max_rate >= (freq_gate + APLL_FREQ_STEP)) {
			talbe_tmp[i++] = max_rate;
			max_rate -= APLL_FREQ_STEP;
		}

		max_rate = freq_gate;
		do {
			max_rate -= APLL_FREQ_STEP;
		} while (max_rate >= 1000000);

		while ((max_rate > freq_gate / 2 + APLL_FREQ_STEP)
		       && (max_rate > LOW_APLL_FREQ)) {
			talbe_tmp[i++] = max_rate;
			max_rate -= APLL_FREQ_STEP;
		}
	} else {
		max_rate = mpll_rate;
	}

	for (j=1;(i<CPUFREQ_NR) && (mpll_rate/j >= MIN_FREQ)
		     && (j <= MAX_MPLL_DIV);i++) {
		talbe_tmp[i] = mpll_rate / j++;
	}

	talbe_tmp[i] = ~1;

	memset(freq_table,0,sizeof(freq_table));
#define _FREQ_TAB(in, fr)				\
	freq_table[in].index = in;			\
	freq_table[in].frequency = fr
	for (j = 0; j <= i; j++) {
		int k, in = 0;
		unsigned int ft = ~1;
		for (k = 0; k <= i; k++) {
			if (talbe_tmp[k] < ft) {
				ft = talbe_tmp[k];
				in = k;
			}
		}
		talbe_tmp[in] = ~1;
		if (ft != ~1)
			_FREQ_TAB(j, ft);
	}
	_FREQ_TAB(j, CPUFREQ_TABLE_END);
#undef _FREQ_TAB
done:
	clk_put(apll);
	clk_put(sclka);
	clk_put(mpll);
#if 0
	printk("cpufreq table: ");
	for (i = 0; freq_table[i].frequency != CPUFREQ_TABLE_END; i++) {
		printk("%u ", freq_table[i].frequency);
	}
	printk("\n");
#endif
	return 0;
mpll_err:
	clk_put(sclka);
apll_err:
	clk_put(mpll);
	return -EINVAL;
}

static int jz4780_verify_speed(struct cpufreq_policy *policy)
{
	return cpufreq_frequency_table_verify(policy, freq_table);
}

static unsigned int jz4780_getspeed(unsigned int cpu)
{
	if (cpu >= NR_CPUS)
		return 0;

	return clk_get_rate(cpu_clk) / 1000;
}

static int jz4780_target(struct cpufreq_policy *policy,
		unsigned int target_freq,
		unsigned int relation)
{
	unsigned int i;
	int r,ret = 0;
	struct cpufreq_freqs freqs;
	unsigned long freq, flags, volt = 0;
	int this_cpu = smp_processor_id();

	ret = cpufreq_frequency_table_target(policy, freq_table, target_freq,
			relation, &i);
	if (ret) {
		printk("%s: cpu%d: no freq match for %d(ret=%d)\n",
				__func__, policy->cpu, target_freq, ret);
		return ret;
	}
	freqs.new = freq_table[i].frequency;
	if (!freqs.new) {
		printk("%s: cpu%d: no match for freq %d\n", __func__,
				policy->cpu, target_freq);
		return -EINVAL;
	}

	freqs.old = jz4780_getspeed(policy->cpu);
	freqs.cpu = policy->cpu;

	if (freqs.old == freqs.new && policy->cur == freqs.new)
		return ret;

	freq = freqs.new * 1000;
	if ((freqs.old < freq_gate) && (freqs.new > freq_gate))
		freqs.new = freq_gate;

	if (cpu_regulator && (freqs.new > freqs.old)) {
		int vol_t;

		volt = regulator_find_voltage(freqs.new);
		vol_t = vol_v;

		if (volt > vol_v) {
			cancel_delayed_work_sync(&vol_work);
			vol_t = regulator_get_voltage(cpu_regulator);
		}
		vol_v = volt;
		while (vol_t < vol_v) {
			vol_t += 50000;
			r = regulator_set_voltage(cpu_regulator, vol_t, vol_t);
			if (r < 0) {
				pr_err("unable to scale voltage up. volt:%d\n", vol_t);
				freqs.new = freqs.old;
				goto done;
			}
			msleep(tVOL_L);
			if (vol_t >= vol_v)
				break;
		}
	}
	/* notifiers */
	for_each_cpu(i, policy->cpus) {
		freqs.cpu = i;
		cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
	}

	spin_lock_irqsave(&freq_lock, flags);

	ret = clk_set_rate(cpu_clk, freqs.new * 1000);

	freqs.new = jz4780_getspeed(policy->cpu);
	if ((freqs.new > freq_gate) && (freqs.old <= freq_gate)) {
		timer_start = cpu_clock(this_cpu);
	} else if ((freqs.new <= freq_gate) && (freqs.old > freq_gate)){
		timer_end = cpu_clock(this_cpu);
		radical_time += timer_end - timer_start;
	}
#ifdef CONFIG_SMP
	/*
	 * Note that loops_per_jiffy is not updated on SMP systems in
	 * cpufreq driver. So, update the per-CPU loops_per_jiffy value
	 * on frequency transition. We need to update all dependent CPUs.
	 */
	for_each_cpu(i, policy->cpus) {
		struct lpj_info *lpj = &per_cpu(lpj_ref, i);
		if (!lpj->freq) {
			lpj->ref = cpu_data[i].udelay_val;
			lpj->freq = freqs.old;
		}

		cpu_data[i].udelay_val = cpufreq_scale(lpj->ref, lpj->freq, freqs.new);
	}

	/* And don't forget to adjust the global one */
	if (!global_lpj_ref.freq) {
		global_lpj_ref.ref = loops_per_jiffy;
		global_lpj_ref.freq = freqs.old;
	}
	loops_per_jiffy = cpufreq_scale(global_lpj_ref.ref, global_lpj_ref.freq,
			freqs.new);
#endif
	spin_unlock_irqrestore(&freq_lock, flags);
	if (cpu_regulator && (freqs.new < freqs.old)) {
		volt = regulator_find_voltage(freqs.new);
		if (volt < vol_v) {
			vol_v = volt;
			schedule_delayed_work(&vol_work,
					      msecs_to_jiffies(tVOL_H));
		}
	}
done:
	/* notifiers */
	for_each_cpu(i, policy->cpus) {
		freqs.cpu = i;
		cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
	}
	pr_debug("-%d\n", freqs.new);

	return ret;
}

static int freq_write(char *filename, char *data)
{
	int fd = sys_open(filename, O_RDWR, 0);
	unsigned count = strlen(data);

	if (fd < 0) {
		printk(KERN_WARNING "%s: Can not open %s\n",
			__func__, filename);
		return -ENOENT;
	}
	if ((unsigned)sys_write(fd, (char *)data, count) != count) {
		printk(KERN_WARNING "%s: Can not write %s\n",
			__func__, filename);
		return -EIO;
	}
	return 0;
}

static int freq_monitor(void *d)
{
	while (1) {
		int this_cpu;
		unsigned int radical_t;

		spin_lock(&freq_lock);
		this_cpu = smp_processor_id();

		if (timer_end < timer_start) {
			timer_end = cpu_clock(this_cpu);
			radical_time += timer_end - timer_start;
			timer_start = cpu_clock(this_cpu);
		}
		do_div(radical_time, 1000000);
		radical_t = (unsigned int)radical_time;
		if (radical_t > (DETE_PERIOD * HIGH_THRESHOLD / 100)) {
			radical_cnt++;
		} else {
			if (radical_cnt > 0)
				radical_cnt--;
		}
		pr_debug("radical_t=%ums radical_cnt=%d\n", radical_t, radical_cnt);
		radical_time = 0;
		spin_unlock(&freq_lock);

		if (radical_cnt > HIFREQ_MINUTE * 60000 / DETE_PERIOD) {
			char max_freq[10];
			sprintf(max_freq, "%u", freq_gate);
			pr_info("set maxfreq to normal mode\n");
			freq_write("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", max_freq);
			radical_cnt = 0;
		}
		msleep(DETE_PERIOD);
	}
	return 0;
}

static void vol_down_work(struct work_struct *work)
{
	int r;
	int vol_t;

	vol_t = regulator_get_voltage(cpu_regulator);
	if (vol_t > vol_v) {
		vol_t -= 50000;
		r = regulator_set_voltage(cpu_regulator, vol_t, vol_t);
		if (r < 0) {
			pr_err("unable to scale voltage down. volt:%d\n", vol_v);
			return;
		}
		msleep(tVOL_L);
		if (vol_t > vol_v)
			schedule_delayed_work(&vol_work, msecs_to_jiffies(tVOL_L));
	}
}

static int __cpuinit jz4780_cpu_init(struct cpufreq_policy *policy)
{
	if (policy->cpu >= NR_CPUS) {
		return -EINVAL;
	}

	if(cpufreq_frequency_table_cpuinfo(policy, freq_table))
		return -ENODATA;

	cpufreq_frequency_table_get_attr(freq_table, policy->cpu);

	policy->min = policy->cpuinfo.min_freq;
/*
	policy->max = policy->cpuinfo.max_freq;
	policy->max = freq_high;
*/
	policy->max = freq_gate;

	policy->cur = jz4780_getspeed(policy->cpu);

	/*
	 * On JZ47XX SMP configuartion, both processors share the voltage
	 * and clock. So both CPUs needs to be scaled together and hence
	 * needs software co-ordination. Use cpufreq affected_cpus
	 * interface to handle this scenario.
	 */
#ifdef CONFIG_SMP
	policy->shared_type = CPUFREQ_SHARED_TYPE_ANY;
	cpumask_setall(policy->cpus);
#endif

	/* 300us for latency. FIXME: what's the actual transition time? */
	policy->cpuinfo.transition_latency = 500 * 1000;

	return 0;
}

static unsigned int rate;
int jz4780_cpu_suspend(struct cpufreq_policy *policy)
{
	if(cpu_clk) {
		rate = clk_get_rate(cpu_clk);
		clk_set_rate(cpu_clk,MIN_FREQ * 1000);
	}

	return 0;
}

int jz4780_cpu_resume(struct cpufreq_policy *policy)
{
	if(cpu_clk) {
		clk_set_rate(cpu_clk,rate);
	}

	return 0;
}

static struct freq_attr *jz4780_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver jz4780_driver = {
	.name		= "jz4780",
	.flags		= CPUFREQ_STICKY,
	.verify		= jz4780_verify_speed,
	.target		= jz4780_target,
	.get		= jz4780_getspeed,
	.init		= jz4780_cpu_init,
	.suspend	= jz4780_cpu_suspend,
	.resume		= jz4780_cpu_resume,
	.attr		= jz4780_cpufreq_attr,
};

static int __init jz4780_cpufreq_init(void)
{
	if (freq_config.nocpufreq) {
		pr_info("No cpufreq\n");
		return -EPERM;
	}

	cpu_clk = clk_get(NULL, "cclk");

	if (IS_ERR(cpu_clk))
		return PTR_ERR(cpu_clk);
#ifdef CONFIG_CPUFREQ_CHANGE_VCORE
	cpu_regulator = regulator_get(NULL, "vcore");
	if (IS_ERR(cpu_regulator)) {
		pr_warning("%s: unable to get CPU regulator\n", __func__);
		cpu_regulator = NULL;
	} else {
		/*
		 * Ensure physical regulator is present.
		 * (e.g. could be dummy regulator.)
		 */
		vol_v = regulator_get_voltage(cpu_regulator);
		if (vol_v < 0) {
			pr_warn("%s: physical regulator not present for CPU\n",
					__func__);
			regulator_put(cpu_regulator);
			cpu_regulator = NULL;
		}
	}
#else
	cpu_regulator = NULL;
#endif
	spin_lock_init(&freq_lock);

	if(freq_table_prepare())
		return -EINVAL;

	INIT_DELAYED_WORK(&vol_work, vol_down_work);
	freq_thread = kthread_run(freq_monitor, NULL, "freq_monitor");

	return cpufreq_register_driver(&jz4780_driver);
}

static void __exit jz4780_cpufreq_exit(void)
{
	if(cpu_clk)
		clk_put(cpu_clk);
	if(cpu_regulator)
		regulator_put(cpu_regulator);
	cpufreq_unregister_driver(&jz4780_driver);
}

MODULE_AUTHOR("ztyan<ztyan@ingenic.cn>");
MODULE_DESCRIPTION("cpufreq driver for JZ47XX SoCs");
MODULE_LICENSE("GPL");
module_init(jz4780_cpufreq_init);
module_exit(jz4780_cpufreq_exit);

