/*
* Copyright© 2014 Ingenic Semiconductor Co.,Ltd
*
* Author: qipengzhen <aric.pzqi@ingenic.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/
#include <linux/slab.h>
#include <media/v4l2-mem2mem.h>

#include "api/helix_x264_enc.h"

#include "h264enc/common.h"
#include "h264enc/set.h"
#include "h264e.h"


static uint8_t cqm_8iy_tab[64] = {
    6+10,10+10,13+10,16+10,18+10,23+10,25+10,27+10,
    10+10,11+10,16+10,18+10,23+10,25+10,27+10,29+10,
    13+10,16+10,18+10,23+10,25+10,27+10,29+10,31+10,
    16+10,18+10,23+10,25+10,27+10,29+10,31+10,33+10,
    18+10,23+10,25+10,27+10,29+10,31+10,33+10,36+10,
    23+10,25+10,27+10,29+10,31+10,33+10,36+10,38+10,
    25+10,27+10,29+10,31+10,33+10,36+10,38+10,40+10,
    27+10,29+10,31+10,33+10,36+10,38+10,40+10,42+10
};
static uint8_t cqm_8py_tab[64] = {
    9+10,13+10,15+10,17+10,19+10,21+10,22+10,24+10,
    13+10,13+10,17+10,19+10,21+10,22+10,24+10,25+10,
    15+10,17+10,19+10,21+10,22+10,24+10,25+10,27+10,
    17+10,19+10,21+10,22+10,24+10,25+10,27+10,28+10,
    19+10,21+10,22+10,24+10,25+10,27+10,28+10,30+10,
    21+10,22+10,24+10,25+10,27+10,28+10,30+10,32+10,
    22+10,24+10,25+10,27+10,28+10,30+10,32+10,33+10,
    24+10,25+10,27+10,28+10,30+10,32+10,33+10,35+10
};


/* static int      rc_deadzone[9]         = {0x1d,0x12,0x15,0xb,0x1d,0x12,0x1d,0x15,0xb}; */
static uint16_t rc_skin_ofst[4]        = {97, 183, 271, 884};//[0]: 1.5, [1]:0.4, [2]:0.6, [3]: 5.1
static uint8_t  rc_mult_factor[3]      = {3,9,5};
//static int8_t   rc_skin_qp_ofst[4]     = {-3, -2, -1, 0};
static int8_t   rc_skin_qp_ofst[4]     = {-6, -4, -2, 0};
static uint8_t  rc_skin_pxlu_thd[3][2] = {{100, 120},{50, 100},{120, 150}};
static uint8_t  rc_skin_pxlv_thd[3][2] = {{140, 175},{125, 140},{175, 225}};
static uint8_t  rc_shift_factor[3][3]  = {{3,5,8},{4,5,6},{4,5,6}};//[0][]:0.4, [1][]:0.6, [2][]: 5.1



/*TODO: export to high level ????*/
int h264e_set_params(struct h264e_ctx *ctx, unsigned id, unsigned int val)
{
	return 0;
}

int h264e_get_params(struct h264e_ctx *ctx, int id)
{
	//struct h264e_params *p = &ctx->p;
	unsigned int val = 0;
	/*switch*/

	return val;
}

int h264e_set_fmt(struct h264e_ctx *ctx, int width, int height, int format)
{
	struct h264e_params *p = &ctx->p;
	int ret = 0;

	p->width = width;
	p->height = height;
	switch(format) {
		case HELIX_NV12_MODE:
		case HELIX_NV21_MODE:
		case HELIX_TILE_MODE:
			p->format = format;
			break;
		default:
			pr_err("Unsupported pix fmt: %x\n", format);
			ret = -EINVAL;
			break;
	}

	ctx->framesize = width * height;

	return ret;
}

//to profile_idc
static int v4l2_profile(int i_profile_idc)
{
	switch (i_profile_idc) {
		case PROFILE_BASELINE:           return V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
		case PROFILE_MAIN:               return V4L2_MPEG_VIDEO_H264_PROFILE_MAIN;
		case PROFILE_HIGH10:             return V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_10;
		case PROFILE_HIGH422:            return V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_422;
		case PROFILE_HIGH444_PREDICTIVE: return V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_444_PREDICTIVE;
		default: return -EINVAL;
	}

}

static int h264e_profile_idc(int v4l2_profile)
{
	switch (v4l2_profile) {
		case V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE: 		return PROFILE_BASELINE;
		case V4L2_MPEG_VIDEO_H264_PROFILE_MAIN: 		return PROFILE_MAIN;
		case V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_10: 		return PROFILE_HIGH10;
		case V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_422: 		return PROFILE_HIGH422;
		case V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_444_PREDICTIVE: 	return PROFILE_HIGH444_PREDICTIVE;
		default: return -EINVAL;
	}
}

