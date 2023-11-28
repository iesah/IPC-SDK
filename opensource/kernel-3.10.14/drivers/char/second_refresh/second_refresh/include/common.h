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


#define __second_refresh __second(.second_refresh)

/*#define __res_mem		__section(.voice_res)*/

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

#endif /* __COMMON_H__ */
