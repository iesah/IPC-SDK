/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  HW ECC-BCH support functions
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#ifndef __SOC_BCH_H__
#define __SOC_BCH_H__

typedef enum {
	BCH_RET_OK = 0,
	BCH_RET_UNCORRECTABLE = -1,
	BCH_RET_UNEXPECTED = -2,

	/* TODO: remvoe when bch driver will be completely done */
	BCH_RET_UNSUPPORTED = -3
} bch_req_ret_t;

typedef enum {
	BCH_REQ_DECODE = 0,
	BCH_REQ_ENCODE,
	BCH_REQ_CORRECT,
	BCH_REQ_DECODE_CORRECT
} request_type_t;

typedef enum {
	BCH_DATA_WIDTH_8 = 0,
	BCH_DATA_WIDTH_16,
	BCH_DATA_WIDTH_32
} bch_data_width_t;

struct bch_request;
typedef void (*bch_complete_t)(struct bch_request *);

struct bch_request {
	/* public members */

	request_type_t type;     /* (in) decode or encode */
	int ecc_level;           /* (in) must 4 * n (n = 1, 2, 3 ... 16) */
	int parity_size;         /* 14 * ecc_level / 8 */

	const void *raw_data;    /* (in) must word aligned */
	u32 blksz;               /* (in) according to raw_data_width MAX=1900 */

	void *ecc_data;          /* (in) must word aligned */

	u32 *errrept_data;       /* (in/out) must word aligned */
	u32 errrept_word_cnt;    /* (in/out) errrept_data counter */

	u32 cnt_ecc_errors;      /* (out) ecc data errors bits counter */
	bch_req_ret_t ret_val;   /* (out) request return value */

	struct device *dev;      /* (in) the device that request belong to */

	bch_complete_t complete; /* (in) called when request done */

	/* private members */

	struct list_head node;
};

typedef struct bch_request bch_request_t;

extern int bch_request_submit(bch_request_t *req);

static inline int bch_ecc_bits_to_bytes(int ecc_bits)
{
	return (ecc_bits * 14 + 7) / 8;
}

#define MAX_ERRREPT_DATA_SIZE   (64 * sizeof(u32))
#define MAX_ECC_DATA_SIZE       (64 * 14 / 8)

#endif
