#ifndef ETH_H_
#define ETH_H_

#include <stdint.h>

// RFC 894 ethernet frame
// 以太网封装 不需要做CRC校验(硬件会做)
typedef struct eth_head_t {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type;
} eth_head_t;

typedef struct pppoe_head_t {
    uint8_t vTy;
#define PPPoE_V(pppoe) (((pppoe)->vhl & 0xf0) >> 4)
#define PPPoE_Ty(pppoe) ((pppoe)->vhl & 0x0f)
    uint8_t code;
    uint16_t session_id;
    uint16_t length;
    uint16_t protocol;
} pppoe_head_t;

#define ETH_IP 0x0800
#define ETH_ARP 0x0806
#define ETH_RARP 0x8035
#define ETH_PPPoE_Session 0x8864
#define ETH_PPPoE_Discovery 0x8863


#define PPP_IP 0x0021
#define PPP_LCP 0xC021
#define PPP_NCP 0x8021
#define PPP_PAP 0xC023
#define PPP_LQR 0xC025
#define PPP_CHAP 0xC223

int eth_parse(const unsigned char* data);
int pppoe_parse_S(const pppoe_head_t* pppoe);
int pppoe_parse_D(const pppoe_head_t* pppoe);
#endif
