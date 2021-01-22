#ifdef MINGW
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "eth.h"
#include "gaze.h"
#include "ip.h"
#include "arp.h"
#include "output.h"
int pppoe_parse_S(const pppoe_head_t *pppoe)
{
    const pppoe_head_t *ppp = pppoe;
    uint16_t protocol = ntohs(ppp->protocol);
    switch (protocol)
    {
    case PPP_IP:
        //printf("IP\n");
        return ip_parse((const ip_head_t *)(ppp + 1));
    default:
        _output_bytes((char *)ppp, ntohs(ppp->length) + 8);
        return GAZE_OK;
    }
    return GAZE_ETH_FAIL;
}
int pppoe_parse_D(const pppoe_head_t *pppoe)
{
    const pppoe_head_t *ppp = pppoe;
    _output_bytes((char *)ppp, ntohs(ppp->length) + 8);
    return GAZE_OK;
}

int eth_parse(const unsigned char *data)
{
    const eth_head_t *eth = (eth_head_t *)data;
    uint16_t type = ntohs(eth->type);
    switch (type)
    {
    case ETH_IP:
        //printf("IP\n");
        return ip_parse((const ip_head_t *)(eth + 1));
    case ETH_ARP:
        return arp_parse((const arp_head_t *)(eth + 1));
    case ETH_RARP:
        return GAZE_ETH_NOT_SUPPORT;
    case ETH_PPPoE_Session:
        return pppoe_parse_S((const pppoe_head_t *)(eth + 1));
    case ETH_PPPoE_Discovery:
        return pppoe_parse_D((const pppoe_head_t *)(eth + 1));
    default:
        return GAZE_ETH_NOT_SUPPORT;
    }

    return GAZE_ETH_FAIL;
}
