#ifdef MINGW
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include "gaze.h"
#include "udp.h"
#include "ip.h"
#include "main.h"
#include "output.h"
#include "link.h"

void protocol_based_udp(uint16_t sport, uint16_t dport)
{
    if (sport == 67 || dport == 67)
    {
        printf("***************protacol:DHCP******************\n");
    }
    else if (sport == 53 || dport == 53)
    {
        printf("***************protacol:DNS******************\n");
    }
    else
    {
        printf("***************protacol:UDP******************\n");
    }
}
int udp_parse(const udp_head_t *udp, uint32_t sip, uint32_t dip, uint16_t datalen)
{

    int send = 1;
    int receive = 0;
    const char *buffer = (const char *)(udp);
    uint16_t sport = ntohs(udp->sport);
    uint16_t dport = ntohs(udp->dport);
    protocol_based_udp(sport, dport);
    printf("datalen:%d\n",ntohs(udp->datalen));
    printf("sender port:%d    receiver port:%d\n", sport, dport );
    print_ip(send,(const unsigned char*) &sip);
    print_ip(receive,(const unsigned char*) &dip);
    _output_bytes(buffer, ntohs(udp->datalen));
    printf("\n\n");
    return GAZE_OK;
}
