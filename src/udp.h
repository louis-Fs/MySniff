#ifndef UDP_H_
#define UDP_H_

#include <stdint.h>


typedef struct udp_head_t {
    uint16_t sport;
    uint16_t dport;
    uint16_t datalen;
    uint16_t checksum;
} udp_head_t;


int udp_parse(const udp_head_t* head, uint32_t sip, uint32_t dip, uint16_t datalen);

#endif

void protocol_based_udp(uint16_t sport, uint16_t dport);