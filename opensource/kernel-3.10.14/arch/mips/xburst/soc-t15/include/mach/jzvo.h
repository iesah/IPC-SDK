#ifndef __JZVO_H__
#define __JZVO_H__

enum {
	BT656	= 0,
	BT1120
};

enum {
	YUV422		= 0,
	YU12,
	NV12,
	NV21
};

enum {
	LEEND		= 0,	/* Little endianness */
	BIGEND,				/* Big endianness */
};

enum {
	BTMFST		= 0,	/* Bottom field first */
	TOPFST				/* Top field first */
};

enum {
	INNER_SYNC	= 0,
	OUT_SYNC,
};

enum {
	LOW_ACT		= 0,
	HIGH_ACT,
};

enum {
	INNERSYNC_525I,
	INNERSYNC_625I,
	INNERSYNC_720P24,
	INNERSYNC_720P25,
	INNERSYNC_720P30,
	INNERSYNC_720P60,
	INNERSYNC_1080P24,
	INNERSYNC_1080P25,
	INNERSYNC_1080P30,
	INNERSYNC_1080I50,
	INNERSYNC_1080I60,

	OUTSYNC_525I,
	OUTSYNC_625I,
	OUTSYNC_720P24,
	OUTSYNC_720P30,
	OUTSYNC_720P50,
	OUTSYNC_720P60,
	OUTSYNC_1080P24,
	OUTSYNC_1080P30,
	OUTSYNC_1080I50,
	OUTSYNC_1080I60,
	OUTSYNC_1080P50,
	OUTSYNC_1080P60,
};

#ifdef CONFIG_VO_FMT_YUV422
#define BUFFMT	YUV422
#elif defined(CONFIG_VO_FMT_YU12)
#define BUFFMT	YU12
#elif defined(CONFIG_VO_FMT_YU21)
#define BUFFMT	YU21
#elif defined(CONFIG_VO_FMT_NV12)
#define BUFFMT	NV12
#elif defined(CONFIG_VO_FMT_NV21)
#define BUFFMT	NV21
#endif

#define MHZ 1000000

struct jzvo_config {
	unsigned int config_name;
	unsigned int xres;
	unsigned int yres;
	unsigned int clk;
	union {
		struct {
			unsigned short aw;		/* active width */
			unsigned short bw;		/* blank width */
		};
		unsigned int tw;
	};
	union {
		struct {
			unsigned short bbh0;	/* odd field bottom blank height */
			unsigned short tbh0;	/* odd field top blank height */
		};
		unsigned int bh0;
	};
	union {
		struct {
			unsigned short bbh1;	/* even field bottom blank height */
			unsigned short tbh1;	/* even field top blank height */
		};
		unsigned int bh1;
	};
	unsigned int ah0;				/* odd field active height */
	unsigned int ah1;				/* even field active height */
	union {
		struct {
			unsigned short hpe;		/* horizontal pulse end */
			unsigned short hps;		/* horizontal pulse start */
		};
		unsigned int hpc;
	};
	union {
		struct {
			unsigned short vpe0;	/* odd field vertical vertical pulse end */
			unsigned short vps0;	/* odd field vertical pulse start */
		};
		unsigned int vpc0;
	};
	union {
		struct {
			unsigned short vpe1;	/* even field vertical pulse end */
			unsigned short vps1;	/* even field vertical pulse start */
		};
		unsigned int vpc1;
	};
	union {
		struct {
			unsigned short hdee;	/* horizontal DE end */
			unsigned short hdes;	/* horizontal DE start */
		};
		unsigned int hdec;
	};
	union {
		struct {
			unsigned short vdee0;	/* odd field vertical de end */
			unsigned short vdes0;	/* odd field vertical de start */
		};
		unsigned int vdec0;
	};
	union {
		struct {
			unsigned short vdee1;	/* even field vertical DE end */
			unsigned short vdes1;	/* even field vertical DE start */
		};
		unsigned int vdec1;
	};
	union {
		struct {
			unsigned short vpss0;
			unsigned short vpss1;
		};
		unsigned int vpcs;
	};
	union {
		struct {
			unsigned buffmt		:3; /* 0 ~ 2, buffer format */
			unsigned syncs		:1; /* 3, sync system: out sync or inner sync */
			unsigned fseq		:1; /* 4, field sequence */
			unsigned progr		:1; /* 5, progressive or interlaced */
			unsigned auxen		:1;	/* 6, auxiliary data enable*/
			unsigned			:2;	/* 7, reserved */
			unsigned hsyncp		:1; /* 9, hsync polarity */
			unsigned vsyncp		:1; /* 10, vsync polarity */
			unsigned dep		:1; /* 11, de polarity */
			unsigned ycpos		:1; /* 12, gray(Y) and color(C) data sequence */
			unsigned ycchan		:1; /* 13, YC channel count */
			unsigned ddzero		:1; /* 14, fill zero when data disable */
			unsigned addcrc		:1; /* 15, */
			unsigned addln		:1; /* 16, */
			unsigned addseav	:1; /* 17, add sav and eav */
			unsigned			:1; /* 18, */
			unsigned btmode		:1; /* 19, bt565 or bt1120 */
			unsigned auxstren	:1; /* 20, auxiliary data stride enable */
			unsigned pixstren 	:1; /* 21, pixel data stride enable */
			unsigned aux0en		:1; /* 22, */
			unsigned aux1en		:1; /* 23, */
			unsigned aux2en		:1; /* 24, */
			unsigned low2bits	:2; /* 25 ~ 26 */
			unsigned			:5; /* 27 ~ 31 */
		};
		unsigned int ctrl;
	};
};

struct jzvo_platform_data {
	struct jzvo_config *vo_config;
};

#endif /* __JZVO_H__ */
