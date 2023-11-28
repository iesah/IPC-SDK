#ifndef ANDROID_INGENIC_FRAMEBUFFER_CONTROLLER_H
#define ANDROID_INGENIC_FRAMEBUFFER_CONTROLLER_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#define __u32 unsigned int
#define __u16 unsigned short

/* image enhancement */
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

#define JZFB_GET_MODENUM        _IOR('F', 0x100, int)
#define JZFB_GET_MODELIST       _IOR('F', 0x101, int)
#define JZFB_SET_VIDMEM			_IOW('F', 0x102, unsigned int *)
#define JZFB_SET_MODE           _IOW('F', 0x103, int)
#define JZFB_GET_RESOLUTION     _IOWR('F', 0x105, struct jzfb_mode_res)

#define JZFB_ENABLE			_IOW('F', 0x104, int)

/* Reserved for future extend */
#define JZFB_SET_FG_SIZE		_IOW('F', 0x116, struct jzfb_fg_size)
#define JZFB_GET_FG_SIZE		_IOWR('F', 0x117, struct jzfb_fg_size)
#define JZFB_SET_FG_POS			_IOW('F', 0x118, struct jzfb_fg_pos)
#define JZFB_GET_FG_POS			_IOWR('F', 0x119, struct jzfb_fg_pos)
/* Reserved for future extend */
#define JZFB_GET_BUFFER         _IOR('F', 0x120, int)
#define JZFB_SET_ALPHA			_IOW('F', 0x123, struct jzfb_fg_alpha)
#define JZFB_SET_BACKGROUND		_IOW('F', 0x124, struct jzfb_bg)
#define JZFB_SET_COLORKEY		_IOW('F', 0x125, struct jzfb_color_key)
#define JZFB_AOSD_EN			_IOW('F', 0x126, struct jzfb_aosd)
#define JZFB_16X16_BLOCK_EN		_IOW('F', 0x127, int)
#define JZFB_IPU0_TO_BUF		_IOW('F', 0x128, int)
#define JZFB_ENABLE_LCDC_CLK		_IOW('F', 0x130, int)
/* Reserved for future extend */
#define JZFB_ENABLE_FG0			_IOW('F', 0x139, int)
#define JZFB_ENABLE_FG1			_IOW('F', 0x140, int)

/* image enhancement ioctl commands */
#define JZFB_GET_GAMMA                  _IOR('F', 0x141, struct enh_gamma)
#define JZFB_SET_GAMMA                  _IOW('F', 0x142, struct enh_gamma)
#define JZFB_GET_CSC                    _IOR('F', 0x143, struct enh_csc)
#define JZFB_SET_CSC                    _IOW('F', 0x144, struct enh_csc)
#define JZFB_GET_LUMA                   _IOR('F', 0x145, struct enh_luma)
#define JZFB_SET_LUMA                   _IOW('F', 0x146, struct enh_luma)
#define JZFB_GET_HUE                    _IOR('F', 0x147, struct enh_hue)
#define JZFB_SET_HUE                    _IOW('F', 0x148, struct enh_hue)
#define JZFB_GET_CHROMA                 _IOR('F', 0x149, struct enh_chroma)
#define JZFB_SET_CHROMA                 _IOW('F', 0x150, struct enh_chroma)
#define JZFB_GET_VEE                    _IOR('F', 0x151, struct enh_vee)
#define JZFB_SET_VEE                    _IOW('F', 0x152, struct enh_vee)
/* Reserved for future extend */
#define JZFB_GET_DITHER                 _IOR('F', 0x158, struct enh_dither)
#define JZFB_SET_DITHER                 _IOW('F', 0x159, struct enh_dither)

#define JZFB_ENABLE_ENH			_IOW('F', 0x160, struct enh_dither)
#define JZFB_RESET_ENH			_IOW('F', 0x161, struct enh_dither)
#define JZFB_RECOVER_ENH		_IOW('F', 0x162, struct enh_dither)

/* Reserved for future extend */
#define JZFB_SET_VSYNCINT		_IOW('F', 0x210, int)

#define JZFB_SET_PAN_SYNC		_IOW('F', 0x220, int)

/* hdmi ioctl commands /dev/hdmi */
#define HDMI_POWER_OFF			_IO('F', 0x301)
#define	HDMI_VIDEOMODE_CHANGE		_IOW('F', 0x302, int)
#define	HDMI_POWER_ON			_IO('F', 0x303)
#define HDMI_GET_TVMODENUM              _IOR('F', 0x304, int)
#define HDMI_GET_TVMODE		        _IOR('F', 0x305, int)
#define HDMI_POWER_OFF_COMPLETE         _IO('F', 0x306)

#define MODE_NAME_LEN 32
#endif // ANDROID_INGENIC_FRAMEBUFFER_CONTROLLER_H