static int v4l2_h264_level(int i_level_idc)
{
	switch (i_level_idc) {
		case 10: return V4L2_MPEG_VIDEO_H264_LEVEL_1_0;
		case 9:  return V4L2_MPEG_VIDEO_H264_LEVEL_1B;
		case 11: return V4L2_MPEG_VIDEO_H264_LEVEL_1_1;
		case 12: return V4L2_MPEG_VIDEO_H264_LEVEL_1_2;
		case 13: return V4L2_MPEG_VIDEO_H264_LEVEL_1_3;
		case 20: return V4L2_MPEG_VIDEO_H264_LEVEL_2_0;
		case 21: return V4L2_MPEG_VIDEO_H264_LEVEL_2_1;
		case 22: return V4L2_MPEG_VIDEO_H264_LEVEL_2_2;
		case 30: return V4L2_MPEG_VIDEO_H264_LEVEL_3_0;
		case 31: return V4L2_MPEG_VIDEO_H264_LEVEL_3_1;
		case 32: return V4L2_MPEG_VIDEO_H264_LEVEL_3_2;
		case 40: return V4L2_MPEG_VIDEO_H264_LEVEL_4_0;
		case 41: return V4L2_MPEG_VIDEO_H264_LEVEL_4_1;
		default: return -EINVAL;
	}
}

static int h264e_level_idc(int level)
{
	switch (level) {
		case V4L2_MPEG_VIDEO_H264_LEVEL_1_0: return 10;
		case V4L2_MPEG_VIDEO_H264_LEVEL_1B:  return 9;
		case V4L2_MPEG_VIDEO_H264_LEVEL_1_1: return 11;
		case V4L2_MPEG_VIDEO_H264_LEVEL_1_2: return 12;
		case V4L2_MPEG_VIDEO_H264_LEVEL_1_3: return 13;
		case V4L2_MPEG_VIDEO_H264_LEVEL_2_0: return 20;
		case V4L2_MPEG_VIDEO_H264_LEVEL_2_1: return 21;
		case V4L2_MPEG_VIDEO_H264_LEVEL_2_2: return 22;
		case V4L2_MPEG_VIDEO_H264_LEVEL_3_0: return 30;
		case V4L2_MPEG_VIDEO_H264_LEVEL_3_1: return 31;
		case V4L2_MPEG_VIDEO_H264_LEVEL_3_2: return 32;
		case V4L2_MPEG_VIDEO_H264_LEVEL_4_0: return 40;
		case V4L2_MPEG_VIDEO_H264_LEVEL_4_1: return 41;
		default: return -EINVAL;
	}
}

/* ----------------------------- Picture Parameter Sets ---------------*/
static void h264e_sps_init(struct h264e_ctx *ctx, h264_sps_t *sps, int i_id)
{
	struct h264e_params *p = &ctx->p;
	int max_frame_num;

	sps->i_id 			= i_id;
	sps->i_mb_width 		= C_ALIGN(p->width, 16) / 16;
	sps->i_mb_height 		= C_ALIGN(p->height, 16) / 16;
	sps->i_chroma_format_idc 	= CHROMA_420; //vpu格式决定的...
	sps->b_qpprime_y_zero_transform_bypass = 0; //TODO，bool值，先设置为0.与rc有关，。。。
	sps->i_profile_idc 		= h264e_profile_idc(p->h264_profile); //要转换成协议规定的值
	sps->b_constraint_set0 		= sps->i_profile_idc == PROFILE_BASELINE; //与profile有关的规定.
	sps->b_constraint_set1 		=  sps->i_profile_idc <= PROFILE_MAIN;
	sps->b_constraint_set2 		= 0;
	sps->b_constraint_set3 		= 0;

	sps->i_level_idc 		= h264e_level_idc(p->h264_level);
	if((sps->i_level_idc == 9) && (sps->i_profile_idc == PROFILE_BASELINE || sps->i_profile_idc == PROFILE_MAIN)) {
		sps->b_constraint_set0 	= 1; /* level 1b with Baseline or Main profile is signalled via constraint_set3 */
		sps->i_level_idc 	= 11;
	}


	sps->i_num_ref_frames 		= p->max_ref_pic; //参考帧数为一帧？是软件参数决定还是vpu硬件决定的。。

	sps->vui.i_num_reorder_frames 	= 0; //TODO
	sps->vui.i_max_dec_frame_buffering = sps->i_num_ref_frames; //TODO, 不知道vui是什么意思?

	max_frame_num = 1; /* TODO: Num of refs + current frame */
	sps->i_log2_max_frame_num 	= 4;
	while((1 << sps->i_log2_max_frame_num) <= max_frame_num)
		sps->i_log2_max_frame_num++;

	//sps->i_poc_type = p->num_bframe || p->interlaced ? 0 : 2; //TODO, 0个B帧，不支持场分离模式。这个应该是由vpu硬件决定的吧？, 最好还是从param传过来.
	/*固定2，图像顺序与解码顺序一致.*/
	sps->i_poc_type = 2; //TODO, 0个B帧，不支持场分离模式。这个应该是由vpu硬件决定的吧？, 最好还是从param传过来.

	if(sps->i_poc_type == 0) {
		/*目前不支持*/
	}

	sps->b_vui = 0; //vui信息可以从哪里来的??, 编码层用来干什么?

	sps->b_gaps_in_frame_num_value_allowed = 0;
	sps->b_frame_mbs_only = 1; //不支持场分离模式.
	if(!sps->b_frame_mbs_only)
		sps->i_mb_height = (sps->i_mb_height + 1) & ~1;
	sps->b_mb_adaptive_frame_field = p->interlaced;
	sps->b_direct8x8_inference = 1;
	sps->b_crop = 0;	/*TODO .... From sw */

	//TODO:干什么用的还不知道.
//	h264_sps_init_reconfigurable(sps, sw);


	sps->vui.b_overscan_info_present = 0;
	sps->vui.b_overscan_info = 0;

	sps->i_cqm_preset = 0;	//使用flat16量化表??
	sps->b_avcintra = 0;	//不处理这类的. seq_scaling_list_present_flag=0, 解码时应该推断出来.??怎么推断?

}

