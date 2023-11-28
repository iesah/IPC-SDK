#include "avpu_dmabuf.h"

#include <linux/uaccess.h>
#include <linux/dma-buf.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Kevin Grandemange");
MODULE_AUTHOR("Sebastien Alaiwan");
MODULE_AUTHOR("Antoine Gruzelle");
MODULE_DESCRIPTION("JZ Common");

struct avpu_dmabuf_priv {
	struct avpu_dma_buffer *buffer;

	/* DMABUF related */
	struct device *dev;
	struct sg_table *sgt_base;
	enum dma_data_direction dma_dir;

};

struct avpu_dmabuf_attachment {
	struct sg_table sgt;
	enum dma_data_direction dma_dir;
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
/* device argument was removed */
static int avpu_dmabuf_attach(struct dma_buf *dbuf, struct dma_buf_attachment *dbuf_attach)
#else
static int avpu_dmabuf_attach(struct dma_buf *dbuf, struct device* dev, struct dma_buf_attachment *dbuf_attach)
#endif
{
	struct avpu_dmabuf_priv *dinfo = dbuf->priv;

	struct avpu_dmabuf_attachment *attach;

	struct scatterlist *rd, *wr;
	struct sg_table *sgt;
	int ret, i;

	attach = kzalloc(sizeof(*attach), GFP_KERNEL);
	if (!attach)
		return -ENOMEM;

	sgt = &attach->sgt;

	ret = sg_alloc_table(sgt, dinfo->sgt_base->orig_nents, GFP_KERNEL);
	if (ret) {
		kfree(attach);
		return -ENOMEM;
	}

	rd = dinfo->sgt_base->sgl;
	wr = sgt->sgl;

	for (i = 0; i < sgt->orig_nents; ++i) {
		sg_set_page(wr, sg_page(rd), rd->length, rd->offset);
		rd = sg_next(rd);
		wr = sg_next(wr);
	}

	attach->dma_dir = DMA_NONE;

	dbuf_attach->priv = attach;

	return 0;
}

static void avpu_dmabuf_detach(struct dma_buf *dbuf,
			      struct dma_buf_attachment *db_attach)
{
	struct avpu_dmabuf_attachment *attach = db_attach->priv;
	struct sg_table *sgt;

	if (!attach)
		return;

	sgt = &attach->sgt;

	/* release the scatterlist cache */
	if (attach->dma_dir != DMA_NONE)
		dma_unmap_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
			     attach->dma_dir);

	sg_free_table(sgt);
	kfree(attach);
	db_attach->priv = NULL;
}

static struct sg_table *avpu_dmabuf_map(struct dma_buf_attachment *db_attach,
				       enum dma_data_direction dma_dir)
{
	struct avpu_dmabuf_attachment *attach = db_attach->priv;
	struct sg_table *sgt;
	struct mutex *lock = &db_attach->dmabuf->lock;

	mutex_lock(lock);

	sgt = &attach->sgt;

	if (attach->dma_dir == dma_dir) {
		mutex_unlock(lock);
		return sgt;
	}

	if (attach->dma_dir != DMA_NONE) {
		dma_unmap_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
			     attach->dma_dir);
		attach->dma_dir = DMA_NONE;
	}

	sgt->nents = dma_map_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
				dma_dir);

	if (!sgt->nents) {
		pr_err("failed to map scatterlist\n");
		mutex_unlock(lock);
		return ERR_PTR(-EIO);
	}

	attach->dma_dir = dma_dir;

	mutex_unlock(lock);

	return sgt;
}

static void avpu_dmabuf_unmap(struct dma_buf_attachment *at,
			     struct sg_table *sg, enum dma_data_direction dir)
{
}

static int avpu_dmabuf_mmap(struct dma_buf *buf, struct vm_area_struct *vma)
{
	struct avpu_dmabuf_priv *dinfo = buf->priv;
	unsigned long start = vma->vm_start;
	unsigned long vsize = vma->vm_end - start;
	struct avpu_dma_buffer *buffer = dinfo->buffer;
	int ret;

	if (!dinfo) {
		pr_err("No buffer to map\n");
		return -EINVAL;
	}

	vma->vm_pgoff = 0;

	ret = dma_mmap_coherent(dinfo->dev, vma, buffer->cpu_handle,
				buffer->dma_handle, vsize);

	if (ret < 0) {
		pr_err("Remapping memory failed, error: %d\n", ret);
		return ret;
	}

	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;

	return 0;
}

static void avpu_dmabuf_release(struct dma_buf *buf)
{
	struct avpu_dmabuf_priv *dinfo = buf->priv;
	struct avpu_dma_buffer *buffer = dinfo->buffer;

	if (dinfo->sgt_base) {
		sg_free_table(dinfo->sgt_base);
		kfree(dinfo->sgt_base);
	}


	dma_free_coherent(dinfo->dev, buffer->size, buffer->cpu_handle,
			  buffer->dma_handle);

	put_device(dinfo->dev);
	kzfree(buffer);
	kfree(dinfo);
}

static void *avpu_dmabuf_kmap(struct dma_buf *dmabuf, unsigned long page_num)
{
	struct avpu_dmabuf_priv *dinfo = dmabuf->priv;
	void *vaddr = dinfo->buffer->cpu_handle;

	return vaddr + page_num * PAGE_SIZE;
}

