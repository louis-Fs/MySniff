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
#include "arp.h"

int arp_parse(const arp_head_t* head)
{
    int send=1;
    int receve=0;
    const char* buffer=(const char*)(head);
    mac_addr_t *sender_mac = ( mac_addr_t *)(head + 1);
    ip_addr_t  *sender_ip = (ip_addr_t *)(sender_mac+1);

    mac_addr_t *receiver_mac = ( mac_addr_t *)(sender_ip + 1);
    ip_addr_t  *receiver_ip = (ip_addr_t *)(receiver_mac + 1);

    printf("***************protacol:ARP******************\n");

    print_mac(send,sender_mac->data);
    print_ip(send,sender_ip->data);

    print_mac(receve,receiver_mac->data);
    print_ip(receve,receiver_ip->data);
    printf("\n\n");

    _output_bytes(buffer,28);

    return GAZE_OK;



}

