#include <linux/slab.h>
#include <linux/vmalloc.h>

#include "os/NandAlloc.h"
#if 0
void *Nand_VirtualAlloc(int size)
{
	return kmalloc(size,GFP_KERNEL); //vmalloc(size);
}

void Nand_VirtualFree(void *val)
{
	kfree(val);
}



void *Nand_ContinueAlloc(int size)
{
	return kmalloc(size,GFP_KERNEL);
}

void Nand_ContinueFree(void *val)
{
	kfree(val);
}
#endif
