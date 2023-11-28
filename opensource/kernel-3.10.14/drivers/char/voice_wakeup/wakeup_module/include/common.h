/*
 * common.h
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#ifndef noinline
#define noinline __attribute__((noinline))
#endif

#ifndef __section
# define __section(S) __attribute__((__section__(#S)))
#endif


#define __res_mem		__section(.voice_res)

#define ALIGN_ADDR_WORD(addr)	(void *)((((unsigned int)(addr) >> 2) + 1) << 2)



#define NULL (void *)0

#define REG8(addr)  *((volatile u8 *)(addr))
#define REG16(addr) *((volatile u16 *)(addr))
#define REG32(addr) *((volatile u32 *)(addr))



typedef		char s8;
typedef	unsigned char	u8;
typedef		short s16;
typedef unsigned short	u16;
typedef		int s32;
typedef unsigned int	u32;

#define get_cp0_ebase()		__read_32bit_c0_register($15, 1)

#define __read_32bit_c0_register(source, sel)               \
	({ int __res;                               \
	 if (sel == 0)                           \
	 __asm__ __volatile__(                   \
		 "mfc0\t%0, " #source "\n\t"         \
		 : "=r" (__res));                \
	 else                                \
	 __asm__ __volatile__(                   \
		 ".set\tmips32\n\t"              \
		 "mfc0\t%0, " #source ", " #sel "\n\t"       \
		 ".set\tmips0\n\t"               \
		 : "=r" (__res));                \
	 __res;                              \
	 })


#define CONFIG_SYS_DCACHE_SIZE  (32*1024)
#define CONFIG_SYS_ICACHE_SIZE  (32*1024)
#define CONFIG_SYS_SCACHE_SIZE  (512*1024)
#define CONFIG_SYS_CACHELINE_SIZE   32

#define cache_op(op, addr)      \
	__asm__ __volatile__(       \
			".set   push\n"     \
			".set   noreorder\n"    \
			".set   mips3\n"    \
			"cache  %0, %1\n"   \
			".set   pop\n"      \
			:           \
			: "i" (op), "R" (*(unsigned char *)(addr))  \
			: "memory"                  \
			)




/**
 *      |-------------|     <--- SLEEP_TCSM_BOOTCODE_TEXT
 *      | BOOT CODE   |
 *      |-------------|     <--- SLEEP_TCSM_RESUMECODE_TEXT
 *      |    ...      |
 *      | RESUME CODE |
 *      |    ...      |
 *      |-------------|     <--- SLEEP_TCSM_RESUME_DATA
 *      | RESUME DATA |
 *      |_____________|
 */

#define SLEEP_TCSM_SPACE           0xb3423000
#define SLEEP_TCSM_LEN             4096

#define SLEEP_TCSM_BOOT_LEN        256
#define SLEEP_TCSM_DATA_LEN        64
#define SLEEP_TCSM_RESUME_LEN      (SLEEP_TCSM_LEN - SLEEP_TCSM_BOOT_LEN - SLEEP_TCSM_DATA_LEN)

#define SLEEP_TCSM_BOOT_TEXT       (SLEEP_TCSM_SPACE)
#define SLEEP_TCSM_RESUME_TEXT     (SLEEP_TCSM_BOOT_TEXT + SLEEP_TCSM_BOOT_LEN)
#define SLEEP_TCSM_RESUME_DATA     (SLEEP_TCSM_RESUME_TEXT + SLEEP_TCSM_RESUME_LEN)

#define CPU_RESMUE_SP               0xb3425FFC  /* BANK3~BANK2 */


#define _cpu_switch_restore()						\
	do {											\
		int val = 0;								\
		val = REG32(SLEEP_TCSM_RESUME_DATA + 24);	\
		val |= (7 << 20);							\
		REG32(0xb0000000) = val;					\
		while((REG32(0xB00000D4) & 7))				\
			TCSM_PCHAR('w');						\
	} while(0)

#define _cpu_switch_24MHZ()				\
	do {								\
		REG32(0xb0000000) = 0x95800000;	\
		while((REG32(0xB00000D4) & 7))	\
			    TCSM_PCHAR('w');		\
	} while(0)



/* Choose One Work Mode: */
/*#define CONFIG_CPU_IDLE_SLEEP*/
#define CONFIG_CPU_SWITCH_FREQUENCY





#endif /* __COMMON_H__ */
