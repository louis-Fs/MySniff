#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

int is_local_address(uint32_t addr);

extern int g_debug;
extern int global_flag;

#define PRINTF(fmt, args...) { if (g_debug) { printf(fmt, ##args); } }

#endif

