/*
 * Author: qipengzhen <aric.pzqi@ingenic.com>
 *
 * wav recorder for m200 dmic driver.
 */

#include "headers.h"



#define ARRAY_SIZE(x) (sizeof((x))/sizeof((x)[0]))

/* my own header files */
#include "wav.h"


#define FILE_TYPE_WAV	0


#define FMT8BITS    AFMT_U8
#define FMT16BITS   AFMT_S16_LE
#define FMT24BITS   AFMT_S24_LE

#define DEFAULT_SND_DEV "/dev/jz-dmic"

#define DEFAULT_SND_SPD 16000
#define DEFAULT_SND_CHN 1


#define DEFAULT_BUFF_SIZE 4096//    (4096 * 3)

/* unit: s */
#define DEFAULT_DURATION    10


/* now sndkit only support: U8, S16_LE, S24_3LE, other formats hasnot be support */
const char *supported_fmts[] = {
	"S8",
	"U8",
	"S16_LE",
	"S16_BE",
	"U16_LE",
	"U16_BE",
	"S24_3LE",
	"S24_3BE",
	"U24_3LE",
	"U24_3BE",
};


static int support_rates[] = {
	8000,
	16000,
	48000, /*test only*/
};

static int rate_is_supported(int rate) {
	unsigned int i = 0;

	for (i = 0; i < ARRAY_SIZE(support_rates); i++)
		if (support_rates[i] == rate)
			return 1;

	return 0;
}

struct record_params {
	int dev_fd;
	int record_fd;
	int buffer_size;
	int duration;
	int format;
	int channels;
	int speed;
	int file_type;
	int record_set_dmic;
	int set_dmic_num;
};


static struct option long_options[] = {
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'V' },

	{ "device", 1, 0, 'D' },
	{ "channels", 1, 0, 'c' },
	{ "format", 1, 0, 'f' },
	{ "rate", 1, 0, 'r'},
	{ "frequency", 1, 0, 'F' },
	{ "duration", 1, 0, 'd' },
	{ "buffer-size", 1, 0, 'B' },

	{ "record", 0, 0, 'R' },
	{ "record-file", 1, 0, 'w' },

	{0, 0, 0, 0}
};

static char optstring[] = "+hVWTD:m:t:c:f:r:F:d:B:vPp:Ss:Rw:o:";

static void usage(void) {
	unsigned int i = 0;

	printf("Usage: dmic_test [OPTION]... [FILE]...\n\n");

	printf("-h --help\t\tprint this message\n");

	printf("-c --channels=#\t\tchannels\n");
	printf("-f --format=FORMAT\tsample format (case insensitive)\n");
	printf("-r --rate=#\t\tsample rate(fs)\n");
	printf("-F --frequency=#\tsine wave frequency(fa)\n");
	printf("-d --duration=#\t\trecord duration default is 10s, or analysis size\n");
	printf("-B --buffer-size=#\tbuffer size\n");

	printf("-R --record\t\tenable record\n");
	printf("-w --record-file=FILE\tfile to save the recorded data\n");

	printf("\n");
	printf("Supported speeds are:");
	for (i = 0; i < ARRAY_SIZE(support_rates); i++) {
		printf("%d", support_rates[i]);

		if (i != (ARRAY_SIZE(support_rates) - 1))
			printf(", ");
	}

	printf("\n\n");

	printf("Recognized sample formats are:");
	printf(" U8, S16_LE, S24_3LE");
	printf("\n");
}

static int open_device(char *file, int oflag, int *is_reg_file) {
	struct stat stats;
	int fd = -1;

	*is_reg_file = 0;

	if ((lstat(file, &stats) == 0) &&
	    S_ISREG(stats.st_mode)) {
		printf("NOTE: device file is a regular file.\n");
		*is_reg_file = 1;
	}

	fd = open(file, oflag);
	if (fd < 0) {
		fprintf(stderr, "%s: cannot open file %s: %s\n",
			__func__,
			file,
			strerror(errno));

	}

	return fd;
}