static void h264e_pps_init(struct h264e_ctx *ctx, h264_pps_t *pps, int i_id, h264_sps_t *sps)
{
	struct h264e_params *p = &ctx->p;

	pps->i_id = i_id;
	pps->i_sps_id = sps->i_id;
	pps->b_cabac = p->i_cabac;

	pps->b_pic_order = 0;
	pps->i_num_slice_groups = 1; /* base & main profile.*/

	pps->i_num_ref_idx_l0_default_active = p->max_ref_pic;
	pps->i_num_ref_idx_l1_default_active = 1;

	pps->b_weighted_pred = 0;
	pps->b_weighted_bipred = 0;

	pps->i_pic_init_qp = 0; //TODO;
	pps->i_pic_init_qs = 26; //TODO

	pps->i_chroma_qp_index_offset = 0; //TODO;
	pps->b_deblocking_filter_control = 0; //TODO, 应该为１
	pps->b_constrained_intra_pred = 0;
	pps->b_redundant_pic_cnt = 0;

	pps->b_transform_8x8_mode = p->h264_8x8_transform; //base and main 不支持，应该固定为0.
}


static int encapsulate_nal(bs_t *s, int s_off, char *dst, int i_type, int i_ref_idc)
{

	uint8_t *src = s->p_start + s_off;
	uint8_t *end = src + (bs_pos(s) / 8);
	char *orig_dst = dst;

	printk("======i_nal_type %d\n", i_type);
	printk("======i_nal_ref_idc= %d\n", i_ref_idc);
	printk("=====bs_pos(s) %d\n", bs_pos(s));

	/* prefix */
	*dst++ = 0x00;
	*dst++ = 0x00;
	*dst++ = 0x00;
	*dst++ = 0x01;

	/* nal header */
	*dst++ = (0 << 7) | (i_ref_idc << 5) | (i_type);


	if(src < end) *dst++ = *src++;
	if(src < end) *dst++ = *src++;
	while( src < end )
	{
		if( src[0] <= 0x03 && !dst[-2] && !dst[-1] )
			*dst++ = 0x03;
		*dst++ = *src++;
	}

	return dst - orig_dst; // 打包的总大小. framesize;
}

/**
* @h264e_generate_headers generate sps/pps headers according to params.
*			and store the header until encoder closed.
* @ctx h264e_ctx.
*
* @Return 0
*/
int h264e_generate_headers(struct h264e_ctx *ctx)
{
	//struct h264e_params *p = &ctx->p;
	int framesize = ctx->framesize;
	/* used to encode header for tmp.*/
	char *bs_tmp = NULL;
	char *bs_header = ctx->bs_header;
	bs_t bs;
	int bs_size = 0;
	h264_sps_t *sps = &ctx->sps;
	h264_pps_t *pps = &ctx->pps;

	bs_tmp = kzalloc(framesize, GFP_KERNEL);
	if(!bs_tmp) {
		return -ENOMEM;
	}

	h264e_sps_init(ctx, sps, 0);
	h264e_pps_init(ctx, pps, 0, sps);

	/* 1. encode sps */
	bs_init(&bs, bs_tmp, framesize);
	h264e_sps_write(&bs, sps);

	bs_size = encapsulate_nal(&bs, 0, bs_header, NAL_SPS, NAL_PRIORITY_HIGHEST);

	ctx->bs_header_size += bs_size;
	bs_header	    += bs_size;

	/* 2. encode pps */
	bs_init(&bs, bs_tmp, framesize);
	h264e_pps_write(&bs, sps, pps);

	bs_size = encapsulate_nal(&bs, 0, bs_header, NAL_PPS, NAL_PRIORITY_HIGHEST);
	ctx->bs_header_size += bs_size;
	bs_header 	   += bs_size;


	if(ctx->bs_header_size > (MAX_BS_HEADER_SIZE - 512)) {
		pr_warn("bs_header buffer almost overflow!\n");
	}

	/* 3. encode sei */

	/* 4. encode more */

	/* release temp buffer. */
	kfree(bs_tmp);
	return 0;
}

