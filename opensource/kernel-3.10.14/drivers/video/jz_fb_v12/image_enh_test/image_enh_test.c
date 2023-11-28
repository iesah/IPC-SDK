#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "image_enh_test.h"
#define KEY_COLOR_RED     0x00
#define KEY_COLOR_GREEN   0x04
#define KEY_COLOR_BLUE    0x00
#define KEY_COLOR_ALPHA   0xFF

#define HDMI_DEV  "/dev/hdmi"

#define SOURCE_BUFFER_SIZE 0x200000	/* 2M */
#define START_ADDR_ALIGN 0x1000	/* 4096 byte */
#define STRIDE_ALIGN 0x800	/* 2048 byte */
#define PIXEL_ALIGN 16		/* 16 pixel */

#define DBG_CMA(sss, aaa...)                                             \

/*contrast value--( 0~100 D:50 )*/
int setContrast(int mFd, int value)
{
	struct enh_luma tmp_luma;
	int tmp_value = value * 2047 / 100;
	if (ioctl(mFd, JZFB_GET_LUMA, &tmp_luma) < 0) {
		printf("JzFb get Contrast faild\n");
		return -1;
	}

	tmp_luma.contrast_en = 1;
	tmp_luma.contrast = tmp_value;

	if (ioctl(mFd, JZFB_SET_LUMA, &tmp_luma) < 0) {
		printf("JzFb set Contrast faild\n");
		return -1;
	}

	return 0;
}

/*brightness value--( 0~100 D:50 )*/
int setBrightness(int mFd, int value)
{
	struct enh_luma tmp_luma;
	int tmp_value;
	if (value >= 50)
		tmp_value = (value - 50) * 2047 / 100;
	else
		tmp_value = (100 - value) * 2047 / 100;

	if (ioctl(mFd, JZFB_GET_LUMA, &tmp_luma) < 0) {
		printf("JzFb get Brightness faild\n");
		return -1;
	}

	tmp_luma.brightness_en = 1;
	tmp_luma.brightness = tmp_value;

	if (ioctl(mFd, JZFB_SET_LUMA, &tmp_luma) < 0) {
		printf("JzFb set Brightness faild\n");
		return -1;
	}
	return 0;
}

#define P 3.1415926
#define E 0.000001
/*hue value--( 0~100 D:100)*/
int setHue(int mFd, int value)
{
	struct enh_hue tmp_hue;
	int val_s, val_c;
	double tmp_value, val_sin, val_cos;

	if (value == 25)
		value = 24;
	if (value == 100)
		value = 25;
	tmp_value = value * 2 * P / 100;

	val_sin = sin(tmp_value);
	val_cos = cos(tmp_value);

	if (val_sin >= -E && val_sin <= E) {
		val_s = 1024;
	} else if (val_sin > E) {
		val_s = val_sin * 1024 + 1 + 1024;
	} else {
		val_s = 3072 - (1 + val_sin) * 1024;
	}

	if (val_cos >= -E && val_cos <= E) {
		val_c = 1024;
	} else if (val_cos > E) {
		val_c = val_cos * 1024 + 1024;
	} else {
		val_c = 3072 - (1 + val_cos) * 1024;
	}

	printf("set=%d---val=%lf val_sin=%lf val_cos=%lf  val_s=%d val_c=%d \n",
	       value, tmp_value, val_sin, val_cos, val_s, val_c);
	tmp_hue.hue_en = 1;
	tmp_hue.hue_sin = val_s;
	tmp_hue.hue_cos = val_c;

	if (ioctl(mFd, JZFB_SET_HUE, &tmp_hue) < 0) {
		printf("JzFb set HueSin faild\n");
		return -1;
	}
	return 0;
}

/*saturation value--( 0~100 ) D:50 */
int setSaturation(int mFd, int value)
{
	struct enh_chroma tmp_chroma;
	int tmp_value = value * 2047 / 100;
	if (ioctl(mFd, JZFB_GET_CHROMA, &tmp_chroma) < 0) {
		printf("JzFb get Saturation  faild\n");
		return -1;
	}

	tmp_chroma.saturation_en = 1;
	tmp_chroma.saturation = tmp_value;

	if (ioctl(mFd, JZFB_SET_CHROMA, &tmp_chroma) < 0) {
		printf("JzFb set Saturation  faild\n");
		return -1;
	}
	return 0;
}

#define AMBIT 100
void calcule_table(int value, short table[1024])
{
	double intersect;
	int inter, i;
	intersect = 1024 * value / 100;
	inter = (int)intersect;

	for (i = 0; i < inter; i++) {
		table[i] = i * ((double)(AMBIT - value) / value);
	}
	for (i = inter; i < 1024; i++) {
		table[i] =
		    (value * (double)(i - 1023) / (AMBIT - value)) + 1023;
	}
#if 0
	for (i = 0; i < 1024; i++) {
		printf("t[%4d]=%4d ", i, table[i]);
		if ((i + 1) % 10 == 0)
			printf("\n");
	}
#endif
}

/*gamma value--( 0~100 ) D:50 */
int setGamma(int mFd, int value)
{
	struct enh_gamma tmp_gamma;
	int i, tmp_value = 100 - value;

	calcule_table(tmp_value, (short *)tmp_gamma.gamma_data);
	tmp_gamma.gamma_en = 1;

	if (ioctl(mFd, JZFB_SET_GAMMA, &tmp_gamma) < 0) {
		printf("JZFB_SET_GAMMA faild\n");
		return -1;
	}
	return 0;
}

/*vee value--( 0~100 ) D:50 */
int setVee(int mFd, int value)
{
	struct enh_vee tmp_vee;
	int i, tmp_value = 100 - value;

	calcule_table(tmp_value, (short *)tmp_vee.vee_data);
	tmp_vee.vee_en = 1;

	if (ioctl(mFd, JZFB_SET_VEE, &tmp_vee) < 0) {
		printf("JZFB_SET_VEE faild\n");
		return -1;
	}
	return 0;
}

int enable_enh(int mFd, int value)
{
	if (ioctl(mFd, JZFB_ENABLE_ENH, &value) < 0) {
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int mFd = open("/dev/fb0", O_RDWR);
	if (!strcmp(argv[1], "contrast")) {
		setContrast(mFd, atoi(argv[2]));
		printf("please enter a number (0 ~ 100)\n");
	}

	if (!strcmp(argv[1], "brightness")) {
		setBrightness(mFd, atoi(argv[2]));
		printf("please enter a number (0 ~ 100)\n");
	}

	if (!strcmp(argv[1], "hue")) {
		setHue(mFd, atoi(argv[2]));
		printf("please enter a number (0 ~ 100)\n");
	}

	if (!strcmp(argv[1], "saturation")) {
		setSaturation(mFd, atoi(argv[2]));
		printf("please enter a number (0 ~ 100)\n");
	}

	if (!strcmp(argv[1], "gamma")) {
		setGamma(mFd, atoi(argv[2]));
		printf("please enter a number (0 ~ 100)\n");
	}

	if (!strcmp(argv[1], "vee")) {
		setVee(mFd, atoi(argv[2]));
		printf("please enter a number (0 ~ 100)\n");
	}

	return 0;
}
