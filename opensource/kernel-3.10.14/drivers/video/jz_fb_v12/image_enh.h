#ifndef _IMAGE_ENH_H_
#define _IMAGE_ENH_H_

struct enh_gamma {
	__u32 gamma_en:1;
	__u16 gamma_data[1024];
};

struct enh_csc {
	__u32 rgb2ycc_en:1;
	__u32 rgb2ycc_mode;
	__u32 ycc2rgb_en:1;
	__u32 ycc2rgb_mode;
};

struct enh_luma {
	__u32 brightness_en:1;
	__u32 brightness;
	__u32 contrast_en:1;
	__u32 contrast;
};

struct enh_hue {
	__u32 hue_en:1;
	__u32 hue_sin;
	__u32 hue_cos;
};

struct enh_chroma {
	__u32 saturation_en:1;
	__u32 saturation;
};

struct enh_vee {
	__u32 vee_en:1;
	__u16 vee_data[1024];
};

struct enh_dither {
	__u32 dither_en:1;
	__u32 dither_red;
	__u32 dither_green;
	__u32 dither_blue;
};

/* image enhancement ioctl commands */
#define JZFB_SET_GAMMA			_IOW('F', 0x142, struct enh_gamma)
#define JZFB_GET_CSC			_IOR('F', 0x143, struct enh_csc)
#define JZFB_SET_CSC			_IOW('F', 0x144, struct enh_csc)
#define JZFB_GET_LUMA			_IOR('F', 0x145, struct enh_luma)
#define JZFB_SET_LUMA			_IOW('F', 0x146, struct enh_luma)
#define JZFB_GET_HUE			_IOR('F', 0x147, struct enh_hue)
#define JZFB_SET_HUE			_IOW('F', 0x148, struct enh_hue)
#define JZFB_GET_CHROMA			_IOR('F', 0x149, struct enh_chroma)
#define JZFB_SET_CHROMA			_IOW('F', 0x150, struct enh_chroma)
#define JZFB_SET_VEE			_IOW('F', 0x152, struct enh_vee)
/* Reserved for future extend */
#define JZFB_GET_DITHER			_IOR('F', 0x158, struct enh_dither)
#define JZFB_SET_DITHER			_IOW('F', 0x159, struct enh_dither)
#define JZFB_ENABLE_ENH			_IOW('F', 0x160, struct enh_dither)
#define JZFB_RESET_ENH			_IOW('F', 0x161, struct enh_dither)
#define JZFB_RECOVER_ENH		_IOW('F', 0x162, struct enh_dither)
#endif /* _IMAGE_ENH_H_ */
int jzfb_image_enh_ioctl_internal(struct fb_info *info, unsigned int cmd,
				  unsigned long arg);
