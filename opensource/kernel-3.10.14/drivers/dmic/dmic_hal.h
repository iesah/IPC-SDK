#ifndef DMIC_HAL_H
#define DMIC_HAL_H

#include "mic.h"


static void inline dmic_write_reg(struct mic_dev *mic_dev, unsigned int reg,
        unsigned int val)
{
    struct mic *dmic = &mic_dev->dmic;

    writel(val, dmic->iomem + reg);
}

static unsigned int inline dmic_read_reg(struct mic_dev *mic_dev, unsigned int reg)
{
    struct mic *dmic = &mic_dev->dmic;

    return readl(dmic->iomem + reg);
}
#define dmic_set_reg(dev, addr, val, mask, offset)\
    do {    \
        int tmp_val = val;                          \
        int read_val = dmic_read_reg(dev, addr);         \
        read_val &= (~mask);                    \
        tmp_val = ((tmp_val << offset) & mask); \
        tmp_val |= read_val;                    \
        dmic_write_reg(dev, addr, tmp_val);           \
    }while(0)

#define dmic_get_reg(dev, addr, mask, offset)  \
    ((dmic_read_reg(dev, addr) & mask) >> offset)


#define DMICCR0     0x0
#define DMICGCR     0x4
#define DMICIMR     0x8
#define DMICINTCR   0xc
#define DMICTRICR   0x10
#define DMICTHRH    0x14
#define DMICTHRL    0x18
#define DMICTRIMMAX 0x1c
#define DMICTRINMAX 0x20
#define DMICDR      0x30
#define DMICFTHR    0x34
#define DMICFSR     0x38
#define DMICCGDIS   0x50


/*    DMICCR0  */
#define DMIC_RESET      31
#define DMIC_RESET_MASK     (0x1 << DMIC_RESET)
#define DMIC_RESET_TRI  30
#define DMIC_RESET_TRI_MASK (0x1 << DMIC_RESET_TRI)
#define DMIC_CHNUM      16
#define DMIC_CHNUM_MASK         (0x7 << DMIC_CHNUM)
#define DMIC_UNPACK_MSB 13
#define DMIC_UNPACK_MSB_MASK    (0x1 << DMIC_UNPACK_MSB)
#define DMIC_UNPACK_DIS 12
#define DMIC_UNPACK_DIS_MASK    (0x1 << DMIC_UNPACK_DIS)
#define DMIC_SW_LR      11
#define DMIC_SW_LR_MASK     (0x1 << DMIC_SW_LR)
#define DMIC_SPLIT_DI   10
#define DMIC_SPLIT_DI_MASK  (0x1 << DMIC_SPLIT_DI)
#define DMIC_PACK_EN    8
#define DMIC_PACK_EN_MASK   (0x1 << DMIC_PACK_EN)
#define DMIC_SR         6
#define DMIC_SR_MASK        (0x3 << DMIC_SR)
#define DMIC_LP_MODE    3
#define DMIC_LP_MODE_MASK   (0x1 << DMIC_LP_MODE)
#define DMIC_HPF1_MODE  2
#define DMIC_HPF1_MODE_MASK (0x1 << DMIC_HPF1_MODE)
#define DMIC_TRI_EN     1
#define DMIC_TRI_EN_MASK    (0x1 << DMIC_TRI_EN)
#define DMIC_EN         0
#define DMIC_EN_MASK        (0x1 << DMIC_EN)

#define __dmic_reset(dev)\
        dmic_set_reg(dev,DMICCR0,1,DMIC_RESET_MASK,DMIC_RESET)

#define __dmic_get_reset(dev)\
        dmic_get_reg(dev,DMICCR0,DMIC_RESET_MASK,DMIC_RESET)

#define __dmic_reset_tri(dev)\
        dmic_set_reg(dev,DMICCR0,1,DMIC_RESET_TRI_MASK,DMIC_RESET_TRI)

#define __dmic_set_chnum(dev,n)\
        dmic_set_reg(dev,DMICCR0,n,DMIC_CHNUM_MASK,DMIC_CHNUM)

#define __dmic_get_chnum(dev,n)\
        dmic_set_reg(dev,DMICCR0,DMIC_CHNUM_MASK,DMIC_CHNUM)

#define __dmic_unpack_msb(dev)\
        dmic_set_reg(dev,DMICCR0,1,DMIC_UNPACK_MSB_MASK,DMIC_UNPACK_MSB)

#define __dmic_unpack_dis(dev)\
        dmic_set_reg(dev,DMICCR0,1,DMIC_UNPACK_DIS_MASK,DMIC_UNPACK_DIS)

#define __dmic_enable_sw_lr(dev)\
        dmic_set_reg(dev,DMICCR0,1,DMIC_SW_LR_MASK,DMIC_SW_LR)

#define __dmic_disable_sw_lr(dev)\
        dmic_set_reg(dev,DMICCR0,0,DMIC_SW_LR_MASK,DMIC_SW_LR)

