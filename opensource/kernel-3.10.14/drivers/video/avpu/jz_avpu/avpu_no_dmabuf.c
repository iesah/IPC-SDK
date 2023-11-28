#include "avpu_dmabuf.h"

int avpu_create_dmabuf_fd(struct device *dev, unsigned long size,
			 struct avpu_dma_buffer *buffer)
{
	pr_err("dmabuf interface not supported");
	return -EINVAL;
}

int avpu_allocate_dmabuf(struct device *dev, int size, u32 *fd)
{
	pr_err("dmabuf interface not supported");
	return -EINVAL;
}

int avpu_dmabuf_get_address(struct device *dev, u32 fd, u32 *bus_address)
{
	pr_err("dmabuf interface not supported");
	return -EINVAL;
}

