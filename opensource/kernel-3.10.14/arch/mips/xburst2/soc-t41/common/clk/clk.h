#ifndef _CLK_H_
#define _CLK_H_
#include <linux/init.h>
#include <linux/kernel.h>
struct clk;
struct cpm_pwc;
struct clk_ops {
	int (*enable)(struct clk *,int);
	struct clk* (*get_parent)(struct clk *);
	int (*set_parent)(struct clk *,struct clk *);
	unsigned long (*get_rate)(struct clk *);
	int (*set_rate)(struct clk *,unsigned long);
	int (*set_round_rate)(struct clk *,unsigned long);
};
struct clk {
	const char *name;
	unsigned long rate;
	struct clk *parent;
	unsigned long flags;
#define CLK_FLG_NOALLOC	BIT(0)
#define CLK_FLG_ENABLE	BIT(1)
#define CLK_GATE_BIT(flg)	((flg) >> 24)
#define CLK_FLG_GATE	BIT(2)
#define CLK_CPCCR_NO(flg)	(((flg) >> 24) & 0xff)
#define CLK_FLG_CPCCR	BIT(3)
#define CLK_CGU_NO(flg) 	(((flg) >> 24) & 0xff)
#define CLK_FLG_CGU	BIT(4)
#define CLK_PLL_NO(flg) 	(((flg) >> 24) & 0xff)
#define CLK_FLG_PLL	BIT(5)
#define CLK_PWC_NO(flg) 	(((flg) >> 24) & 0xff)
#define CLK_FLG_PWC	BIT(6)
#define CLK_PARENT(flg) 	(((flg) >> 16) & 0xff)
#define CLK_RELATIVE(flg) 	(((flg) >> 16) & 0xff)
#define CLK_FLG_PARENT	BIT(7)
#define CLK_FLG_RELATIVE BIT(8)
	struct clk_ops *ops;
	atomic_t count;
	int init_state;
	struct clk *source;
	struct clk *child;
	unsigned int CLK_ID;
};

enum {
	CLK_ID_EXT     = 0,
	CLK_ID_EXT0,
#define CLK_NAME_EXT0		"ext0"
	CLK_ID_EXT1,
#define CLK_NAME_EXT1		"ext1"
	CLK_ID_OTGPHY,
#define CLK_NAME_OTGPHY	    "otg_phy"

