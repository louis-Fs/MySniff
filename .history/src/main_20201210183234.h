#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

int is_local_address(uint32_t addr);


typedef struct telnet_capture_t 
{
    uint16_t peer_port;
    uint16_t local_port;
    //int use_flag; //struct used or not
    //int grap_finish_flag;
    int username_finish_flag;
    int password_finish_flag;
    int write_flag;
    char username[128];
    char password[128];
    struct telnet_capture_t *next;

} telnet_capture_t;

extern int g_debug;

#define PRINTF(fmt, args...) { if (g_debug) { printf(fmt, ##args); } }

#endif