static int h264e_slice_init(struct h264e_ctx * ctx, int i_nal_type)
{
	if(i_nal_type == NAL_SLICE_IDR) {

		h264e_slice_header_init(&ctx->sh, &ctx->sps, &ctx->pps, ctx->p.i_idr_pic_id, ctx->i_frame, ctx->p.i_qp);
		ctx->p.i_idr_pic_id ^= 1;
	} else {
		h264e_slice_header_init(&ctx->sh, &ctx->sps, &ctx->pps, -1, ctx->i_frame, ctx->p.p_qp);

		ctx->sh.i_num_ref_idx_l0_active = 1;
		ctx->sh.i_num_ref_idx_l1_active = 1;
	}

	return 0;
}

void H264E_DumpInfo(_H264E_SliceInfo *s)
{
	printk("s->des_va   = 0x%08x\n", (unsigned int)s->des_va);
	printk("s->des_pa   = 0x%08x\n", s->des_pa);
//	printk("s->emc_bs_va = 0x%08x\n", s->emc_bs_va);
	printk("s->emc_bs_pa = 0x%08x\n", s->emc_bs_pa);
	printk("s->frame_type = 0x%x\n", s->frame_type);
	printk("s->raw_format =   %d\n", s->raw_format);
	printk("s->mb_width  = 	  %d\n", s->mb_width);
	printk("s->mb_height =    %d\n", s->mb_height);
	printk("s->first_mby =    %d\n", s->first_mby);
	printk("s->last_mby  =    %d\n", s->last_mby);
	printk("s->frame_width =  %d\n", s->frame_width);
	printk("s->frame_height = %d\n", s->frame_height);
	printk("s->stride[0]    = %d\n", s->stride[0]);
	printk("s->stride[1]    = %d\n", s->stride[1]);
	printk("s->state	= %x\n", (unsigned int)s->state);
	printk("s->raw[0]   = 0x%08x\n", s->raw[0]);
	printk("s->raw[1]   = 0x%08x\n", s->raw[1]);
	printk("s->raw[2]   = 0x%08x\n", s->raw[2]);
	printk("s->fb[0][0] = 0x%08x\n", s->fb[0][0]);
	printk("s->fb[0][1] = 0x%08x\n", s->fb[0][1]);
	printk("s->fb[1][0] = 0x%08x\n", s->fb[1][0]);
	printk("s->fb[1][1] = 0x%08x\n", s->fb[1][1]);
	printk("s->fb[2][0] = 0x%08x\n", s->fb[2][0]);
	printk("s->fb[2][1] = 0x%08x\n", s->fb[2][1]);
	printk("s->jh[0][0] = 0x%08x\n", s->jh[0][0]);
	printk("s->jh[0][1] = 0x%08x\n", s->jh[0][1]);
	printk("s->jh[1][0] = 0x%08x\n", s->jh[1][0]);
	printk("s->jh[1][1] = 0x%08x\n", s->jh[1][1]);
	printk("s->jh[2][0] = 0x%08x\n", s->jh[2][0]);
	printk("s->jh[2][1] = 0x%08x\n", s->jh[2][1]);
	printk("s->spe_y_addr = 0x%08x\n", s->spe_y_addr);
	printk("s->spe_c_addr = 0x%08x\n", s->spe_c_addr);
	printk("s->emc_recon_pa= 0x%08x\n", s->emc_recon_pa);
	printk("s->emc_qpt_pa  = 0x%08x\n", s->emc_qpt_pa);
	printk("s->emc_mv_pa   = 0x%08x\n", s->emc_mv_pa);
	printk("s->emc_se_pa   = 0x%08x\n", s->emc_se_pa);
	printk("s->emc_rc_pa   = 0x%08x\n", s->emc_rc_pa);
	printk("s->emc_cpx_pa  = 0x%08x\n", s->emc_cpx_pa);
	printk("s->emc_mod_pa  = 0x%08x\n", s->emc_mod_pa);
	printk("s->emc_ncu_pa  = 0x%08x\n", s->emc_ncu_pa);
	printk("s->emc_sad_pa  = 0x%08x\n", s->emc_sad_pa);
}