static int open_file(char *file, int oflag) {
	int fd = -1;

	fd = open(file, oflag);
	if (fd < 0) {
		fprintf(stderr, "%s: cannot open file %s: %s\n",
			__func__,
			file,
			strerror(errno));
	}
	return fd;
}


static struct record_params record_params;
static int record(struct record_params *params) {
	int dev_fd = params->dev_fd;
	int record_fd = params->record_fd;
	int buffer_size = params->buffer_size;
	int duration = params->duration;
	int format = params->format;
	int channels = params->channels;
	int speed = params->speed;
	int file_type = params->file_type;
	int record_set_dmic = params->record_set_dmic;
	int set_dmic_num = params->set_dmic_num;

	int ret = 0;
	int n_read = 0;
	int n_to_write = 0;
	int n_to_write2 = 0;
	int n;
	int pos;
	unsigned char *buff;
	unsigned char *buff1;
	int format_bits = 16;
	int record_len = 0;

	buff = malloc(buffer_size);

	if (buff == NULL) {
		fprintf(stderr, "REC: alloc buffer failed, size = %d\n", buffer_size);
		return -1;
	}

	format_bits = 16;

	printf("REC: format_bits = %d, channels = %d, speed = %d, duration = %d\n", format_bits, channels, speed, duration);

	record_len = format_bits * channels * speed / 8 * duration;
	buff1 = malloc(record_len);

	printf("record len = %d\n", record_len);

	while (record_len > 0) {
		n_read = read(dev_fd, buff,
				(buffer_size > record_len)
				? record_len
				: buffer_size);

		if (n_read < 0) {
			perror("read sound device(file) failed");
			ret = -1;
			goto out;
		}

		if (n_read == 0) {
			printf("REC: zero bytes! why?\n");
			continue;
		}

		if ((file_type == FILE_TYPE_WAV) &&
				(format_bits == 24)) {
			wav_32bit_to_24bit(buff, n_read);
			n_read = n_read / 4 * 3;
		}

		if (n_read < record_len)
			n_to_write = n_read;
		else
			n_to_write = record_len;

		n_to_write2 = n_to_write;

		pos = 0;
		if ((buff1 = memcpy(buff1, buff + pos, n_to_write)) < 0) {
			perror("REC: write sound data file failed");
			ret = -1;
			goto out;
		}
		buff1 += n_to_write;

		record_len -= n_to_write2;
	}


	printf("start write\n");
	record_len = format_bits * channels * speed / 8 * duration;
	buff1 -= record_len;
	pos = 0;
	while (record_len > 0) {
		if ((n = write(record_fd, buff1 + pos, record_len)) < 0) {
			perror("REC: write sound data file failed");
			ret = -1;
			goto out;
		}

		record_len -= n;
		pos += n;
	}
	printf("writed\n");





 out:
	free(buff);
	free(buff1);
	printf("REC: end.\n");
	return ret;
}

static void *record_thread(void *arg) {
	record((struct record_params *)arg);

	return NULL;
}


