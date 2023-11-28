#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>




#include "wakeup.h"

#define WAKEUP_DEV	"/dev/jz-wakeup"
#define SYSFS_WAKEUP	"/sys/class/jz-wakeup/jz-wakeup/wakeup"

#define INPUT_KEYEVENT_PWR	26


char data_buf[4096];
int main(int argc, char *argv[])
{
	int i;
	int fd;
	int dev_fd;
	int sys_fd;

	int ret;
	if(argc == 1) {
		printf("usage: %s [resource_file]\n", argv[0]);
		return -1;
	}

	fd = open(argv[1], O_RDWR);
	if(fd < 0) {
		perror("open resource");
		return -1;
	}
	printf("open file[%s], ok!\n", argv[1]);

	dev_fd = open(WAKEUP_DEV, O_RDWR);
	if(dev_fd < 0) {
		perror("open wakeup");
		close(fd);
		return -1;
	}
	printf("open file[%s] ok!\n", WAKEUP_DEV);
	sys_fd = open(SYSFS_WAKEUP, O_RDWR);
	if(sys_fd < 0) {
		perror("open sysfs");
		close(dev_fd);
		close(fd);
		return -1;
	}
	printf("open file[%s] ok!\n", SYSFS_WAKEUP);

	/* read from resource , set to device */
	while(1) {
		int read_cnt;
		int write_cnt;
		read_cnt = read(fd, data_buf, 4096);
		if(read_cnt == 0) {
			printf("read done!!!!\n");
			break;
		}
		write_cnt = write(dev_fd, data_buf, read_cnt);
	}

	char temp = '1';
	ret = write(sys_fd, &temp, 1);
	if(ret < 0) {
		perror("write sysfd");
	}

	while(1) {
		int a;
		char buf[20];
		printf("#### begin read !!!!!\n");
		lseek(sys_fd, 0L, SEEK_SET);
		ret = read(sys_fd, buf, 10);
		buf[ret] = 0;
		printf("################ret:%d, %s\n", ret, buf);
		if(ret <= 0) {
			perror("read sysfs");
		} else {
			system("input keyevent 26");
			printf("#########read ok!, wakeup ok!\n");
		}
	}

	close(sys_fd);
	close(dev_fd);
	close(fd);
	printf("exit.\n");
	return 0;
}