#define __dmic_split(dev)\
        dmic_set_reg(dev,DMICCR0,1,DMIC_SPLIT_DI_MASK,DMIC_SPLIT_DI)

#define __dmic_enable_pack(dev)\
        dmic_set_reg(dev,DMICCR0,1,DMIC_PACK_EN_MASK,DMIC_PACK_EN)

#define __dmic_disable_pack(dev)\
        dmic_set_reg(dev,DMICCR0,0,DMIC_PACK_EN_MASK,DMIC_PACK_EN)

#define __dmic_set_sr(dev,n)\
        dmic_set_reg(dev,DMICCR0,n,DMIC_SR_MASK,DMIC_SR)

#define __dmic_set_sr_8k(dev)\
        __dmic_set_sr(dev,0)

#define __dmic_set_sr_16k(dev)\
        __dmic_set_sr(dev,1)

#define __dmic_set_sr_48k(dev)\
        __dmic_set_sr(dev,2)

#define __dmic_enable_lp(dev)\
        dmic_set_reg(dev,DMICCR0,1,DMIC_LP_MODE_MASK,DMIC_LP_MODE)

#define __dmic_disable_lp(dev)\
        dmic_set_reg(dev,DMICCR0,0,DMIC_LP_MODE_MASK,DMIC_LP_MODE)

#define __dmic_enable_hpf1(dev)\
        dmic_set_reg(dev,DMICCR0,1,DMIC_HPF1_MODE_MASK,DMIC_HPF1_MODE)

#define __dmic_disable_hpf1(dev)\
        dmic_set_reg(dev,DMICCR0,0,DMIC_HPF1_MODE_MASK,DMIC_HPF1_MODE)

#define __dmic_is_enable_tri(dev)\
        dmic_get_reg(dev,DMICCR0,DMIC_TRI_EN_MASK,DMIC_TRI_EN)

#define __dmic_enable_tri(dev)\
        dmic_set_reg(dev,DMICCR0,1,DMIC_TRI_EN_MASK,DMIC_TRI_EN)

#define __dmic_disable_tri(dev)\
        dmic_set_reg(dev,DMICCR0,0,DMIC_TRI_EN_MASK,DMIC_TRI_EN)

#define __dmic_is_enable(dev)\
        dmic_get_reg(dev,DMICCR0,DMIC_EN_MASK,DMIC_EN)

#define __dmic_enable(dev)\
        dmic_set_reg(dev,DMICCR0,1,DMIC_EN_MASK,DMIC_EN)

#define __dmic_disable(dev)\
        dmic_set_reg(dev,DMICCR0,0,DMIC_EN_MASK,DMIC_EN)

/*DMICGCR*/
#define DMIC_GCR    0
#define DMIC_GCR_MASK   (0Xf << DMIC_GCR)

#define __dmic_set_gcr(dev,n)\
        dmic_set_reg(dev, DMICGCR, n, DMIC_GCR_MASK,DMIC_GCR)

/* DMICIMR */
#define DMIC_FIFO_TRIG_MASK 5
#define DMIC_FIFO_TRIG_MSK      (1 << DMIC_FIFO_TRIG_MASK)
#define DMIC_WAKE_MASK  4
#define DMIC_WAKE_MSK       (1 << DMIC_WAKE_MASK)
#define DMIC_EMPTY_MASK 3
#define DMIC_EMPTY_MSK      (1 << DMIC_EMPTY_MASK)
#define DMIC_FULL_MASK  2
#define DMIC_FULL_MSK       (1 << DMIC_FULL_MASK)
#define DMIC_PRERD_MASK 1
#define DMIC_PRERD_MSK      (1 << DMIC_PRERD_MASK)
#define DMIC_TRI_MASK   0
#define DMIC_TRI_MSK        (1 << DMIC_TRI_MASK)

#define __dmic_mask_all_int(dev)\
    dmic_set_reg(dev,DMICIMR, 0x3f, 0x3f, 0)


/*DMICINTCR*/
#define DMIC_FIFO_TRIG_FLAG 4
#define DMIC_FIFO_TRIG_FLAG_MASK        (1 << DMIC_WAKE_FLAG)
#define DMIC_WAKE_FLAG  4
#define DMIC_WAKE_FLAG_MASK     (1 << DMIC_WAKE_FLAG)
#define DMIC_EMPTY_FLAG 3
#define DMIC_EMPTY_FLAG_MASK    (1 << DMIC_EMPTY_FLAG)
#define DMIC_FULL_FLAG  2
#define DMIC_FULL_FLAG_MASK     (1 << DMIC_FULL_FLAG)
#define DMIC_PRERD_FLAG 1
#define DMIC_PRERD_FLAG_MASK    (1 << DMIC_PRERD_FLAG)
#define DMIC_TRI_FLAG   0
#define DMIC_TRI_FLAG_MASK      (1 << DMIC_TRI_FLAG)

