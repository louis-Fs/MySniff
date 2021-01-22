#ifndef PROTOCOL_BASED_TCP_H_
#define PROTOCOL_BASED_TCP_H_

#include <stdint.h>
#include "gaze.h"
#include "tcp.h"

extern int global_flag;

int protocol_based_tcp(link_key_t *tcp, const uint8_t *start);
void protocol_process(link_key_t *tcp, int flag, const uint8_t *start, int lengt);



int http_justify(const uint8_t *start);
int http_methods(const uint8_t *start);
int three_str_cmp(const char *src, const char *str1, const char *str2, int len);
int http_print(const uint8_t *start, int length);
void http_txt_write(const uint8_t *data);

void ftp_txt_write(const uint8_t *data, uint16_t sport, uint16_t dport, int totlen);
void ftp_print(const uint8_t *start, int length);

#endif