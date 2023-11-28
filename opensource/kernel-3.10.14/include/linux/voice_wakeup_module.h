#ifndef __VOICE_WAKEUP_MODULE_H__
#define __VOICE_WAKEUP_MODULE_H__


enum open_mode {
	EARLY_SLEEP = 1,
	DEEP_SLEEP,
	NORMAL_RECORD,
	NORMAL_WAKEUP
};



#define		SLEEP_BUFFER_SIZE	(32 * 1024)
#define		NR_BUFFERS			(8)

struct sleep_buffer {
	unsigned char *buffer[NR_BUFFERS];
	unsigned int nr_buffers;
	unsigned long total_len;
};


#define DMIC_IOCTL_SET_SAMPLERATE	0x200



#define WAKEUP_HANDLER_ADDR	(0x8ff00004)
#define SYS_WAKEUP_OK		(0x1)
#define SYS_WAKEUP_FAILED	(0x2)
#define SYS_NEED_DATA		(0x3)

#define TCSM_BANK5_V		(0xb3427000)
#define TCSM_BUFFER_SIZE	(4096)

#define TCSM_DATA_BUFFER_ADDR	(0xb3422000) /* bank0 */
#define TCSM_DATA_BUFFER_SIZE	(4096)

int wakeup_module_open(int mode);

int wakeup_module_close(int mode);

void wakeup_module_cache_prefetch(void);

int wakeup_module_handler(int par);

dma_addr_t wakeup_module_get_dma_address(void);

unsigned char wakeup_module_get_resource_addr(void);

int wakeup_module_ioctl(int cmd, unsigned long args);


int wakeup_module_process_data(void);

int wakeup_module_is_cpu_wakeup_by_dmic(void);


int wakeup_module_set_sleep_buffer(struct sleep_buffer *);

int wakeup_module_get_sleep_process(void);

int wakeup_module_wakeup_enable(int enable);

int wakeup_module_is_wakeup_enabled(void);




#endif
