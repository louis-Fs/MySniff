#ifdef MINGW
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <stdio.h>

#include "gaze.h"
#include "main.h"
#include "output.h"
#include "ip.h"
#include "mp.h"

int igmp_parse(const ip_head_t* ip)
{
    int flag=-1;
    int send=1;
    int receve=0;
    //ip_head_t* ip_=ip;
    igmp_head_t* igmp=(igmp_head_t*)(ip + 1);
    ip_addr_t* ip_no_name=(ip_addr_t*)(igmp + 1);

    char *type= (IGMP_T(igmp) == 1 )?"ordinary_query":"membership_report";
    printf("***************protacol:IGMP******************\n");
    print_ip(send,ip->src.data);
    print_ip(receve,ip->dst.data);
    printf("\n");
    if (IGMP_V(igmp) == 1) {
        printf("version: 1\n");
        printf("datagram type:%s\n",type);
    }
    print_ip(flag,ip_no_name->data);
    
    return GAZE_OK;
}

void datagram_type(uint8_t type, uint8_t code)
{
    uint8_t type_ = (type & 0x0f)<<4;
    uint8_t code_ = (code & 0x0f);
    uint8_t type_code = (type_ | code_);
    switch (type_code)
    {
    case 0x00:
        printf("echo reply(ping)\n");
        break;
    case 0x30:
        printf("dest. network unreachable\n");
        break;
    case 0x31:
        printf("dest host unreachable\n");
        break;
    case 0x32:
        printf("dest protocol unreachable\n");
        break;
    case 0x33:
        printf("dest port unreachable\n");
        break;
    case 0x36:
        printf("dest network unknown\n");
        break;
    case 0x37:
        printf("dest host unknown\n");
        break;
    case 0x40:
        printf("source quench\n");
        break;
    case 0x80:
        printf("echo request (ping)\n");
        break;
    case 0x90:
        printf("route advertisement\n");
        break;
    case 0xa0:
        printf("router discovery\n");
        break;
    case 0xb0:
        printf("TTL expired\n");
        break;
    case 0xc0:
        printf("bad IP header\n");
        break;
    
    default:
        break;
    }
}
int icmp_parse(const ip_head_t* ip)
{
    int send=1;
    int receve=0;
    //ip_head_t* ip_=ip;
    icmp_head_t* icmp=(icmp_head_t*)(ip + 1);
    printf("***************protacol:ICMP******************\n");
    datagram_type(icmp->type,icmp->code);
    printf("seqence number: %d  \n",ntohs(icmp->seqnum));
    print_ip(send,ip->src.data);
    print_ip(receve,ip->dst.data);
    printf("\n");
    
    
    return GAZE_OK;

}