int main(int argc, char *argv[])
{
	char dmic_dev[128];
	int dev_fd = -1;
	int file_type = FILE_TYPE_WAV;

	int channels = DEFAULT_SND_CHN;
	int channels_user = 0;
	int format = FMT16BITS;
	int format_bits = 16;
	int format_user = 0;
	int speed = DEFAULT_SND_SPD;
	int speed_user = 0;

	unsigned int duration = DEFAULT_DURATION;
	unsigned int buffer_size = DEFAULT_BUFF_SIZE;

	int record_enable = 0;
	char record_file[128];
	int record_fd = -1;

	pthread_t record_pid;
	int recording = 0xdead;

	int vol = 0;
	int sine_freq = 1000;
	int ret = 0;
	int is_dev_reg_file;
	int convert_raw_to_wav = 0;
	int is_strip_wav = 0;
	int record_set_dmic = 0;
	int set_dmic_num = 0;

	char c;

	strcpy(dmic_dev, DEFAULT_SND_DEV);
	memset(record_file, 0, sizeof(record_file));

	while (1) {
		c = getopt_long(argc, argv, optstring, long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage();
			return 0;

		case 'c':
			channels = atoi(optarg);
			channels_user = 1;
			break;

		case 'f':
			format_user = 1;
			break;

		case 'r':
			speed = atoi(optarg);
			if (!rate_is_supported(speed)) {
				printf("unrecognized sample rate: %d\n", speed);
				return 127;
			}
			speed_user = 1;
			break;

		case 'F':
			sine_freq = atoi(optarg);
			if (sine_freq == 0) {
				printf("sine wave frequency can not be 0.\n");
				return 127;
			}
			break;

		case 'd':
			duration = atoi(optarg);
			if (duration == 0) {
				printf("duration is 0, what do you want to do?\n");
				return 127;
			}
			break;

		case 'B':
			buffer_size = atoi(optarg);
			break;

		case 'v':
			break;


		case 'R':
			record_enable = 1;
			break;

		case 'w':
			if (strlen(optarg) > 127) {
				printf("file name is too long: %s\n", optarg);
				return 127;
			}
			strncpy(record_file, optarg, strlen(optarg));
			break;

		default:
			/* do nothing */
			printf("Unknown option '%c'\n", c);
			usage();
			return 127;
		}
	}

	if (dmic_dev[0] == '\0') {
		fprintf(stderr, "device is not specified!\n");
		return 127;
	}

	if (record_enable  && (record_file[0] == '\0')) {
		fprintf(stderr, "record file is not specified!\n");
		return 127;
	}

	/* open device */
	if (record_enable) {
		printf("Record\n");
		dev_fd = open_device(dmic_dev, O_RDONLY, &is_dev_reg_file);


		/*SET SAMPLE RATE:*/

#define DMIC_SET_CHANNEL    0x100
#define DMIC_ENABLE         0x101
#define DMIC_DISABLE        0x102
#define DMIC_SET_SAMPLERATE 0x103
#define DMIC_GET_ROUTE      0x104
#define DMIC_SET_ROUTE      0x105
#define DMIC_SET_DEVICE     0x106


		int ret = 0;
		ret = ioctl(dev_fd, DMIC_SET_SAMPLERATE, &speed);
		if(ret < 0) {
			printf("[ioctl] dmic set sample rate!\n");
		}

	}

	if ( (record_enable) && (dev_fd < 0)) {
		fprintf(stderr, "open device failed.\n");
		return -1;
	}

	format_bits = 16;

	printf("%d", format_bits);

	printf(" channels = %d, speed = %d\n",
	       channels, speed);

	/* open record file */
	if (record_enable) {
		record_fd = open_file(record_file, O_WRONLY | O_CREAT);
		if (record_fd < 0)
			return -1;
	}

	/* prepare wav header */
	if (record_enable && (file_type == FILE_TYPE_WAV)) {
		int hi;
		int hn;
		unsigned char wav_head[WAV_HEAD_SIZE];
		unsigned char *wav_head2 = wav_head;

		int record_len = format_bits * channels * speed / 8 * duration;

		init_wav_header(wav_head, record_len, channels, speed, format_bits);
		hi = WAV_HEAD_SIZE;
		while(hi > 0) {
			if ((hn = write(record_fd, wav_head2, hi)) < 0) {
				perror("write sound data file failed");
				return -1;
			}
			hi -= hn;
			wav_head2 += hn;
		}
	}

	/*printf("format = %d,channels = %d,speed = %d\n",format,channels,speed);*/
	/*set sound format */

	if (record_enable) {

		record_params.dev_fd = dev_fd;
		record_params.record_fd = record_fd;
		record_params.buffer_size = buffer_size;
		record_params.duration = duration;
		record_params.format = format;
		record_params.channels = channels;
		record_params.speed = speed;
		record_params.file_type = file_type;
		record_params.record_set_dmic = record_set_dmic;
		record_params.set_dmic_num = set_dmic_num;

		recording = pthread_create(&record_pid, NULL, record_thread, &record_params);
		if (recording != 0) {
			perror("REC: start record thread failed.");
		}
	}

	if (recording == 0) {
		pthread_join(record_pid, NULL);
	}

	if (record_enable) {
		ret = close(dev_fd);
		if (ret)
			printf("rdev errno: %d\n", ret);
		else
			printf("rdev success: %d\n", ret);
	}

	return ret;
}
