#ifndef __WAV_H__
#define __WAV_H__

struct chunk_head {
	char type[4];
	unsigned int length;
};

struct fmt_chunk_head {
	struct chunk_head chunk_head;
	unsigned short fmt_tag;
	unsigned short channels;
	unsigned int sample_rate;
	unsigned int bytes_per_second; /* sample rate * block align */
	unsigned short block_align; /* channels * bits/sample / 8 */
	unsigned short bits_per_sample;
};

#define WAV_HEAD_SIZE		(3 * 4 + sizeof(struct fmt_chunk_head) + 2 * 4)
#define MAX_WAV_HEAD_SIZE	256

extern void init_wav_header(unsigned char *data, int data_len,
		     unsigned int channels, unsigned int sample_rate, unsigned int bits);

extern int parse_wav_header(unsigned char *data, int *head_len,
		     unsigned int *channels, unsigned int *sample_rate, unsigned int *bits);


extern void wav_expand_24bit_to_32bit(unsigned char *data, int len);
extern void wav_32bit_to_24bit(unsigned char *data, int len);

#endif /* __WAV_H__ */
