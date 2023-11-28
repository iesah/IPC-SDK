#include <linux/poll.h>
#include "mic.h"

struct mic_dev *m_mic_dev = NULL;

static inline void wait_for_dma_data(struct mic_dev *mic_dev, struct mic_file_data *fdata) {
    struct mic *dmic = &mic_dev->dmic;

    if (dmic->cnt - fdata->cnt >= 2) {
        fdata->cnt = dmic->cnt;
    }

    wait_event(mic_dev->wait_queue,
            !((fdata->cnt >= dmic->cnt || mic_dev->is_stoped)
            && fdata->is_enabled));
}

static int mic_read(struct file *filp, char *buf, size_t size, loff_t *f_pos)
{
    unsigned long long fdata_cnt;
    int id;
    short *dmic_buf;
    int raws_pre_sec;
    struct raw_data *raw_data;
    struct mic_dev *mic_dev = m_mic_dev;
    struct mic_file_data *fdata = (struct mic_file_data *)filp->private_data;
    struct mic *dmic = &mic_dev->dmic;

    int i;
    int total_size = mic_dev->raws_pre_sec * sizeof(struct raw_data);

    if (size < total_size) {
        dev_err(mic_dev->dev,
                "error buf size: %d, need %d\n", size, total_size);
        return -EINVAL;
    }

    /**
     * wait for dma callback
     */
    wait_for_dma_data(mic_dev, fdata);

    if (!mic_dev->is_enabled || !fdata->is_enabled) {
        dev_err(mic_dev->dev, "dmic is no enable\n");
        return -EPERM;
    }

    mutex_lock(&mic_dev->buf_lock);

    fdata_cnt = fdata->cnt;
    id = do_div(fdata_cnt , dmic->buf_cnt);      //id = fdata_cnt % buf_cnt
    dmic_buf = (short *)dmic->buf[id];
    raw_data = mic_dev->data_buf;

    /**
     * copy dmic
     */
    dma_cache_sync(NULL,
            (void *)dmic_buf, dmic->buf_len, DMA_DEV_TO_MEM);

    raws_pre_sec = mic_dev->raws_pre_sec;
    for (i = 0; i < raws_pre_sec;i++) {
        raw_data[i].dmic_channel[0] = dmic_buf[i * 4 + 0];
        raw_data[i].dmic_channel[1] = dmic_buf[i * 4 + 1];
        raw_data[i].dmic_channel[2] = dmic_buf[i * 4 + 2];
        raw_data[i].dmic_channel[3] = dmic_buf[i * 4 + 3];
#if 0
 if((i%15) == 0)
	printk("ch0=%d,ch1=%d,ch2=%d,ch3=%d\r\n",raw_data[i].dmic_channel[0],raw_data[i].dmic_channel[1],raw_data[i].dmic_channel[2],raw_data[i].dmic_channel[3]);
#endif
    }

    /**
     * copy to user
     */
    copy_to_user(buf, raw_data, total_size);
    fdata->cnt++;

    mutex_unlock(&mic_dev->buf_lock);

    return total_size;
}

static int mic_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    return count;
}

static long mic_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
    struct mic_dev *mic_dev = m_mic_dev;
    struct mic_file_data *fdata =
            (struct mic_file_data *)filp->private_data;
    switch (cmd) {
    case MIC_SET_PERIODS_MS:
        return mic_set_periods_ms(fdata, mic_dev, args);

    case MIC_GET_PERIODS_MS:
        copy_to_user((int *)args, &mic_dev->periods_ms, sizeof(int));
        return 0;

    case MIC_ENABLE_RECORD:
        return mic_enable_record(fdata, mic_dev);

    case MIC_DISABLE_RECORD:
        mic_disable_record(fdata, mic_dev);
        return 0;

    case MIC_SET_DMIC_GAIN:
        dmic_set_gain(mic_dev, args);
        return 0;

    case MIC_GET_DMIC_GAIN:
        copy_to_user((int *)args, &mic_dev->dmic.gain, sizeof(int));
        return 0;

    case MIC_GET_DMIC_GAIN_RANGE:
        dmic_get_gain_range((struct gain_range *)args);
        return 0;


    default:
        panic("error mic ioctl\n");
        break;
    }

    return 0;
}

unsigned int mic_poll(struct file *filp, struct poll_table_struct * table) {
    struct mic_dev *mic_dev = m_mic_dev;
    struct mic_file_data *fdata = (struct mic_file_data *)filp->private_data;
    unsigned int mask = 0;

    wait_for_dma_data(mic_dev, fdata);

    mask |= POLLIN | POLLRDNORM;

    return mask;
}

static int mic_open(struct inode *inode, struct file *filp)
{
    struct mic_dev *mic_dev = m_mic_dev;
    struct mic_file_data *fdata = (struct mic_file_data *)
            kmalloc(sizeof(struct mic_file_data), GFP_KERNEL);

    filp->private_data = fdata;
    memset(fdata, 0, sizeof(struct mic_file_data));

    fdata->is_enabled = 0;
    fdata->cnt = 0;
    fdata->periods_ms = INT_MAX;
    INIT_LIST_HEAD(&fdata->entry);

    spin_lock(&mic_dev->list_lock);
    list_add_tail(&fdata->entry, &mic_dev->filp_data_list);
    spin_unlock(&mic_dev->list_lock);

    mic_enable_record(fdata, mic_dev);

    return 0;
}

static int mic_close(struct inode *inode, struct file *filp)
{
    struct mic_dev *mic_dev = m_mic_dev;
    struct mic_file_data *fdata =
            (struct mic_file_data *)filp->private_data;

    mic_disable_record(fdata, mic_dev);

    spin_lock(&mic_dev->list_lock);
    list_del(&fdata->entry);
    spin_unlock(&mic_dev->list_lock);

    kfree(fdata);

    return 0;
}

static struct file_operations mic_ops = {
    .owner = THIS_MODULE,
    .write = mic_write,
    .read = mic_read,
    .open = mic_open,
    .poll = mic_poll,
    .release = mic_close,
    .unlocked_ioctl = mic_ioctl,
};

int mic_sys_init(struct mic_dev *mic_dev) {
    dev_t dev = 0;
    int ret, dev_no;

    m_mic_dev = mic_dev;
    mic_dev->class = class_create(THIS_MODULE, "dmic");
    mic_dev->minor = 0;
    mic_dev->nr_devs = 1;
    ret = alloc_chrdev_region(&dev, mic_dev->minor, mic_dev->nr_devs, "dmic");
    if(ret) {
        printk("alloc chrdev failed\n");
        return -1;
    }
    mic_dev->major = MAJOR(dev);

    spin_lock_init(&mic_dev->list_lock);
    INIT_LIST_HEAD(&mic_dev->filp_data_list);

    dev_no = MKDEV(mic_dev->major, mic_dev->minor);
    cdev_init(&mic_dev->cdev, &mic_ops);
    mic_dev->cdev.owner = THIS_MODULE;
    cdev_add(&mic_dev->cdev, dev_no, 1);

    mic_dev->is_enabled = 0;
    mic_dev->is_stoped = 0;
    device_create(mic_dev->class, NULL, dev_no, NULL, "dmic");

    return 0;
}