/**
* @h264e_fill_slice_info
*
* @ctx
* @bs_off	指定vpu输出码流相对bs起始位置的偏移.
*
* @Return -
*/
static int h264e_fill_slice_info(struct h264e_ctx *ctx, int bs_off)
{
	int i = 0;
	int ret = 0;
	unsigned char bmap[3][3] = {
		{0, 2, 1},
		{1, 0, 2},
		{2, 1, 0}
	};

	//unsigned char *bufidx = bmap[sw->i_frame % (sw->max_ref_pic + 1)]; /*TODO ,只参考一帧*/
	unsigned char *bufidx = bmap[ctx->i_frame % 3]; /*TODO ,只参考一帧*/
	H264E_SliceInfo_t *s = ctx->s;


	h264_sps_t *sps 	= &ctx->sps;
	//h264_pps_t *pps 	= &ctx->pps;

	/* 1. rc output [11] */
	{
		s->mb_width 		= sps->i_mb_width;
		s->mb_height 		= sps->i_mb_height;

		s->frame_type 		= ctx->sh.i_type == SLICE_TYPE_I ? 0:1;	/*全部用Ｉ帧.*/
		s->first_mby 		= 0;
		s->last_mby 		= s->mb_height - 1;
		s->frame_width 		= ctx->p.width;
		s->frame_height 	= ctx->p.height;
		s->qp			= ctx->sh.i_type == SLICE_TYPE_I ? ctx->p.i_qp : ctx->p.p_qp;  /* 使用CBR码率控制.*/
		s->base_qp		= s->qp;
		s->max_qp		= ctx->p.h264_max_qp;
		s->min_qp		= ctx->p.h264_min_qp;
	}

	/*2. motion cfg [25]*/
	{
	}

	/*3. quant cfg [4]*/
	{
		s->dct8x8_en = 0;
		/*TODO: 既然是常量，没必要每次都更改，放到初始化更合适?*/
		memset(s->scaling_list, 0x10, sizeof(s->scaling_list));
		memcpy(s->scaling_list8[0], cqm_8iy_tab, sizeof(uint8_t) * 64);
		memcpy(s->scaling_list8[1], cqm_8py_tab, sizeof(uint8_t) * 64);

		/*TODO: deadzone 是如何得到的?*/
		//s->deadzone[0] = ;
		//s->deadzone[1] = ;
		//s->deadzone[2] = ;
		//s->deadzone[3] = ;
		//s->deadzone[4] = ;
		//s->deadzone[5] = ;
		//s->deadzone[6] = ;
		//s->deadzone[7] = ;
		//s->deadzone[8] = ;
	}
	/*4.loop filter cfg [4] */
	{
		s->deblock 		= ctx->p.deblock;
		s->rotate 		= 0;
		s->alpha_c0_offset 	= 0;
		s->beta_offset		= 0;
	}

	/*11. skin judge cfg[14]*/
	{
		/*TODO.*/
		s->skin_dt_en     = 0; //sipe->skin_dt_en;
		s->skin_lvl       = 0; //sipe->skin_lvl;
		s->skin_cnt_thd   = 20; //sipe->skin_cnt_thd;
		s->ncu_mov_en     = 0;
		s->ncu_move_len   = 20;
		s->ncu_move_info  = NULL;
		s->buf_share_en   = 0;
		s->buf_share_size = 0;
		s->frame_idx      = 0;
		//s->is_first_Pframe = 0;

		memcpy(s->skin_pxlu_thd, rc_skin_pxlu_thd, sizeof(s->skin_pxlu_thd));
		memcpy(s->skin_pxlv_thd, rc_skin_pxlv_thd, sizeof(s->skin_pxlv_thd));
		memcpy(s->skin_qp_ofst,  rc_skin_qp_ofst,  sizeof(s->skin_qp_ofst));
		memcpy(s->mult_factor,   rc_mult_factor,   sizeof(s->mult_factor));
		memcpy(s->shift_factor,  rc_shift_factor,  sizeof(s->shift_factor));
		memcpy(s->skin_ofst,     rc_skin_ofst,     sizeof(s->skin_ofst));
	}

	s->state 		= ctx->cb.state;
	/*输出码流.*/
	/*新版硬件使用emc_bs替换了bs*/
	s->raw_format 		= ctx->p.format;
	s->stride[0] 		= s->frame_width;	/*Y SRIDE*/
	s->stride[1]		= s->raw_format == HELIX_420P_MODE ?
				  s->frame_width / 2 : s->frame_width;
	/*VPU输入, CPU输出*/
	for(i = 0; i < ctx->frame->num_planes; i++) {
		s->raw[i] = ctx->frame->fb_addr[i].pa;
		printk("raw.pa %x\n", ctx->frame->fb_addr[i].pa);
		printk("raw.va %x\n", (unsigned int)ctx->frame->fb_addr[i].va);
	}

	/*
	   s->fb[0][0], --> 当前帧输出重构帧y
	   s->fb[0][1], --> 当前帧输出重构帧c，与输入raw数据接近一致
	   s->fb[1][0], --> 当前帧的参考帧y, I帧没有参考帧,P帧参考前一帧.
	   s->fb[1][1], --> 当前帧的参考帧c
	   s->fb[2][0], --> 前两帧, 当前mref开启时使用.
	   s->fb[2][1],

	   I 帧			    I  P  P  P
	   s->fb[0][0] = sw->fb[0][0]  0->1->2
	   s->fb[1][0] = ?                0->1->2
	   s->fb[2][0] = ?                   0->1->2
	   P 帧
	   s->fb[0][0] = sw->fb[1][0]
	   s->fb[1][0] = sw->fb[0][0]
	   s->fb[2][0] = ?
	   P 帧
	   s->fb[0][0] = sw->fb[2][0]
	   s->fb[1][0] = sw->fb[1][0]
	   s->fb[2][0] = sw->fb[0][0]
	 */

	for(i = 0; i < 3; i++) {
		s->fb[i][0] = ctx->fb[bufidx[i]].yaddr_pa;
		s->fb[i][1] = ctx->fb[bufidx[i]].caddr_pa;
#if 0
		s->jh[i][0] = sw->fb[bufidx[i]].yaddr_pa;
		s->jh[i][1] = sw->fb[bufidx[i]].caddr_pa;
#endif
	}

	s->des_va 	= ctx->desc;
	s->des_pa 	= ctx->desc_pa;

	/*VPU输出，CPU输入*/
	//s->emc_bs_va = sw->bs_tmp_buf.va + bs_off;
	/*TODO , replace this to bs in X2000 chips. bs->va or bs->pa*/
	s->emc_bs_pa = ctx->bs_tmp_buf.pa + bs_off;

	/*VPU使用？*/
	s->emc_dblk_va 	= ctx->emc_buf;
	s->emc_recon_va = s->emc_dblk_va + DBLK_SIZE;
	s->emc_mv_va 	= s->emc_recon_va + RECON_SIZE;
	s->emc_se_va 	= s->emc_mv_va + MV_SIZE;
	s->emc_qpt_va 	= s->emc_se_va + SE_SIZE;
	s->emc_rc_va 	= s->emc_qpt_va + QPT_SIZE;
	s->emc_cpx_va 	= s->emc_rc_va + RC_SIZE;
	s->emc_mod_va 	= s->emc_cpx_va + CPX_SIZE;
	s->emc_sad_va 	= s->emc_mod_va + MOD_SIZE;
	s->emc_ncu_va 	= s->emc_sad_va + SAD_SIZE;

	s->emc_dblk_pa 	= ctx->emc_buf_pa;
	s->emc_recon_pa = s->emc_dblk_pa + DBLK_SIZE;
	s->emc_mv_pa 	= s->emc_recon_pa + RECON_SIZE;
	s->emc_se_pa 	= s->emc_mv_pa + MV_SIZE;
	s->emc_qpt_pa 	= s->emc_se_pa + SE_SIZE;
	s->emc_rc_pa 	= s->emc_qpt_pa + QPT_SIZE;
	s->emc_cpx_pa 	= s->emc_rc_pa + RC_SIZE;
	s->emc_mod_pa 	= s->emc_cpx_pa + CPX_SIZE;
	s->emc_sad_pa 	= s->emc_mod_pa + MOD_SIZE;
	s->emc_ncu_pa 	= s->emc_sad_pa + SAD_SIZE;


	H264E_SliceInit(s);
	H264E_DumpInfo(s);

	/*TODO: 当使用非一致性缓存时，需要刷cache, 未来是要改成非一致性缓存的.*/
	//dma_cache_wback_inv(ctx->desc, ctx->vdma_chain_len);

	return ret;
}

