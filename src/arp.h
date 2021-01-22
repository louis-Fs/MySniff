#ifndef ARP_H_
#define ARP_H_

#include <stdint.h>

typedef struct mac_addr_t {
    uint8_t data[6];
} mac_addr_t;

typedef struct arp_head_t {
    uint16_t hrd_type;
    uint16_t pro_type;
    uint8_t  hrd_addr_len;
    uint8_t  pro_addr_len;
    uint16_t opcode;
} arp_head_t;

int arp_parse(const arp_head_t* head);



#endif