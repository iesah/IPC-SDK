#ifndef __VOICE_second_refresh_MODULE_H__
#define __VOICE_second_refresh_MODULE_H__

int second_refresh_open(int mode);

int second_refresh_close(int mode);

void second_refresh_cache_prefetch(void);

int second_refresh_handler(int par);

#endif