static int h264e_encode_slice(struct h264e_ctx *ctx, int force_idr, int *keyframe)
{
	int idr = 0;
	int i_nal_type = 0;
	int i_nal_ref_idc = 0;
	bs_t bs;
	int ret = 0;
	int tmp_align;

	/*decide I/P Frame*/
	if(!(ctx->i_frame % ctx->p.gop_size) || force_idr) {
		ctx->i_frame = 0;
		idr = 1;
	}

	/* only I/P support.*/
	if(idr) {
		i_nal_type = NAL_SLICE_IDR;
		i_nal_ref_idc = NAL_PRIORITY_HIGHEST;
		ctx->sh.i_type = SLICE_TYPE_I;
		*keyframe = 1;
	} else {
		i_nal_type = NAL_SLICE;
		i_nal_ref_idc = NAL_PRIORITY_HIGH;
		ctx->sh.i_type = SLICE_TYPE_P;
		//ctx->sh.i_type = SLICE_TYPE_I;
		*keyframe = 0;
	}

	/* TODO: replace bs_tmp_buf.va to bs.va*/
	bs_init(&bs, ctx->bs_tmp_buf.va, ctx->bs_tmp_buf.size);

	ret = h264e_slice_init(ctx, i_nal_type);
	if(ret < 0)
		return ret;

	h264e_slice_header_write(&bs, &ctx->sh, i_nal_ref_idc);

	if(ctx->p.i_cabac) {
		bs_align_1(&bs);
		h264_cabac_context_init(&ctx->cb, ctx->sh.i_type, ctx->sh.i_qp_delta, ctx->sh.i_cabac_init_idc);
	}

	/*由于VPU可能不支持动态插入0x03的处理，暂时输出到tmp_bs_buf后插入0x03*/
	tmp_align = 256;
	/*TODO , 假设了slice_header小于256 bytes.*/
	h264e_fill_slice_info(ctx, tmp_align);
	/*fill slice info*/

	ret = ingenic_vpu_start(ctx->priv);
	if(ret < 0) {
		return ret;
	}

	/*编码后处理.*/
	if(bs.i_left == 32) {
		memcpy(bs.p, ctx->bs_tmp_buf.va + tmp_align, ctx->encoded_bs_len);
		bs.p += ctx->encoded_bs_len;
	} else {

		int len = ctx->encoded_bs_len;
		volatile unsigned char *p = ctx->bs_tmp_buf.va + tmp_align;

		while(len --) {
			bs_write(&bs, 8, *p++);
		}
		bs_rbsp_trailing(&bs);
	}

	/* 修正 encoded_bs_len， 包含0x03的插入.*/
	ctx->encoded_bs_len = encapsulate_nal(&bs, 0, ctx->bs->va, i_nal_type, i_nal_ref_idc);

	printk("-----h264e_ctx->r_bs_len %d\n", ctx->r_bs_len);
	printk("-----h264e_ctx->encoded_bs_len: %d\n", ctx->encoded_bs_len);
	/*done!*/

	ctx->i_frame++;
	ctx->frameindex++;

	return ret;
}

