#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define OPENPMON                                        \
	char perf_array[256] = {0};                         \
	int fperf = open("/proc/jz/pmon/perform",O_RDWR);   \
	if(fperf <= 0)                                      \
        printf("open pmon error!\n");                   \

#define CLOSEFS close(fperf)

#define START0 memset(perf_array,0,255); write(fperf,"perf0",5);
#define START1 memset(perf_array,0,255); write(fperf,"perf1",5);
#define START2 memset(perf_array,0,255); write(fperf,"perf2",5);
#define START3 memset(perf_array,0,255); write(fperf,"perf3",5);
#define START4 memset(perf_array,0,255); write(fperf,"perf4",5);
#define START5 memset(perf_array,0,255); write(fperf,"perf5",5);
#define START6 memset(perf_array,0,255); write(fperf,"perf6",5);
#define START7 memset(perf_array,0,255); write(fperf,"perf7",5);
#define START8 memset(perf_array,0,255); write(fperf,"perf8",5);
#define START9 memset(perf_array,0,255); write(fperf,"perf9",5);
#define START10 memset(perf_array,0,255); write(fperf,"perf10",6);
#define START48 memset(perf_array,0,255); write(fperf,"perf48",6);
#define START49 memset(perf_array,0,255); write(fperf,"perf49",6);
#define START50 memset(perf_array,0,255); write(fperf,"perf50",6);
#define START51 memset(perf_array,0,255); write(fperf,"perf51",6);
#define START52 memset(perf_array,0,255); write(fperf,"perf52",6);
#define START53 memset(perf_array,0,255); write(fperf,"perf53",6);
#define START54 memset(perf_array,0,255); write(fperf,"perf54",6);
#define START55 memset(perf_array,0,255); write(fperf,"perf55",6);
#define START56 memset(perf_array,0,255); write(fperf,"perf56",6);
#define START57 memset(perf_array,0,255); write(fperf,"perf57",6);
#define START58 memset(perf_array,0,255); write(fperf,"perf58",6);
#define START59 memset(perf_array,0,255); write(fperf,"perf59",6);
#define STOP                                    \
	lseek(fperf,0,SEEK_SET);                    \
	read(fperf,perf_array,255);                 \
	write(fperf,"perfstop",8);                  \
	printf("%s",perf_array);

