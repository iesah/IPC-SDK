#include <asm/addrspace.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>

	.section	".text.jzsoc"
NESTED(__jzsoc_secondary_start, 16, sp)

	//.set	mips32
	.set	noreorder

	/* cause and status register */
#define STATUS_BITS (ST0_CU0)
	mtc0	zero, CP0_CAUSE
	li	v0, STATUS_BITS
	mtc0	v0, CP0_STATUS
	nop

	/* kseg0 cache attribute */
	mfc0	v0, $16, 0
	ori	v0, v0, 3
	mtc0	v0, $16, 0
	/* pagemask, will set in kernel */
	mtc0	zero, $5,0

	//qiao
	ori		v1, zero, 1
	ori		v0, zero, 2
	sll		v1, v1, CONFIG_NR_CPUS
	subu	v1, v1, v0

1:
	ll	v0, 0(t8)
	or	v0, v0, v1
	sc	v0, 0(t8)
	beqz	v0, 1b
	nop

	/* get cpu mask bit */
	mfc0	v0, $15, 1   /* ebase: core_id */
	andi	v0, v0, 0x3ff
	ori		v1, zero, 1
	sllv	v1, v1, v0

	/* wait for set cpu_start. busy loop! */
2:
	lw	v0, 0(t7)
	and	v0, v0, v1
	beqz	v0, 2b
	nop
	/* read out sp,gp */
	lw	sp, 0(sp)
	lw	gp, 0(gp)

	/* set cpu_running bitmap */
3:
	ll		v0, 0(t9)
	or		v0, v0, v1
	sc		v0, 0(t9)
	beqz	v0, 3b
	nop

	.set	reorder

	j	smp_bootstrap
	nop
	END(__jzsoc_secondary_start)
