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
#include "tcp.h"
#include "ip.h"
#include "main.h"
#include "checksum.h"
#include "link.h"
#include "protocol_based_tcp.h"
#include "output.h"

int _tcp_checksum(const tcp_head_t *tcp, uint32_t sip, uint32_t dip, uint16_t tcpbytes)
{
    // pseudo header
    struct phead
    {
        uint32_t sip;
        uint32_t dip;
        uint8_t mbz;
        uint8_t proto;
        uint16_t len;
    } ph;
    ph.mbz = 0;
    ph.proto = IP_TCP;
    ph.len = htons(tcpbytes);
    ph.sip = sip;
    ph.dip = dip;
    struct cksum_vec vec[2];
    vec[0].ptr = (const uint8_t *)(&ph);
    vec[0].len = sizeof(ph);
    vec[1].ptr = (const uint8_t *)(tcp);
    vec[1].len = tcpbytes;
    uint16_t sum = checksum(vec, 2);
    if (sum != 0)
    {
        uint16_t tcpsum = ntohs(tcp->checksum);
        PRINTF("tcp checksum: %x, tcpbytes=%d, head->cksum=%d\n", sum, tcpbytes, tcpsum);
        return GAZE_TCP_CHECKSUM_ERROR;
    }
    return 0;
}

int _tcp_option(const unsigned char *start, int bytes)
{
    PRINTF("\t");
    for (int i = 0; i < bytes;)
    {
        if (start[i] == TCP_OPTION_NOP)
        {
            PRINTF("<nop> ");
            ++i;
            continue;
        }

        if (i + 1 >= bytes)
            return GAZE_TCP_OPTION_FAIL;
        int len = (int)start[i + 1];
        if (i + len > bytes)
            return GAZE_TCP_OPTION_FAIL;

        if (start[i] == TCP_OPTION_MSS)
        {
            assert(len == 4);
            PRINTF("<mss %u> ", ntohs(*(uint16_t *)&start[i + 2]));
        }
        else if (start[i] == TCP_OPTION_TS)
        {
            assert(len == 10);
            PRINTF("<ts %u %u> ", ntohl(*(uint32_t *)&start[i + 2]), ntohl(*(uint32_t *)&start[i + 6]));
        }
        else if (start[i] == TCP_OPTION_WND_SCALE)
        {
            assert(len == 3);
            PRINTF("<window scale %d> ", (int)*(uint8_t *)&start[i + 2]);
        }
        else if (start[i] == TCP_OPTION_SACK)
        {
            assert(len == 2);
            PRINTF("<SACK> ");
        }
        else if (start[i] == TCP_OPTION_EOF)
        {
            break;
        }
        else
        {
            PRINTF("<option[%d]> ", start[i]);
        }
        i += len;
    }
    PRINTF("\n");
    return 0;
}

int _tcp_head_option(const tcp_head_t *tcp, uint32_t sip, uint32_t dip)
{

    // tcp port
    uint16_t sport, dport;
    sport = ntohs(tcp->sport);
    dport = ntohs(tcp->dport);

    // print address
    struct in_addr addr;
    addr.s_addr = sip;
    if (is_local_address(sip) == 0)
    {
        PRINTF("local[%s:%d] --> peer[", inet_ntoa(addr), sport);
    }
    else
    {
        PRINTF("peer[%s:%d] --> local[", inet_ntoa(addr), sport);
    }
    addr.s_addr = dip;
    PRINTF("%s:%d]\n", inet_ntoa(addr), dport);

    // option
    int headbytes = (int)(tcp->offx2 >> 4) << 2;
    int optbytes = headbytes - sizeof(tcp_head_t);
    if (optbytes > 0)
    {
        const unsigned char *start = (const unsigned char *)tcp + sizeof(tcp_head_t);
        return _tcp_option(start, optbytes);
    }
    return 0;
}

int _tcp_flag(const tcp_head_t *tcp, link_key_t *key, struct link_value_t *val, uint16_t tcpbytes)
{
    uint32_t seq = ntohl(tcp->seq);
    uint32_t ack = ntohl(tcp->ack);
    link_value_on_seq(val, seq);
    if (tcp->flags & TCP_FLAG_ACK)
    {
        link_value_on_ack(key, val, ack);
    }
    if (tcp->flags & TCP_FLAG_FIN)
    {
        link_value_on_fin(val, seq);
    }
    if (tcp->flags & TCP_FLAG_SYN)
    {
        PRINTF("\tSYN\n");
    }
    if (tcp->flags & TCP_FLAG_RST)
    {
        PRINTF("\tRST\n");
    }
    int headbytes = (int)(tcp->offx2 >> 4) << 2;
    int databytes = tcpbytes - headbytes;
    if (tcp->flags & TCP_FLAG_PSH)
    {
        PRINTF("\tPSH[%d]\n", databytes);
    }
    if (databytes > 0)
    {
        link_value_on_psh(val, seq, databytes, (const char *)tcp + headbytes);
    }
    if (tcp->flags & TCP_FLAG_URG)
    {
        PRINTF("\tURG\n");
    }
    return 0;
}

int tcp_parse(const tcp_head_t *tcp, uint32_t sip, uint32_t dip, uint16_t tcpbytes)
{
    int ret;

    
    

    ///////////////////////////////********************////////////////////////////////
    //////////////////////////////下面这个地方需要全局变量 int protocol_based_tcp ////
    ////////////////////////////我先定义一个局部变量 int protocol_based_tcp 你到时候在改就行了
    /*

    int protocol_based_tcp = 1; //http
    int protocol_based_tcp = 2; //telnet
    int protocol_based_tcp = 3; //ftp

    */

    /*
    下面这个部分就是打印tcp payload的函数，payload里面可以判断具体是什么协议
    g_recv_hook g_recv_hook 就是打印所有的tcp payload，我会把他注释掉，然后放进
    我的函数里，如果符合协议 才会打印。
   */
  /*
    const unsigned char *start;
    int protocol_based_tcp_flag;
    int headbytes = (int)(tcp->offx2 >> 4) << 2;

    start = (const unsigned char *)tcp + headbytes;
    _output_bytes(start, tcpbytes);
    protocol_based_tcp_flag = protocol_based_tcp(tcp, start);
    protocol_process(tcp, protocol_based_tcp_flag, start, tcpbytes);
    */

    // tcp checksum
    if (is_local_address(sip))
    {
        ret = _tcp_checksum(tcp, sip, dip, tcpbytes);
        if (ret < 0)
        {
            return ret;
        }
    }

    // tcp head option
    ret = _tcp_head_option(tcp, sip, dip);
    if (ret < 0)
    {
        return ret;
    }

    // tcpp link
    link_key_t key;
    link_key_init(&key, sip, dip, ntohs(tcp->sport), ntohs(tcp->dport));
    struct link_value_t *val = link_find_insert(&key, is_local_address(sip));
    if (!val)
        return GAZE_TCP_LINK_FAIL;

    // tcp flag
    ret = _tcp_flag(tcp, &key, val, tcpbytes);
    if (ret < 0)
    {
        return ret;
    }

    // tcp finish
    ret = link_value_is_finish(val);
    if (ret == 0)
    {
        link_erase(&key);
    }

    PRINTF("\n\n");
    return GAZE_OK;
}