/* add 0xff nal .*/
static int h264e_filler_nal(int size, char *p)
{
        if (size < 6)
                return -EINVAL;

        p[0] = 0x00;
        p[1] = 0x00;
        p[2] = 0x00;
        p[3] = 0x01;
        p[4] = 0x0c;
        memset(p + 5, 0xff, size - 6);
        /* Add rbsp stop bit and trailing at the end */
        p[size - 1] = 0x80;

        return 0;
}

static int h264e_padding(int size, char *p)
{
	int ret = 0;

	if(size == 0)
		return 0;

        ret = h264e_filler_nal(size, p);
	if(ret < 0)
		return ret;

        return size;
}

int h264e_encode_headers(struct h264e_ctx *ctx, struct ingenic_vcodec_mem *bs)
{
	int ret = 0;
	unsigned int align = 0;

	if(ctx->encoded_bs_len != 0) {
		pr_info("set encoded_bs_len to 0 before encode!\n");
	}

	printk("----%s, %d, ctx->bs_header_size: %d\n", __func__, __LINE__, ctx->bs_header_size);
	memcpy(bs->va, ctx->bs_header, ctx->bs_header_size);

	/*align to 256*/
	align = (ctx->bs_header_size + 255) / 256 * 256;


	ret = h264e_padding(align - ctx->bs_header_size, bs->va + ctx->bs_header_size);

	ctx->encoded_bs_len = align;

	return ret;
}

/*
	input:
	@h264e_ctx
	@vframe_buffer
	output:
	@bs_buffer
	@keyframe
*/
int h264e_encode(struct h264e_ctx *ctx, struct video_frame_buffer *frame,
		struct ingenic_vcodec_mem *bs, int *keyframe)
{
	int ret = 0;
	ctx->frame = frame;
	ctx->bs = bs;

	ret = h264e_encode_slice(ctx, 0, keyframe);

	/* output bistream */

	return ret;
}



