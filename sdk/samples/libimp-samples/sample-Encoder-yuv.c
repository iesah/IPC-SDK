#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "sample-common.h"

extern void *video_vbm_malloc(int size, int align);
extern void video_vbm_free(void *vaddr);
extern intptr_t video_vbm_virt_to_phys(intptr_t vaddr);
extern intptr_t video_vbm_phys_to_virt(intptr_t paddr);

int main(int argc, char *argv[])
{
	int ret = 0;
	int i = 0, frmSize = 0;
	int width = 1920, height = 1080;
	int encNum = 200;
	int out = -1;
	char path[32];
	FILE *inFile = NULL;
	uint8_t *src_buf = NULL;
	void *h = NULL;
	IMPFrameInfo frame;
	streamInfo stream;
	encInfo info;

	memset(&info, 0, sizeof(encInfo));

	info.type = IMP_ENC_TYPE_HEVC;
	info.mode = IMP_ENC_RC_MODE_CAPPED_QUALITY;
	//info.type = IMP_ENC_TYPE_JPEG;
	//info.mode = IMP_ENC_RC_MODE_FIXQP;
	info.frameRate = 25;
	info.gopLength = 25;
	info.targetBitrate = 4096;
	info.maxBitrate = 4096 * 4 / 3;
	info.initQp = 25;
	info.minQp = 15;
	info.maxQp = 48;
	info.maxPictureSize = info.maxBitrate;

	ret = IMP_System_Init();
	if (ret < 0){
		printf("IMP_System_Init failed\n");
		return -1;
	}

	inFile = fopen("1920x1080.nv12", "rb");
	if (inFile == NULL) {
		printf("fopen src file:%s failed\n", "1920x1080.nv12");
		return -1;
	}
	if (info.type == IMP_ENC_TYPE_HEVC)
		strcpy(path, "out.h265");
	else if (info.type == IMP_ENC_TYPE_AVC)
		strcpy(path, "out.h264");

	if (info.type != IMP_ENC_TYPE_JPEG) {
		out = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
		if (out < 0) {
			printf("open out file:%s failed\n", path);
			return -1;
		}
	}
	ret = alcodec_encyuv_init(&h, width, height, (void *)&info);
	if ((ret < 0) || (h == NULL)) {
		printf("alcodec init failed\n");
		return -1;
	}

	frmSize = width * height * 3 / 2;
	src_buf = (uint8_t*)video_vbm_malloc(frmSize, 256);
	if(src_buf == NULL) {
		printf("video_vbm_malloc src_buf failed\n");
		return -1;
	}

	frame.width = width;
	frame.height = height;
	frame.size = frmSize;
	frame.phyAddr = (uint32_t)video_vbm_virt_to_phys((intptr_t)src_buf);
	frame.virAddr = (uint32_t)src_buf;
	stream.streamAddr = (uint8_t *)malloc(frmSize);
	if(stream.streamAddr == NULL) {
		printf("steamAddr malloc failed\n");
		return -1;
	}

	for (i = 0; i < encNum; i++) {
		if (info.type == IMP_ENC_TYPE_JPEG) {
			sprintf(path, "out_%d.jpeg", i);
			out = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
			if (out < 0) {
				printf("open out file:%s failed\n", path);
				return -1;
			}
		}
		if(fread(src_buf, frmSize, 1, inFile) != frmSize) {
			fseek(inFile, 0, SEEK_SET);
			fread(src_buf, 1, frmSize, inFile);
		}

		ret = alcodec_encyuv_encode(h, frame, &stream);
		if (ret < 0) {
			printf("alcodec encode failed\n");
			return -1;
		}
		printf("\r%d encode success", i);
		fflush(stdout);
		if (stream.streamLen != write(out, stream.streamAddr, stream.streamLen)) {
			printf("stream write failed\n");
			return -1;
		}
		if (info.type == IMP_ENC_TYPE_JPEG)
			close(out);
	}
	puts("");

	video_vbm_free(src_buf);
	free(stream.streamAddr);
	alcodec_encyuv_deinit(h);
	if (info.type != IMP_ENC_TYPE_JPEG)
		close(out);
	fclose(inFile);
	IMP_System_Exit();

	return 0;
}