/*DMICTRICR*/
#define DMIC_TRI_MODE   16
#define DMIC_TRI_MODE_MASK  (0xf << DMIC_TRI_MODE)
#define DMIC_TRI_DEBUG  4
#define DMIC_TRI_DEBUG_MASK (0x1 << DMIC_TRI_DEBUG)
#define DMIC_HPF2_EN    3
#define DMIC_HPF2_EN_MASK (0x1 << DMIC_HPF2_EN)
#define DMIC_PREFETCH   1
#define DMIC_PREFETCH_MASK  (0x3 << DMIC_PREFETCH)
#define DMIC_TRI_CLR    0
#define DMIC_TRI_CLR_MASK   (0x1 << DMIC_TRI_CLR)

#define __dmic_enable_hpf2(dev) \
    dmic_set_reg(dev, DMICTRICR, 1, DMIC_HPF2_EN_MASK, DMIC_HPF2_EN)

#define __dmic_disable_hpf2(dev)    \
    dmic_set_reg(dev, DMICTRICR, 0, DMIC_HPF2_EN_MASK, DMIC_HPF2_EN)

#define __dmic_prefetch_8k(dev)    \
    dmic_set_reg(dev, DMICTRICR, 0x01, DMIC_PREFETCH_MASK, DMIC_PREFETCH)

#define __dmic_prefetch_16k(dev)    \
    dmic_set_reg(dev, DMICTRICR, 0x02, DMIC_PREFETCH_MASK, DMIC_PREFETCH)

/*DMICTHRH*/
#define DMIC_THR_H 0
#define DMIC_THR_H_MASK (0xfffff << DMIC_THR_H)

#define __dmic_set_thr_high(dev,n)  \
    dmic_set_reg(dev, DMICTHRH, n, DMIC_THR_H_MASK, DMIC_THR_H)

/*DMICTHRL*/
#define DMIC_THR_L 0
#define DMIC_THR_L_MASK (0xfffff << DMIC_THR_L)

#define __dmic_set_thr_low(dev,n)   \
    dmic_set_reg(dev, DMICTHRL, n, DMIC_THR_L_MASK, DMIC_THR_L)


/* DMICTRIMMAX */
#define DMIC_M_MAX  0
#define DMIC_M_MAX_MASK (0xffffff << DMIC_M_MAX)

/* DMICTRINMAX */
#define DMIC_N_MAX  0
#define DMIC_N_MAX_MASK (0xffff << DMIC_N_MAX)

/* DMICFTHR */
#define DMIC_RDMS   31
#define DMIC_RDMS_MASK  (0x1 << DMIC_RDMS)
#define DMIC_FIFO_THR   0
#define DMIC_FIFO_THR_MASK  (0x3f << DMIC_FIFO_THR)

#define __dmic_is_enable_rdms(dev)\
    dmic_get_reg(dev, DMICFTHR,DMIC_RDMS_MASK,DMIC_RDMS)
#define __dmic_enable_rdms(dev)\
    dmic_set_reg(dev, DMICFTHR,1,DMIC_RDMS_MASK,DMIC_RDMS)
#define __dmic_disable_rdms(dev)\
    dmic_set_reg(dev, DMICFTHR,1,DMIC_RDMS_MASK,DMIC_RDMS)
#define __dmic_set_request(dev,n)   \
    dmic_set_reg(dev, DMICFTHR, n, DMIC_FIFO_THR_MASK, DMIC_FIFO_THR)

/*DMICFSR*/
#define DMIC_FULLS  19
#define DMIC_FULLS_MASK (0x1 << DMIC_FULLS)
#define DMIC_TRIGS  18
#define DMIC_TRIGS_MASK (0x1 << DMIC_TRIGS)
#define DMIC_PRERDS 17
#define DMIC_PRERDS_MASK    (0x1 << DMIC_PRERDS)
#define DMIC_EMPTYS 16
#define DMIC_EMPTYS_MASK    (0x1 << DMIC_EMPTYS)
#define DMIC_FIFO_LVL 0
#define DMIC_FIFO_LVL_MASK  (0x3f << DMIC_FIFO_LVL)

#define __dmic_read_fifo_lvl(dev)\
    dmic_get_reg(dev, DMICFSR,DMIC_FIFO_LVL_MASK,DMIC_FIFO_LVL)


/*DMICDR*/
#define DMIC_DR 0
#define DMIC_DR_MASK  (0xffffffff << DMIC_DR)
#define __dmic_read_dr(dev)\
    dmic_get_reg(dev, DMICDR,DMIC_DR_MASK,DMIC_DR)


enum dmic_rate {
    DMIC_RATE_8000 = 8000,
    DMIC_RATE_16000 = 16000,
    DMIC_RATE_48000 = 48000,
};

#define DMIC_FIFO_DEPTH 64

void dmic_init(struct mic_dev *mic_dev);
void dmic_enable(struct mic_dev *mic_dev);
void dmic_disable(struct mic_dev *mic_dev);
int dmic_is_enable(struct mic_dev *mic_dev);

#endif