	CLK_ID_PLL,
	CLK_ID_APLL,
#define CLK_NAME_APLL		"apll"
	CLK_ID_MPLL,
#define CLK_NAME_MPLL		"mpll"
	CLK_ID_VPLL,
#define CLK_NAME_VPLL		"vpll"
	CLK_ID_SCLKA,
#define CLK_NAME_SCLKA		"sclka"
	/**********************************************************************************/
	CLK_ID_CPPCR,
	CLK_ID_CCLK,
#define CLK_NAME_CCLK		"cclk"
	CLK_ID_L2CLK,
#define CLK_NAME_L2CLK		"l2clk"
	CLK_ID_H0CLK,
#define CLK_NAME_H0CLK		"h0clk"
	CLK_ID_H2CLK,
#define CLK_NAME_H2CLK		"h2clk"
	CLK_ID_PCLK,
#define CLK_NAME_PCLK		"pclk"
	CLK_ID_MSC,
#define CLK_NAME_MSC		"msc"
	/**********************************************************************************/
/**********************************************************************************/
	CLK_ID_CGU,
	CLK_ID_CGU_ISPM,
#define CLK_NAME_CGU_ISPM		"cgu_ispm"
	CLK_ID_CGU_ISPA,
#define CLK_NAME_CGU_ISPA		"cgu_ispa"
	CLK_ID_CGU_ISPS,
#define CLK_NAME_CGU_ISPS		"cgu_isps"
	CLK_ID_CGU_CIM,
#define CLK_NAME_CGU_CIM		"cgu_cim"
	CLK_ID_CGU_MSC_MUX,
#define CLK_NAME_CGU_MSC_MUX	"cgu_msc_mux"
	CLK_ID_CGU_SSI,
#define CLK_NAME_CGU_SSI		"cgu_ssi"
	CLK_ID_CGU_I2S_SPK,
#define CLK_NAME_CGU_I2S_SPK	"cgu_i2s_spk"
	CLK_ID_CGU_I2S_MIC,
#define CLK_NAME_CGU_I2S_MIC	"cgu_i2s_mic"
	CLK_ID_CGU_MSC1,
#define CLK_NAME_CGU_MSC1		"cgu_msc1"
	CLK_ID_CGU_MSC0,
#define CLK_NAME_CGU_MSC0		"cgu_msc0"
	CLK_ID_CGU_LCD,
#define CLK_NAME_CGU_LCD		"cgu_lcd"
	CLK_ID_CGU_MACPHY,
#define CLK_NAME_CGU_MACPHY		"cgu_macphy"
	CLK_ID_CGU_VPU,
#define CLK_NAME_CGU_VPU		"cgu_vpu"
	CLK_ID_CGU_DDR,
#define CLK_NAME_CGU_DDR		"cgu_ddr"
	CLK_ID_CGU_RSA,
#define CLK_NAME_CGU_RSA		"cgu_rsa"
	CLK_ID_CGU_SFC0,
#define CLK_NAME_CGU_SFC0		"cgu_sfc0"
	CLK_ID_CGU_SFC1,
#define CLK_NAME_CGU_SFC1		"cgu_sfc1"
	CLK_ID_CGU_LDC,
#define CLK_NAME_CGU_LDC		"cgu_ldc"
	CLK_ID_CGU_BT0,
#define CLK_NAME_CGU_BT0		"cgu_bt0"
	CLK_ID_CGU_PWM,
#define CLK_NAME_CGU_PWM		"cgu_pwm"

/**********************************************************************************/
	CLK_ID_DEVICES,
	CLK_ID_DDR,
#define CLK_NAME_DDR		"ddr"
	CLK_ID_TCU,
#define CLK_NAME_TCU		"tcu"
	CLK_ID_RTC,
#define CLK_NAME_RTC		"rtc"
	CLK_ID_DES,
#define CLK_NAME_DES		"des"
	CLK_ID_RSA,
#define CLK_NAME_RSA		"rsa"
	CLK_ID_VO,
#define CLK_NAME_VO			"vo"
	CLK_ID_CSI,
#define CLK_NAME_CSI		"csi"

	CLK_ID_LCD,
#define CLK_NAME_LCD		"lcd"
	CLK_ID_ISP,
#define CLK_NAME_ISP		"isp"
	CLK_ID_PDMA,
#define CLK_NAME_PDMA		"pdma"
	CLK_ID_SFC0,
#define CLK_NAME_SFC0		"sfc0"
	CLK_ID_SSI1,
#define CLK_NAME_SSI1       "ssi1"
	CLK_ID_UART5,
#define CLK_NAME_UART5		"uart5"
	CLK_ID_UART4,
#define CLK_NAME_UART4		"uart4"
	CLK_ID_UART3,
#define CLK_NAME_UART3		"uart3"
	CLK_ID_UART2,
#define CLK_NAME_UART2		"uart2"
	CLK_ID_UART1,
#define CLK_NAME_UART1		"uart1"
	CLK_ID_UART0,
#define CLK_NAME_UART0		"uart0"
	CLK_ID_SADC,
#define CLK_NAME_SADC		"sadc"
	CLK_ID_DMIC,
#define CLK_NAME_DMIC		"dmic"
	CLK_ID_AIC,
#define CLK_NAME_AIC		"aic"
	CLK_ID_SSI_SLV,
#define CLK_NAME_SSI_SLV	"ssi_slv"
	CLK_ID_I2C2,
#define CLK_NAME_I2C2		"i2c2"
	CLK_ID_I2C1,
#define CLK_NAME_I2C1		"i2c1"
	CLK_ID_I2C0,
#define CLK_NAME_I2C0		"i2c0"
	CLK_ID_SSI0,
#define CLK_NAME_SSI0		"ssi0"
	CLK_ID_MSC1,
#define CLK_NAME_MSC1		"msc1"
	CLK_ID_MSC0,
#define CLK_NAME_MSC0		"msc0"
	CLK_ID_OTG,
#define CLK_NAME_OTG		"otg1"
	CLK_ID_HASH,
#define CLK_NAME_HASH		"hash"
	CLK_ID_EFUSE,
#define CLK_NAME_EFUSE		"efuse"
	CLK_ID_NEMC,
#define CLK_NAME_NEMC		"nemc"

