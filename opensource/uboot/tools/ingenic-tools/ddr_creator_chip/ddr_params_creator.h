#ifndef	__DDR_PARAMS_CREATOR_H__
#define __DDR_PARAMS_CREATOR_H__
#ifndef CONFIG_SPL_BUILD
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define debug(fmt, ...)
#else
#include <common.h>
#include <generated/ddr_reg_values.h>
#endif	/* define CONFIG_SPL_BUILD */

#include <config.h>
#include <ddr/ddr_common.h>

#define ARRAY_SIZE(A) (sizeof(A)/sizeof(A[0]))

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a > b ? b : a)

#define out_error(fmt,y...) printf("#error "fmt,##y)
#define out_warn(fmt,y...) printf("/* "fmt "*/\n\n",##y)

#define BETWEEN(T, MIN, MAX)				\
	do{						\
		if (T < MIN) {				\
			out_error("%d is too small and check %s %d!\n",T,__FILE__,__LINE__); \
			assert(1);			\
		} else if (T > MAX) {			\
			out_error("%d is too big and check %s %d!\n",T,__FILE__,__LINE__);	\
			assert(1);			\
		}					\
	}while(0)

#define ASSERT_MASK(T,BIT_NUM) do{					\
		if(T < 0) {						\
			out_error("(%d)timing too small and check %s %d!\n",T,__FILE__,__LINE__); \
			assert(1);					\
		}							\
		if(T > (1 << (BIT_NUM + 1)) - 1){			\
			out_error("(%d)timing too big and check %s %d!\n",T,__FILE__,__LINE__); \
			assert(1);					\
		}							\
	}while(0)

#define DDRC_TIMING_SET(n,param,name,bitnum)				\
	do{								\
		int tmp;						\
		tmp = ps2cycle_ceil(p->private_params.param.name,1);	\
		ASSERT_MASK(tmp,bitnum);				\
		ddrc->timing##n.b.name = tmp;				\
	}while(0)

#define DDRP_TIMING_SET(n,param,name,bitcount,min,max)			\
	 do{								\
		 int tmp;						\
		 tmp = ps2cycle_ceil(p->private_params.param.name,1);	\
		 ASSERT_MASK(tmp,bitcount);				\
		 BETWEEN(tmp,min,max);					\
		 ddrp->dtpr##n.b.name = tmp;				\
	 }while(0)

struct ddr_creator_ops{
	int type;
	void (*fill_in_params)(struct ddr_params *ddr_params, struct ddr_chip_info *chip);
	void (*ddrc_params_creator)(struct ddrc_reg *ddrc, struct ddr_params *p);
	void (*ddrp_params_creator)(struct ddrp_reg *ddrc, struct ddr_params *p);
};
struct ddr_out_impedance
{
	unsigned int r;  //micro ohm   table should be sort by descanding.
	unsigned int index;
};
struct ddr_out_impedance* find_nearby_impedance(struct ddr_out_impedance *table,int table_size,int r_ohm);
int ps2cycle_ceil(int ps,int div_tck);
void register_ddr_creator(struct ddr_creator_ops *ops);
void lpddr2_creator_init(void);
void lpddr3_creator_init(void);
int init_supported_ddr(void);
void dump_supported_ddr(void);
void dump_timing_reg(void);
void create_supported_ddr_params(struct ddr_reg_value *generated_reg_values);
int create_one_ddr_params(struct ddr_chip_info *chip, struct ddr_reg_value *reg);
void ddr2_creator_init(void);
void ddr3_creator_init(void);

#endif /*__DDR_PARAMS_CREATOR_H__*/
