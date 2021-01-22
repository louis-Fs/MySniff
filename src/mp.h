#ifndef MP_H_
#define MP_H_

#include <stdint.h>
#include "ip.h"

typedef struct igmp_head_t {
    uint8_t VT;
#define IGMP_V(ip) (((ip)->VT & 0xf0) >> 4)
#define IGMP_T(ip) ((ip)->VT & 0x0f)
    uint8_t unused;
    uint16_t checksum;
} igmp_head_t;

int igmp_parse(const ip_head_t* ip);


typedef struct icmp_head_t {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t flag;
    uint16_t seqnum;
    uint32_t option;//一般无
} icmp_head_t;

int icmp_parse(const ip_head_t* ip);

void datagram_type(uint8_t type, uint8_t code);


#endif