static void *avpu_dmabuf_vmap(struct dma_buf *dbuf)
{
	struct avpu_dmabuf_priv *dinfo = dbuf->priv;
	void *vaddr = dinfo->buffer->cpu_handle;

	return vaddr;
}

static const struct dma_buf_ops avpu_dmabuf_ops = {
	.attach		= avpu_dmabuf_attach,
	.detach		= avpu_dmabuf_detach,
	.map_dma_buf	= avpu_dmabuf_map,
	.unmap_dma_buf	= avpu_dmabuf_unmap,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
/* the map_atomic interface was removed after 4.19 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
	.map_atomic	= avpu_dmabuf_kmap,
#endif
	.map		= avpu_dmabuf_kmap,
#else
	.kmap_atomic	= avpu_dmabuf_kmap,
	.kmap		= avpu_dmabuf_kmap,
#endif
	.vmap		= avpu_dmabuf_vmap,
	.mmap		= avpu_dmabuf_mmap,
	.release	= avpu_dmabuf_release,
};

#if 0
static void define_export_info(struct dma_buf_export_info *exp_info,
			       int size,
			       void *priv)
{
	exp_info->owner = THIS_MODULE;
	exp_info->exp_name = KBUILD_MODNAME;
	exp_info->ops = &avpu_dmabuf_ops;
	exp_info->flags = O_RDWR;
	exp_info->resv = NULL;
	exp_info->size = size;
	exp_info->priv = priv;
}
#endif

static struct sg_table *avpu_get_base_sgt(struct avpu_dmabuf_priv *dinfo)
{
	int ret;
	struct sg_table *sgt;
	struct avpu_dma_buffer *buf = dinfo->buffer;
	struct device *dev = dinfo->dev;

	sgt = kzalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt)
		return NULL;

	ret = dma_get_sgtable(dev, sgt, buf->cpu_handle, buf->dma_handle,
			      buf->size);
	if (ret < 0) {
		kfree(sgt);
		return NULL;
	}

	return sgt;

}

static struct dma_buf *avpu_get_dmabuf(void *dma_info_priv)
{
	struct dma_buf *dbuf;
//	struct dma_buf_export_info exp_info;

	struct avpu_dmabuf_priv *dinfo = dma_info_priv;

	struct avpu_dma_buffer *buf = dinfo->buffer;

//	define_export_info(&exp_info,
//			   buf->size,
//			   (void *)dinfo);

	if (!dinfo->sgt_base)
		dinfo->sgt_base = avpu_get_base_sgt(dinfo);

	if (WARN_ON(!dinfo->sgt_base))
		return NULL;

	dbuf = dma_buf_export((void *)dinfo, &avpu_dmabuf_ops, buf->size, O_RDWR);
	if (IS_ERR(buf)) {
		pr_err("couldn't export dma buf\n");
		return NULL;
	}

	return dbuf;
}

static void *avpu_dmabuf_wrap(struct device *dev, unsigned long size,
			     struct avpu_dma_buffer *buffer)
{
	struct avpu_dmabuf_priv *dinfo;
	struct dma_buf *dbuf;

	dinfo = kzalloc(sizeof(*dinfo), GFP_KERNEL);
	if (!dinfo)
		return ERR_PTR(-ENOMEM);

	dinfo->dev = get_device(dev);
	dinfo->buffer = buffer;
	dinfo->dma_dir = DMA_BIDIRECTIONAL;
	dinfo->sgt_base = avpu_get_base_sgt(dinfo);

	dbuf = avpu_get_dmabuf(dinfo);
	if (IS_ERR_OR_NULL(dbuf))
		return ERR_PTR(-EINVAL);

	return dbuf;
}

int avpu_create_dmabuf_fd(struct device *dev, unsigned long size,
			 struct avpu_dma_buffer *buffer)
{
	struct dma_buf *dbuf = avpu_dmabuf_wrap(dev, size, buffer);

	if (IS_ERR(dbuf))
		return PTR_ERR(dbuf);
	return dma_buf_fd(dbuf, O_RDWR);
}

int avpu_allocate_dmabuf(struct device *dev, int size, u32 *fd)
{
	struct avpu_dma_buffer *buffer;

	buffer = avpu_alloc_dma(dev, size);
	if (!buffer) {
		dev_err(dev, "Can't alloc DMA buffer\n");
		return -ENOMEM;
	}

	*fd = avpu_create_dmabuf_fd(dev, size, buffer);
	return 0;
}

int avpu_dmabuf_get_address(struct device *dev, u32 fd, u32 *bus_address)
{
	struct dma_buf *dbuf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	int err = 0;

	dbuf = dma_buf_get(fd);
	if (IS_ERR(dbuf))
		return -EINVAL;
	attach = dma_buf_attach(dbuf, dev);
	if (IS_ERR(attach)) {
		err = -EINVAL;
		goto fail_attach;
	}
	sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(sgt)) {
		err = -EINVAL;
		goto fail_map;
	}

	*bus_address = sg_dma_address(sgt->sgl);

	dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
fail_map:
	dma_buf_detach(dbuf, attach);
fail_attach:
	dma_buf_put(dbuf);
	return err;
}