/* start streaming.*/
int h264e_alloc_workbuf(struct h264e_ctx *ctx)
{
	int allocate_fb = 0;
	int ret = 0;
	int i = 0;

	pr_info("----%s, %d\n", __func__, __LINE__);

	ctx->bs_header = kzalloc(MAX_BS_HEADER_SIZE, GFP_KERNEL);
	if(!ctx->bs_header) {
		pr_err("Failed to alloc memory for bs_header\n");
		return -ENOMEM;
	}
	ctx->bs_header_size = 0;


	/* TODO, 使用非一致性申请方式.*/
	ctx->desc = dma_alloc_coherent(NULL, ctx->vdma_chain_len, &ctx->desc_pa, GFP_KERNEL);
	if(!ctx->desc) {
		pr_err("Failed to alloc vpu desc buffer!\n");
		ret = -ENOMEM;
		goto err_desc;
	}

	/* TODO: 使用非一致性内存，这些内存分配完只给VPU使用，所以也无所谓.*/
	for(i = 0; i < 3; i++) {
		ctx->fb[i].yaddr = dma_alloc_coherent(NULL, ctx->framesize, &ctx->fb[i].yaddr_pa, GFP_KERNEL);
		ctx->fb[i].caddr = dma_alloc_coherent(NULL, ctx->framesize/2, &ctx->fb[i].caddr_pa, GFP_KERNEL);

		allocate_fb = i;
		if(!ctx->fb[i].yaddr || !ctx->fb[i].caddr) {
			ret = -ENOMEM;
			goto err_fb;
		}
		printk("-------------ctx->fb[%d].yaddr: 0x%08x\n", i, (unsigned int)ctx->fb[i].yaddr);
		printk("-------------ctx->fb[%d].caddr: 0x%08x\n", i, (unsigned int)ctx->fb[i].caddr);
	}

	/*TODO: */
	ctx->emc_buf = dma_alloc_coherent(NULL, EMC_SIZE, &ctx->emc_buf_pa, GFP_KERNEL);
	if(!ctx->emc_buf) {
		pr_err("Failed to alloc vpu emc_buf!\n");
		ret = -ENOMEM;
		goto err_emc_buf;
	}

	ctx->bs_tmp_buf.size = ctx->framesize * 2;
	ctx->bs_tmp_buf.va = dma_alloc_coherent(NULL, ctx->bs_tmp_buf.size, &ctx->bs_tmp_buf.pa, GFP_KERNEL);
	if(!ctx->bs_tmp_buf.va) {
		ret = -ENOMEM;
		goto err_bs_tmp;
	}

	return ret;

err_bs_tmp:
	dma_free_coherent(NULL, EMC_SIZE, ctx->emc_buf, ctx->emc_buf_pa);
err_emc_buf:
err_fb:
	i = allocate_fb;
	while(i--) {
		if(ctx->fb[i].yaddr) {
			dma_free_coherent(NULL, ctx->framesize, ctx->fb[i].yaddr, ctx->fb[i].yaddr_pa);
		}
		if(ctx->fb[i].caddr) {
			dma_free_coherent(NULL, ctx->framesize/2, ctx->fb[i].caddr, ctx->fb[i].caddr_pa);
		}
	}

err_desc:
	kfree(ctx->bs_header);
	ctx->bs_header = NULL;

	return ret;
}

/* stop streaming.*/
int h264e_free_workbuf(struct h264e_ctx *ctx)
{
	int i;

	kfree(ctx->bs_header);
	dma_free_coherent(NULL, ctx->vdma_chain_len, ctx->desc, ctx->desc_pa);
	dma_free_coherent(NULL, EMC_SIZE, ctx->emc_buf, ctx->emc_buf_pa);

	for(i = 0; i < 3; i++) {
		dma_free_coherent(NULL, ctx->framesize, ctx->fb[i].yaddr, ctx->fb[i].yaddr_pa);
		dma_free_coherent(NULL, ctx->framesize/2, ctx->fb[i].caddr, ctx->fb[i].caddr_pa);
	}

	dma_free_coherent(NULL, ctx->bs_tmp_buf.size, ctx->bs_tmp_buf.va, ctx->bs_tmp_buf.pa);

	return 0;
}

/*ctx init*/
/* TODO: 删除默认的结构体，想到一种方法可以修改默认的sliceinfo，
方便调试, 比如导出sys节点?, 配合合肥已经调好的参数??*/
static void h264e_init_slice_info(H264E_SliceInfo_t *s)
{
	memcpy(s, &default_H264E_Sliceinfo, sizeof(default_H264E_Sliceinfo));
}

int h264e_encoder_init(struct h264e_ctx *ctx)
{
	struct h264e_params *p = NULL;
	H264E_SliceInfo_t *s = NULL;

	/*TODO: 尽量减小h264e_ctx直接占用的内存大小，将占用大内存的数据结构使用动态分配.*/
	s = kzalloc(sizeof(H264E_SliceInfo_t), GFP_KERNEL);
	if(!s) {
		return -ENOMEM;
	}

	p = &ctx->p;
	ctx->s = s;

	/* default params. */
	ctx->vdma_chain_len 	= 40960 + 256;
	ctx->frameindex		= 0;
	ctx->i_frame		= 0;

	p->height		= 240;
	p->width		= 160;
	p->max_ref_pic		= 1;
	p->gop_size		= 16;
	p->i_idr_pic_id		= 0;
	p->i_global_qp 		= 26;
	p->h264_profile		= V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
	p->h264_level		= V4L2_MPEG_VIDEO_H264_LEVEL_1_0;
	p->i_cabac		= 1;
	p->interlaced		= 0;
	p->h264_8x8_transform	= 0;
	p->h264_hdr_mode	= V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_1ST_FRAME;
	p->h264_min_qp		= 0;
	p->h264_max_qp		= 51;
	p->i_qp			= 0;
	p->p_qp			= 26;
	p->deblock		= 0;

	h264_cabac_init();
	/*TODO: default sliceinfo*/

	h264e_init_slice_info(s);
	return 0;
}

int h264e_encoder_deinit(struct h264e_ctx *ctx)
{
	if(!ctx)
		return 0;

	if(ctx->s)
		kfree(ctx->s);
	return 0;
}

void h264e_set_priv(struct h264e_ctx *ctx, void *data)
{
	ctx->priv = data;
}
