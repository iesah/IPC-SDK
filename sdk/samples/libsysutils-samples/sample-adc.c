/*
 * Ingenic SU ADC solution.
 *
 * Copyright (C) 2019 Ingenic Semiconductor Co.,Ltd
 * Author: Damon<jiansheng.zhang@ingenic.com>
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>

/* define ioctl command. they are fixed. don't modify! */
#define ADC_ENABLE 		0
#define ADC_DISABLE 	1
#define ADC_SET_VREF 	2
#define ADC_PATH_LEN    32

#define ADC_PATH    "/dev/ingenic_adc_aux_" /* adc channal 0-3 */
#define STD_VAL_VOLTAGE 1800 /* The unit is mv/1000. T10/T20 VREF=3300; T30/T21/T31/T40 VREF=1800 */

#define ENABLE2DISABLE_TIME_DEBUG

int fd;
int vol_n;

void *adc_get_voltage_thread(void *argc)
{
    long int value = 0;
	int ret = 0;
    while(1)
    {
#ifdef ENABLE2DISABLE_TIME_DEBUG
		struct  timeval   tv_begin, tv_end;
		gettimeofday(&tv_begin, NULL);
#endif
		// enable adc
		ret = ioctl(fd, ADC_ENABLE);
		if(ret){
			printf("Failed to enable adc!\n");
			break;
		}

		// get value
		ret = read(fd, &value, sizeof(int));
		if(!ret){
			printf("Failed to read adc value!\n");
			break;
		}

		//printf get value
		printf("### adc value is : %ldmV ###\n", value / 10);

		// disable adc
		ret = ioctl(fd, ADC_DISABLE);
		if(ret){
			printf("Failed to disable adc!\n");
			break;
		}

#ifdef ENABLE2DISABLE_TIME_DEBUG
		gettimeofday(&tv_end, NULL);
		printf("result time = %ld usec !!\n", (tv_end.tv_sec * 1000 + tv_end.tv_usec) - (tv_begin.tv_sec * 1000 + tv_begin.tv_usec));
#endif

        sleep(1);
    }

    /* close fd */
    close(fd);

	return NULL;
}

int main(int argc ,char *argv[])
{
	int ret = 0;
    int ch_id = 3; /* change test channal */
    char path[ADC_PATH_LEN];

    sprintf(path, "%s%d", ADC_PATH, ch_id);

	fd = open(path, O_RDONLY);
	if(fd < 0) {
		printf("sample_adc:open error !\n");
		ret = -1;
		goto error_open;
	}

	/* set reference voltage */
	vol_n = STD_VAL_VOLTAGE;
	ret = ioctl(fd, ADC_SET_VREF, &vol_n);
	if(ret){
		printf("Failed to set reference voltage!\n");
		goto error_vol;
	}

	/* get value thread */
    pthread_t adc_thread_id;
    ret = pthread_create(&adc_thread_id, NULL, adc_get_voltage_thread, NULL);
    if (ret != 0) {
        printf("error: pthread_create error!!!!!!");
        return -1;
    }
    pthread_join(adc_thread_id, NULL);

    return 0;

error_vol:
	close(fd);
error_open:
	return ret;
}
