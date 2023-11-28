/**
 * xb_snd_mixer.h
 *
 * jbbi <jbbi@ingenic.cn>
 *
 * 24 APR 2012
 *
 */

#ifndef __XB_SND_MIXER_H__
#define __XB_SND_MIXER_H__

#include <linux/poll.h>
#include <mach/jzsnd.h>
#include "../devices/xb47xx_i2s_v12.h"

/**
 * functions interface
 **/
loff_t xb_snd_mixer_llseek(struct file *file,
						   loff_t offset,
						   int origin,
						   struct snd_dev_data *ddata);

ssize_t xb_snd_mixer_read(struct file *file,
						  char __user *buffer,
						  size_t count,
						  loff_t *ppos,
						  struct snd_dev_data *ddata);

ssize_t xb_snd_mixer_write(struct file *file,
						   const char __user *buffer,
						   size_t count,
						   loff_t *ppos,
						   struct snd_dev_data *ddata);

unsigned int xb_snd_mixer_poll(struct file *file,
							   poll_table *wait,
							   struct snd_dev_data *ddata);

long xb_snd_mixer_ioctl(struct file *file,
						unsigned int cmd,
						unsigned long arg,
						struct snd_dev_data *ddata);

int xb_snd_mixer_mmap(struct file *file,
					  struct vm_area_struct *vma,
					  struct snd_dev_data *ddata);

int xb_snd_mixer_open(struct inode *inode,
					  struct file *file,
					  struct snd_dev_data *ddata);

int xb_snd_mixer_release(struct inode *inode,
						 struct file *file,
						 struct snd_dev_data *ddata);

int xb_snd_mixer_probe(struct snd_dev_data *ddata);

#endif //__XB_SND_MIXER_H__
