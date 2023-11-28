/*
 *  Copyright (C) 2016 Ingenic Semiconductor Co.,Ltd
 *
 *  X1000 series bootloader for u-boot/rtos/linux
 *
 *  Zhang YanMing <yanming.zhang@ingenic.com, jamincheung@126.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef SPIFLASH_H
#define SPIFLASH_H

#define CMD_WREN                    0x06    /* Write Enable */
#define CMD_WRDI                    0x04    /* Write Disable */
#define CMD_RDSR                    0x05    /* Read Status Register */
#define CMD_RDSR_1                  0x35    /* Read Status1 Register */
#define CMD_RDSR_2                  0x15    /* Read Status2 Register */
#define CMD_WRSR                    0x01    /* Write Status Register */
#define CMD_WRSR_1                  0x31    /* Write Status1 Register */
#define CMD_WRSR_2                  0x11    /* Write Status2 Register */
#define CMD_READ                    0x03    /* Read Data */
#define CMD_FAST_READ               0x0B    /* Read Data at high speed */
#define CMD_DUAL_READ               0x3b    /* Read Data at QUAD fast speed*/
#define CMD_QUAD_READ               0x6b    /* Read Data at QUAD fast speed*/
#define CMD_QUAD_IO_FAST_READ       0xeb    /* Read Data at QUAD IO fast speed*/
#define CMD_PP                      0x02    /* Page Program(write data) */
#define CMD_QPP                     0x32    /* QUAD Page Program(write data) */
#define CMD_SE                      0xD8    /* Sector Erase */
#define CMD_BE                      0xC7    /* Bulk or Chip Erase */
#define CMD_DP                      0xB9    /* Deep Power-Down */
#define CMD_RES                     0xAB    /* Release from Power-Down and Read Electronic Signature */
#define CMD_RDID                    0x9F    /* Read Identification */
#define CMD_SR_WIP                  (1 << 0)
#define CMD_SR_QEP                  (1 << 1)
#define CMD_ERASE_4K                0x20
#define CMD_ERASE_32K               0x52
#define CMD_ERASE_64K               0xd8
#define CMD_ERASE_CE                0x60
#define CMD_EN4B                    0xB7
#define CMD_EX4B                    0xE9
#define CMD_READ4                   0x13    /* Read Data */
#define CMD_FAST_READ4              0x0C    /* Read Data at high speed */
#define CMD_PP_4B                   0x12    /* Page Program(write data) */
#define CMD_ERASE_4K_4B             0x21
#define CMD_ERASE_32K_4B            0x5C
#define CMD_ERASE_64K_4B            0xDC

#define CMD_PARD                    0x13    /* page read */
#define CMD_PE                      0x10    /* program execute*/
#define CMD_PRO_LOAD                0x02    /* program load */
#define CMD_PRO_RDM                 0x84    /* program random */
#define CMD_ERASE_128K              0xd8
#define CMD_R_CACHE                 0x03    /* read from cache */
#define CMD_FR_CACHE                0x0b    /* fast read from cache */
#define CMD_FR_CACHE_QUAD           0x6b
#define CMD_GET_FEATURE             0x0f
#define CMD_SET_FEATURE             0x1f

#define FEATURE_REG_PROTECT         0xa0
#define FEATURE_REG_FEATURE1        0xb0
#define FEATURE_REG_STATUS1         0xc0
#define FEATURE_REG_FEATURE2        0xd0
#define FEATURE_REG_STATUS2         0xf0
#define BITS_ECC_EN                 (1 << 4)
#define BITS_QUAD_EN                (1 << 0)
#define BITS_BUF_EN		    (1 << 3)  /*notice: only use by winbond*/

/* some manufacture with unusual method */
#define MANUFACTURE_PID1_IGNORE 0x00

#define WINBOND_VID             0xef
#define WINBOND_PID0            0xaa
#define WINBOND_PID1            0x21

#define GIGADEVICE_VID          0xc8

#define GD5F1GQ4UC_PID          0xb1
#define GD5F2GQ4UC_PID          0xb2
#define GD5F1GQ4RC_PID          0xa1
#define GD5F2GQ4RC_PID          0xa2
//#define GD5FxGQ4xC_PID1         0x48

#define BITS_BUF_EN                 (1 << 3)

/* Buswidth is 16 bit */
#define NAND_BUSWIDTH_16    0x00000002
#define NAND_BUSWIDTH_8      0x00000001

enum {
    VALUE_SET,
    VALUE_CLR,
};

#define OPERAND_CONTROL(action, val, ret) do{                                   \
    if (action == VALUE_SET)                                                    \
        ret |= val;                                                             \
    else                                                                        \
        ret &= ~val;                                                            \
} while(0)

struct spiflash_register {
    uint32_t addr;
    uint32_t val;
    uint8_t action;
};

struct special_spiflash_id {
    uint8_t vid;
    uint8_t pid;
};

struct special_spiflash_desc {
    uint8_t vid;
    struct spiflash_register regs;
};

int spinor_init(void);
int spinor_read(uint32_t src_addr, uint32_t count, uint32_t dst_addr);

int spinand_read(uint32_t src_addr, uint32_t count, uint32_t dst_addr);

///////////////////upper layer logic depends on flash /////////////////
#ifdef CONFIG_BEIJING_OTA
int ota_load(uint32_t *argv, uint32_t dst_addr);
#endif

#ifdef CONFIG_RECOVERY
int is_recovery_update_failed(void);
#endif
#endif /* SPIFLASH_H */
