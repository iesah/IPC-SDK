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
#include <sysutils/su_adc.h>

#define CHN_NUM 3

void *adc_get_voltage_thread(void *argc)
{
    int value = 0;
	int ret = 0;
    while(1)
    {
		ret = SU_ADC_GetChnValue(CHN_NUM, &value);
		if(ret){
			printf("Failed to get adc value!\n");
			return NULL;
		}

		printf("### su adc value is : %d mV ###\n", value / 10);
		sleep(1);
    }

	return NULL;
}

int main(int argc ,char *argv[])
{
	int ret = 0;

	ret = SU_ADC_Init();
    if (ret != 0) {
        printf("error: su adc init error!!!!!!\n");
		return -1;
	}

	/* enable adc chn */
	ret = SU_ADC_EnableChn(CHN_NUM);
	if (ret != 0) {
		printf("error: enable chn %d error!!!!!!\n", CHN_NUM);
		return -1;
	}

	/* get value thread */
	pthread_t adc_thread_id;
    ret = pthread_create(&adc_thread_id, NULL, adc_get_voltage_thread, NULL);
    if (ret != 0) {
        printf("error: pthread_create error!!!!!!\n");
        return -1;
    }
	pthread_join(adc_thread_id, NULL);

	/* disable adc chn */
	ret = SU_ADC_DisableChn(CHN_NUM);
	if (ret != 0) {
		printf("error: disable chn %d error!!!!!!\n", CHN_NUM);
		return -1;
	}

	/* exit adc */
	ret = SU_ADC_Exit();
    if (ret != 0) {
        printf("error: su adc exit error!!!!!!\n");
        return -1;
    }

	return ret;
}
