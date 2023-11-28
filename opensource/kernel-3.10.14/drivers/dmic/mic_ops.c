#include "mic.h"

static int mic_alloc_sub_mic_buf(struct mic_dev *mic_dev, int mic_type) {
    struct mic *mic = &mic_dev->dmic;
    int data_size = sizeof(short);
    int channels =  4 ;
    int total_size = MIC_BUFFER_TOTAL_LEN * data_size * channels /
            (sizeof(short) * 4 + sizeof(int) * 1);
    int i;

    mic->buf_len = mic_dev->raws_pre_sec * data_size * channels;

    mic->buf_cnt = total_size / mic->buf_len;
    mic->buf_cnt = mic->buf_cnt < 2 ? 2 : mic->buf_cnt;

    if (mic->buf != NULL) {
        kfree(mic->buf[0]);
        kfree(mic->buf);
        mic->buf = NULL;
    }

    mic->buf = (char **)kmalloc(mic->buf_cnt * sizeof(char *), GFP_KERNEL);
    if (!mic->buf) {
        dev_err(mic_dev->dev, "%s: %d: failed to malloc buf!!\n",
                __func__, __LINE__);
        return -ENOMEM;
    }

    mic->buf[0] = kmalloc(mic->buf_len * mic->buf_cnt, GFP_KERNEL);
    if (!mic->buf[0]) {
        dev_err(mic_dev->dev, "%s: %d: failed to malloc buf!!\n",
                __func__, __LINE__);
        return -ENOMEM;
    }

    for (i = 1; i < mic->buf_cnt; i++) {
        mic->buf[i] = mic->buf[i-1] + mic->buf_len;
    }

    memset(mic->buf[0], 0, mic->buf_len * mic->buf_cnt);

    return 0;
}

static void mic_enable_record_force(struct mic_dev *mic_dev) {
    dmic_enable(mic_dev);

    mic_dma_submit_cyclic(mic_dev, &mic_dev->dmic);

    mic_hrtimer_start(mic_dev);
}

static void mic_disable_record_force(struct mic_dev *mic_dev) {
    mic_hrtimer_stop(mic_dev);

    mic_dma_terminate(&mic_dev->dmic);

    dmic_disable(mic_dev);
}

int mic_set_periods_ms_force(struct mic_dev *mic_dev, int periods_ms) {
    int ret;

   /* printk("entry: %s, %d\n", __func__, periods_ms); */

    mutex_lock(&mic_dev->buf_lock);

    if (mic_dev->is_enabled) {
        mic_dev->is_stoped = 1;
        mic_disable_record_force(mic_dev);
    }

    if (mic_dev->data_buf != NULL)
        kfree(mic_dev->data_buf);

    mic_dev->periods_ms = periods_ms;
    mic_dev->raws_pre_sec = 16000 * mic_dev->periods_ms / MSEC_PER_SEC;

    mic_dev->data_buf = kmalloc(mic_dev->raws_pre_sec * sizeof(struct raw_data), GFP_KERNEL);
    if (!mic_dev->data_buf) {
        dev_err(mic_dev->dev, "%s: %d: failed to malloc buf!!\n",
                __func__, __LINE__);
        return -ENOMEM;
    }

    ret = mic_alloc_sub_mic_buf(mic_dev, DMIC);
    if (ret < 0) {
        goto err_alloc_dmic;
    }


    if (mic_dev->is_enabled) {
        mic_dev->is_stoped = 0;
        mic_enable_record_force(mic_dev);
    }

    mutex_unlock(&mic_dev->buf_lock);

    return 0;

err_alloc_dmic:
    kfree(mic_dev->data_buf);
    mic_dev->data_buf = NULL;

    mutex_unlock(&mic_dev->buf_lock);

    return ret;
}

int mic_set_periods_ms(struct mic_file_data *fdata, struct mic_dev *mic_dev, int periods_ms) {
    int min_period = periods_ms;
    struct mic_file_data *tmp;

    /* printk("entry: %s\n", __func__); */

    if (fdata) {
        fdata->periods_ms = periods_ms;
        if (!fdata->is_enabled) {
            return 0;
        }
    }

    spin_lock(&mic_dev->list_lock);

    list_for_each_entry(tmp, &mic_dev->filp_data_list, entry) {
        if (tmp->is_enabled)
            min_period = min(min_period, tmp->periods_ms);
    }

    spin_unlock(&mic_dev->list_lock);

    if (mic_dev->periods_ms == min_period || min_period == INT_MAX)
        return 0;

    return mic_set_periods_ms_force(mic_dev, min_period);
}

int mic_enable_record(struct mic_file_data *fdata, struct mic_dev *mic_dev) {
    struct mic *dmic = &mic_dev->dmic;

    fdata->cnt = mic_dev->dmic.cnt;
    fdata->is_enabled = 1;

    mic_set_periods_ms(NULL, mic_dev, INT_MAX);

    if (!mic_dev->is_enabled) {
        dmic->cnt = 0;
        fdata->cnt = 0;
        mic_dev->is_enabled = 1;
        dmic->offset = 0;
        fdata->dmic_offset = 0;

        mic_enable_record_force(mic_dev);
    }

    return 0;
}

int mic_disable_record(struct mic_file_data *fdata, struct mic_dev *mic_dev) {
    struct mic_file_data *tmp;
    int need_enabled = 0;
   /* printk("entry: %s\n", __func__); */

    spin_lock(&mic_dev->list_lock);

    fdata->is_enabled = 0;
    wake_up_all(&mic_dev->wait_queue);

    list_for_each_entry(tmp, &mic_dev->filp_data_list, entry) {
        if (tmp->is_enabled) {
            need_enabled = 1;
            break;
        }
    }

    spin_unlock(&mic_dev->list_lock);

    mic_set_periods_ms(NULL, mic_dev, INT_MAX);

    if (mic_dev->is_enabled && !need_enabled) {
        mic_dev->is_enabled = 0;
        mic_disable_record_force(mic_dev);
    }

    return 0;
}
