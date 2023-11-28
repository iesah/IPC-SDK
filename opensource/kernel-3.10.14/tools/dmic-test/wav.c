#include "wav.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void init_wav_header(unsigned char *data, int data_len,
		       unsigned int channels, unsigned int sample_rate, unsigned int bits) {
	unsigned int *length;
	struct fmt_chunk_head *fmt_head;

	/* RIFF header */
	data[0] = 'R';
	data[1] = 'I';
	data[2] = 'F';
	data[3] = 'F';

	length = (unsigned int *)(data + 4);
	*length = data_len +
		4 +	      /* "WAVE" */
		sizeof(struct fmt_chunk_head)
		+ 8;	      /* "data" + length(4byte) */

	data[8] = 'W';
	data[9] = 'A';
	data[10] = 'V';
	data[11] = 'E';

	data += 12;

	/* fmt chunk */
	data[0] = 'f';
	data[1] = 'm';
	data[2] = 't';
	data[3] = ' ';

	fmt_head = (struct fmt_chunk_head *)data;
	fmt_head->chunk_head.length = sizeof(struct fmt_chunk_head) - 8;
	fmt_head->fmt_tag = 1;
	fmt_head->channels = channels;
	fmt_head->sample_rate = sample_rate;
	fmt_head->block_align = channels * bits / 8;
	fmt_head->bytes_per_second = sample_rate * fmt_head->block_align;
	fmt_head->bits_per_sample = bits;

	data += sizeof(struct fmt_chunk_head);

	/* data chunk */
	data[0] = 'd';
	data[1] = 'a';
	data[2] = 't';
	data[3] = 'a';

	length = (unsigned int *)(data + 4);
	*length = data_len;
}

int parse_wav_header(unsigned char *data, int *head_len,
		     unsigned int *channels, unsigned int *sample_rate, unsigned int *bits) {
	unsigned int *total_len;
	int *data_len;
	unsigned char *data_orig = data;
	struct fmt_chunk_head *fmt_head;

	/* parse RIFF header */
	if ((data[0] == 'R') &&
	    (data[1] == 'I') &&
	    (data[2] == 'F') &&
	    (data[3] == 'F')) {
		/* do nothing */
	} else
		return 0;

	if ((data[8] == 'W') &&
	    (data[9] == 'A') &&
	    (data[10] == 'V') &&
	    (data[11] == 'E')) {
		/* do nothing */
	} else
		return 0;

	printf("WAV: file is wav file.\n");

	total_len = (unsigned int *)(data + 4);
	printf("WAV: total length = %u\n", *total_len);

	data += 12;

	if ((data[0] == 'J') &&
	    (data[1] == 'U') &&
	    (data[2] == 'N') &&
	    (data[3] == 'K')) {
		unsigned int *junk_len = (unsigned int *)(data + 4);

		data += (*junk_len + 8);
	}

	/* parse fmt chunk */
	if ((data[0] == 'f') &&
	    (data[1] == 'm') &&
	    (data[2] == 't') &&
	    (data[3] == ' ')) {
		printf("WAV: fmt chunk recognized.\n");
	} else {
		printf("WAV: error! fmt chunk expected!\n");
		return 0;
	}

	fmt_head = (struct fmt_chunk_head *)data;
	if (fmt_head->fmt_tag != 1) {
		printf("WAV: sorry, we current do not support uncompress(fmt_tag = %u)\n", fmt_head->fmt_tag);
		return 0;
	}

	*channels = fmt_head->channels;
	*sample_rate = fmt_head->sample_rate;
	*bits = fmt_head->bits_per_sample;

	printf("WAV: channels = %u, sample_rate = %u, bits = %u\n",
	       *channels, *sample_rate, *bits);

	data += fmt_head->chunk_head.length + 8;

	//printf("data: %02x %02x %02x %02x\n",
	//       data[0], data[1], data[2], data[3]);

	if ((data[0] == 'f') &&
	    (data[1] == 'a') &&
	    (data[2] == 'c') &&
	    (data[3] == 't')) {
		data += 12;
	}

	if ((data[0] == 'd') &&
	    (data[1] == 'a') &&
	    (data[2] == 't') &&
	    (data[3] == 'a')) {
		data_len = (int *)(data + 4);
		data += 8;
	} else {
		printf("WAV: error! data chunk expected!\n");
		return 0;
	}

	*head_len = data - data_orig;

	printf("WAV: data_len = %d\n", *data_len);

	return *data_len;
}

void wav_expand_24bit_to_32bit(unsigned char *data, int len) {
	unsigned char tmp_sample[9];
	unsigned char *dst;
	unsigned char *src;
	int i = 0;

	if ((len % 3) != 0) {
		printf("===>len(%d) is not multiple of 3", len);

		len = len - (len % 3);

		printf("===>round to %d\n", len);
	}

	dst = data + len / 3 * 4 - 4;
	src = data + len - 3;

	for (i = 0; i < len / 3 - 3; i++) {
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = 0x0;

		src -= 3;
		dst -= 4;
	}

	memcpy(tmp_sample, data, 9);

	data[0] = tmp_sample[0];
	data[1] = tmp_sample[1];
	data[2] = tmp_sample[2];
	data[3] = 0x0;

	data[4] = tmp_sample[3];
	data[5] = tmp_sample[4];
	data[6] = tmp_sample[5];
	data[7] = 0x0;

	data[8] = tmp_sample[6];
	data[9] = tmp_sample[7];
	data[10] = tmp_sample[8];
	data[11] = 0x0;

	src -= 6;
	dst -= 8;

	assert(src == data);
	assert(dst == data);
}

void wav_32bit_to_24bit(unsigned char *data, int len) {
	int i = 0;
	unsigned char *dst = data;
	unsigned char *src = data;

	for (i = 0; i < len; i += 4) {
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];

		dst += 3;
		src += 4;
	}
}
