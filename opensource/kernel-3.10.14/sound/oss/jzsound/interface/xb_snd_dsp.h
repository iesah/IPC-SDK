/**
 * xb_snd_dsp.h
 *
 * jbbi <jbbi@ingenic.cn>
 *
 * 24 APR 2012
 *
 */

#ifndef __XB_SND_DSP_H__
#define __XB_SND_DSP_H__

#include <linux/dmaengine.h>
#include <linux/wait.h>
#include <linux/scatterlist.h>
#include <linux/spinlock.h>
#include <linux/poll.h>
#include <mach/jzdma.h>
#include <mach/jzsnd.h>
#include <linux/dma-mapping.h>
#include <linux/mutex.h>

/*####################################################*\
 * sound pipe and command used for dsp device
\*####################################################*/

#undef SND_DEBUG
//#define SND_DEBUG
#ifdef SND_DEBUG
#define ENTER_FUNC()	\
	printk("++++++enter %s++++++.\n" ,__func__)
#define LEAVE_FUNC()	\
	printk("------leave %s------.\n" ,__func__)
#define debug_print(fmt,args...)	\
		do {	\
			printk("#######(%s:%d):",__func__,__LINE__);	\
			printk(fmt".\n",##args);\
		} while (0)
#else
#define ENTER_FUNC()
#define LEAVE_FUNC()
#define debug_print(fmt,args...)  do {} while(0)
#endif


#define DEFAULT_REPLAY_SAMPLERATE	44100
#define DEFAULT_RECORD_SAMPLERATE	44100
#define DEFAULT_REPLAY_CHANNEL		2
#define DEFAULT_RECORD_CHANNEL		1
//#define DEFAULT_RECORD_CHANNEL		2
/**
 * sound device
 **/
enum snd_device_t {
	SND_DEVICE_DEFAULT = 0,

	SND_DEVICE_CURRENT,
	SND_DEVICE_HANDSET,
	SND_DEVICE_HEADSET,	//fix for mid pretest
	SND_DEVICE_SPEAKER,	//fix for mid pretest
	SND_DEVICE_HEADPHONE,

	SND_DEVICE_BT,
	SND_DEVICE_BT_EC_OFF,
	SND_DEVICE_HEADSET_AND_SPEAKER,
	SND_DEVICE_TTY_FULL,
	SND_DEVICE_CARKIT,

	SND_DEVICE_FM_SPEAKER,
	SND_DEVICE_FM_HEADSET,
	SND_DEVICE_BUILDIN_MIC,
	SND_DEVICE_HEADSET_MIC,
	SND_DEVICE_HDMI = 15,	//fix for mid pretest

	SND_DEVICE_LOOP_TEST = 16,

	SND_DEVICE_CALL_START,							//call route start mark
	SND_DEVICE_CALL_HEADPHONE = SND_DEVICE_CALL_START,
	SND_DEVICE_CALL_HEADSET,
	SND_DEVICE_CALL_HANDSET,
	SND_DEVICE_CALL_SPEAKER,
	SND_DEVICE_HEADSET_RECORD_INCALL,
	SND_DEVICE_BUILDIN_RECORD_INCALL,
	SND_DEVICE_CALL_END = SND_DEVICE_BUILDIN_RECORD_INCALL,	//call route end mark

	SND_DEVICE_LINEIN_RECORD,
	SND_DEVICE_LINEIN1_RECORD,
	SND_DEVICE_LINEIN2_RECORD,
	SND_DEVICE_LINEIN3_RECORD,

	SND_DEVICE_COUNT
};
/**
 * extern ioctl command for dsp device
 **/
struct direct_info {
    int bytes;
    int offset;
};

enum spipe_mode_t {
    SPIPE_MODE_RECORD,
    SPIPE_MODE_REPLAY,
};

struct spipe_info {
    int spipe_id;
    enum spipe_mode_t spipe_mode;
};

#define SNDCTL_EXT_SET_DEBUG				_SIOW ('P', 102, int)
#define SNDCTL_EXT_MOD_TIMER				_SIOW ('P', 101, int)
#define SNDCTL_EXT_SET_BUFFSIZE				_SIOR ('P', 100, int)
#define SNDCTL_EXT_SET_DEVICE               _SIOR ('P', 99, int)
#define SNDCTL_EXT_SET_STANDBY              _SIOR ('P', 98, int)
#define SNDCTL_EXT_START_BYPASS_TRANS       _SIOW ('P', 97, struct spipe_info)
#define SNDCTL_EXT_STOP_BYPASS_TRANS        _SIOW ('P', 96, struct spipe_info)
#define SNDCTL_EXT_DIRECT_GETINODE          _SIOR ('P', 95, struct direct_info)
#define SNDCTL_EXT_DIRECT_PUTINODE          _SIOW ('P', 94, struct direct_info)
#define SNDCTL_EXT_DIRECT_GETONODE          _SIOR ('P', 93, struct direct_info)
#define SNDCTL_EXT_DIRECT_PUTONODE          _SIOW ('P', 92, struct direct_info)
#define SNDCTL_EXT_STOP_DMA		            _SIOW ('P', 91, int)
#define SNDCTL_EXT_SET_REPLAY_VOLUME        _SIOR ('P', 90, int)
#define SNDCTL_EXT_SET_RECORD_VOLUME        _SIOR ('P', 103, int)
/**
 * dsp device control command
 **/
enum snd_dsp_command {
	/**
	 * the command flowed is used to enable/disable
	 * replay/record.
	 **/
	SND_DSP_ENABLE_REPLAY=0,
	SND_DSP_DISABLE_REPLAY,
	SND_DSP_ENABLE_RECORD,
	SND_DSP_DISABLE_RECORD,
	/**
	 * the command flowed is used to enable/disable the
	 * dma transfer on device.
	 **/
	SND_DSP_ENABLE_DMA_RX,
	SND_DSP_DISABLE_DMA_RX,
	SND_DSP_ENABLE_DMA_TX,
	SND_DSP_DISABLE_DMA_TX,
	/**
	 *@SND_DSP_SET_XXXX_RATE is used to set replay/record rate
	 *@SND_DSP_GET_XXXX_RATE is used to get current samplerate
	 **/
	SND_DSP_SET_REPLAY_RATE,
	SND_DSP_SET_RECORD_RATE,
	SND_DSP_GET_REPLAY_RATE, /*10*/
	SND_DSP_GET_RECORD_RATE,
	/**
	 * @SND_DSP_SET_XXXX_CHANNELS is used to set replay/record
	 * channels, when channels changed, filter maybe also need
	 * changed to a fix value.
	 * @SND_DSP_GET_XXXX_CHANNELS is used to get current channels
	 **/
	SND_DSP_SET_REPLAY_CHANNELS,
	SND_DSP_SET_RECORD_CHANNELS,
	SND_DSP_GET_REPLAY_CHANNELS,
	SND_DSP_GET_RECORD_CHANNELS,
	/**
	 * @SND_DSP_GET_XXXX_FMT_CAP is used to get formats that
	 * replay/record supports.
	 * @SND_DSP_GET_XXXX_FMT used to get current replay/record
	 * format.
	 * @SND_DSP_SET_XXXX_FMT is used to set replay/record format,
	 * if the format changed, trigger,dma max_tsz and filter maybe
	 * also need changed to a fix value. and let them effect.
	 **/
	SND_DSP_GET_REPLAY_FMT_CAP,
	SND_DSP_GET_REPLAY_FMT,
	SND_DSP_SET_REPLAY_FMT,
	SND_DSP_GET_RECORD_FMT_CAP,
	SND_DSP_GET_RECORD_FMT,	/*20*/
	SND_DSP_SET_RECORD_FMT,
	/**
	 * @SND_DSP_SET_DEVICE is used to set audio route
	 * @SND_DSP_SET_STANDBY used to set into/release from stanby
	 **/
	SND_DSP_SET_DEVICE,
	SND_DSP_SET_STANDBY,
	/**
	 *	@SND_MIXER_DUMP_REG dump aic register and codec register val
	 *	@SND_MIXER_DUMP_GPIO dump hp mute gpio ...
	 **/
	SND_MIXER_DUMP_REG,
	SND_MIXER_DUMP_GPIO,
	/**
	 *	@SND_DSP_GET_HP_DETECT dump_hp_detect state
	 **/
	SND_DSP_GET_HP_DETECT,
	/**
	 *	@SND_DSP_SET_RECORD_VOL set adc vol
	 *	@SND_DSP_SET_MIC_VOL	set input vol
	 *	@SND_DSP_SET_REPLAY_VOL set dac vol
	 **/
	SND_DSP_SET_RECORD_VOL,
	SND_DSP_SET_MIC_VOL,
	SND_DSP_SET_REPLAY_VOL,
	/**
	 *	@SND_DSP_SET_DMIC_TRIGGER_MODE set dmic trigger
	 **/
	SND_DSP_SET_DMIC_TRIGGER_MODE, /*30*/
	/**
	 * @SND_DSP_CLR_ROUTE  for pretest
	 **/
	SND_DSP_CLR_ROUTE,

	SND_DSP_DEBUG,
	SND_DSP_SET_VOICE_TRIGGER,
	/**
	 *	@SND_DSP_FLUSH_SYNC, wait for ioctl work done.
	 * */
	SND_DSP_FLUSH_SYNC,	/*34*/
	/**
	 * @SND_DSP_RESUME_PROCEDURE, called by driver, resume.
	 */
	SND_DSP_RESUME_PROCEDURE,
};

/**
 *fragsize, must be dived by PAGE_SIZE
 **/
#define FRAGSIZE_S  (PAGE_SIZE >> 1)
#define FRAGSIZE_M  (PAGE_SIZE)
#define FRAGSIZE_L  (PAGE_SIZE << 1)

#define FRAGCNT_S   2
#define FRAGCNT_M   4
#define FRAGCNT_L   8
#define FRAGCNT_B   16



struct dsp_node {
	struct list_head    list;
	unsigned long       pBuf;
	unsigned int        start;
	unsigned int        end;
	dma_addr_t			phyaddr;
	size_t				size;
	uint32_t			node_number;
};

struct dsp_pipe {
	/* dma */
	struct dma_chan     *dma_chan;
	struct dma_slave_config dma_config;     /* define by device */
	enum jzdma_type     dma_type;
	unsigned int        sg_len;         /* size of scatter list */
	struct scatterlist  *sg;            /* I/O scatter list */
	/* buf */
	unsigned long       vaddr;
	dma_addr_t          paddr;
	size_t              fragsize;              /* define by device */
	size_t              fragcnt;               /* define by device */
	size_t				buffersize;
	size_t				channels;
	unsigned			samplerate;
	int					mod_timer;
	struct list_head    free_node_list;
	struct list_head    use_node_list;
	struct list_head	dma_node_list;
	struct dsp_node     *save_node;
	wait_queue_head_t   wq;
	atomic_t			avialable_couter;

	atomic_t			watchdog_avail;
	struct timer_list	transfer_watchdog;
	uint32_t watchdog_mdelay;
	volatile bool		force_hdmi;

	/* state */
	volatile bool       is_trans;
	volatile bool       wait_stop_dma;
	volatile bool	    force_stop_dma;
	volatile bool       need_reconfig_dma;
	volatile bool       is_used;
	volatile bool       is_shared;
	volatile bool       is_mmapd;
	bool				is_non_block;          /* define by device */
	bool				can_mmap;              /* define by device */
	/* callback funs */
	void (*handle)(struct dsp_pipe *endpoint); /* define by device */
	/* @filter()
	 * buff :		data buffer
	 * cnt :		avialable data size
	 * needed_size:	we need data size
	 *
	 * return covert data size
	 */
	int (*filter)(void *buff, int *cnt, int needed_size);        /* define by device */
	/* lock */
	spinlock_t          pipe_lock;
	struct snd_dev_data *	pddata;
	struct mutex        mutex;
};

struct dsp_endpoints {
    struct dsp_pipe *out_endpoint;
    struct dsp_pipe *in_endpoint;
};




/**
 * filter
 **/
int convert_8bits_signed2unsigned(void *buffer, int *counter,int needed_size);
int convert_8bits_stereo2mono(void *buff, int *data_len,int needed_size);
int convert_8bits_stereo2mono_signed2unsigned(void *buff, int *data_len,int needed_size);
int convert_16bits_stereo2mono(void *buff, int *data_len,int needed_size);
int convert_16bits_stereo2mono_inno(void *buff, int *data_len,int needed_size);
int convert_16bits_stereomix2mono(void *buff, int *data_len,int needed_size);
int convert_32bits_stereo2_16bits_mono(void *buff, int *data_len, int needed_size);
int convert_32bits_2_20bits_tri_mode(void *buff, int *data_len, int needed_size);

/**
 * functions interface
 **/
loff_t xb_snd_dsp_llseek(struct file *file,
						 loff_t offset,
						 int origin,
						 struct snd_dev_data *ddata);

ssize_t xb_snd_dsp_read(struct file *file,
						char __user *buffer,
						size_t count,
						loff_t *ppos,
						struct snd_dev_data *ddata);

ssize_t xb_snd_dsp_write(struct file *file,
						 const char __user *buffer,
						 size_t count,
						 loff_t *ppos,
						 struct snd_dev_data *ddata);

unsigned int xb_snd_dsp_poll(struct file *file,
							   poll_table *wait,
							   struct snd_dev_data *ddata);

long xb_snd_dsp_ioctl(struct file *file,
					  unsigned int cmd,
					  unsigned long arg,
					  struct snd_dev_data *ddata);

int xb_snd_dsp_mmap(struct file *file,
					struct vm_area_struct *vma,
					struct snd_dev_data *ddata);

int xb_snd_dsp_open(struct inode *inode,
					struct file *file,
					struct snd_dev_data *ddata);

int xb_snd_dsp_release(struct inode *inode,
					   struct file *file,
					   struct snd_dev_data *ddata);

int xb_snd_dsp_probe(struct snd_dev_data *ddata);

int xb_snd_dsp_suspend(struct snd_dev_data *ddata);
int xb_snd_dsp_resume(struct snd_dev_data *ddata);
#endif //__XB_SND_DSP_H__