	CLK_ID_IVDC,
#define CLK_NAME_IVDC		"ivdc"
	CLK_ID_CPU,
#define CLK_NAME_CPU		"cpu"
	CLK_ID_APB0,
#define CLK_NAME_APB0		"apb0"
	CLK_ID_JPEG,
#define CLK_NAME_JPEG		"jpeg"
	CLK_ID_LDC,
#define CLK_NAME_LDC		"ldc"
	CLK_ID_SFC1,
#define CLK_NAME_SFC1		"sfc1"
	CLK_ID_SYS_OST,
#define CLK_NAME_SYS_OST	"sys_ost"
	CLK_ID_AHB0,
#define CLK_NAME_AHB0	    "ahb0"
	CLK_ID_BUSMON,
#define CLK_NAME_BUSMON	    "busmonitor"
	CLK_ID_I2D,
#define CLK_NAME_I2D	    "i2d"
	CLK_ID_PWM,
#define CLK_NAME_PWM	    "pwm"
	CLK_ID_DBOX,
#define CLK_NAME_DBOX	    "drawbox"

	CLK_ID_AES,
#define CLK_NAME_AES	    "aes"
	CLK_ID_GMAC,
#define CLK_NAME_GMAC	    "gmac"
	CLK_ID_LZMA,
#define CLK_NAME_LZMA	    "lzma"
	CLK_ID_IPU,
#define CLK_NAME_IPU	    "ipu"
	CLK_ID_DTRNG,
#define CLK_NAME_DTRNG	    "dtrng"
	CLK_ID_AVPU,
#define CLK_NAME_AVPU	    "avpu"
	CLK_ID_STOP,
	CLK_ID_INVALID,
};


enum {	//SHOULD BE CHECKED!!
	CGU_ISPM, CGU_ISPA, CGU_ISPS, CGU_CIM, CGU_MSC_MUX,  CGU_SSI,
	CGU_I2S_SPK, CGU_I2S_MIC,CGU_MSC1,CGU_MSC0,CGU_LCD,
	CGU_MACPHY,CGU_VPU, CGU_DDR,CGU_RSA,CGU_SFC0,CGU_SFC1,
	CGU_LDC,CGU_BT0, CGU_PWM,
};

enum {
	CDIV = 0,L2CDIV,H0DIV,H2DIV,PDIV,SCLKA,
};
struct  freq_udelay_jiffy {
	unsigned int  max_num;
	unsigned int  cpufreq;
	unsigned int  udelay_val;
	unsigned int  loops_per_jiffy;
};

int get_clk_sources_size(void);
struct clk *get_clk_from_id(int clk_id);
int get_clk_id(struct clk *clk);
//typedef int (*DUMP_CALLBACK)(char *, const char *, ...);
//int dump_out_clk(char *str);

void __init init_cgu_clk(struct clk *clk);
void __init init_cpccr_clk(struct clk *clk);
void __init init_ext_pll(struct clk *clk);
void __init init_gate_clk(struct clk *clk);
void __init cpm_pwc_init(void);
void __init init_pwc_clk(struct clk *clk);
int cpm_pwc_enable_ctrl(struct clk *clk,int on);
void cpm_pwc_suspend(void);
void cpm_pwc_resume(void);
#endif /* _CLK_H_ */
