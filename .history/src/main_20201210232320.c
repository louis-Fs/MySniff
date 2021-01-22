#ifdef MINGW
#include <winsock2.h>

#define isMINGW 1

#else
#include <arpa/inet.h>

#define isMINGW 0

#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <pcap.h>
#include <assert.h>

#include "eth.h"
#include "gaze.h"
#include "link.h"
#include "output.h"
#include "protocol_based_tcp.h"
#include "main.h"
void _usage()
{
    printf("usage:\n"
           "\t--tcp         <default/http/ftp/telnet/packet>     \"tcp packets\"\n"
           "\t--udp              \"udp packets\"\n"
           "\t--icmp              \"icmp packets\"\n"
           "\t--igmp              \"igmp packets\"\n"
           "\t--arp              \"arp packets\"\n"
           "\t--eth      <device id>  \"device name, default use first eth device\"\n"
           "\t--ip       <ip address>\n"
           "\t--debug            \"print ip & tcp level debug info\"\n"
           "\t--port     <port>\n"
           "\t--help             \"show usage\"\n");
}

static const char *g_name = "default";
static const char *g_filter = NULL;
static pcap_if_t *g_device = NULL;

int g_debug = 0;
int global_flag = 5;

typedef struct ip_addrs_t
{
    int num;
#define MAX_LOCAL_IP_ADDR_NUM 8
    uint32_t addrs[MAX_LOCAL_IP_ADDR_NUM];
} ip_addrs_t;

static ip_addrs_t g_addrs;
telnet_capture_t *head;
int is_local_address(uint32_t addr)
{
    for (int i = 0; i < g_addrs.num; ++i)
    {
        if (g_addrs.addrs[i] == addr)
            return 0;
    }
    return -1;
}

void _get_all_devices()
{
    char err[PCAP_ERRBUF_SIZE];
    if (pcap_findalldevs(&g_device, err) < 0)
    {
        printf("find all devs error: %s\n", err);
        exit(-1);
    }
}

char *_choose_the_devices(unsigned int input, int mode)
{
    _get_all_devices();
    printf("local devices as flowing: \n");
    printf("\tDescription\t\tname\t\tflag\n");

    unsigned int i = 1;

    //unsigned int input = 0;

    char *temp = (char *)malloc(sizeof(char) * 100);

    for (pcap_if_t *dev = g_device; dev; dev = dev->next)
    {
        dev->flags = i;
        i++;
        if (mode == 0)
        {
            printf("\t%s\t%s\t%u\n", dev->description, dev->name, dev->flags);
            printf("\n");
        }
    }
    if (mode == 0)
    {
        return "No Find Device";
    }
    else
    {
        for (pcap_if_t *dev = g_device; dev; dev = dev->next)
        {
            if (dev->flags == input)
            {
                int temLength;
                int i = 0;
                temLength = strlen(dev->name);
                for (i = 0; i < temLength; i++)
                {
                    temp[i] = dev->name[i];
                }
                temp[temLength] = '\0';

                break;
            }
            else
            {
                return "default";
            }
            
        }
        return temp;
    }

    /*
    while (1)
    {
        printf("You can choose the service you want to sniff with flag\n");
        printf("flag:");
        scanf("%d", &input);
        if (input >= i || input < 0)
        {
            fflush(stdin);
            fflush(stdin);
            printf("You Enter a wrrong number\n");
        }
        else if (input == 0)
        {
            temp = "default";
            return temp;
        }
        else
        {
            for (pcap_if_t *dev = g_device; dev; dev = dev->next)
            {
                if (dev->flags == input)
                {
                    int temLength;
                    int i = 0;
                    temLength = strlen(dev->name);
                    for (i = 0; i < temLength; i++)
                    {
                        temp[i] = dev->name[i];
                    }
                    temp[temLength] = '\0';

                    break;
                }
            }
            break;
        }
    }
    
    return temp;*/
}

int _parse(int argc, char **argv)
{

    struct option opts[] = {
        {"tcp", optional_argument, 0, 't'},
        {"udp", no_argument, 0, 'u'},
        {"eth", optional_argument, 0, 'e'},
        {"ip", required_argument, 0, 'i'},
        {"port", required_argument, 0, 'p'},
        {"plugin", required_argument, 0, 'g'},
        {"debug", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"arp", no_argument, 0, 'a'},
        {"icmp", no_argument, 0, 'x'},
        {"igmp", no_argument, 0, 'y'},
        {0, 0, 0, 0}};

    int index, c;
    static char filter[1024] = {0};
    static char name[1024];
    int first = 0;
    while ((c = getopt_long(argc, argv, "", opts, &index)) != -1)
    {
        switch (c)
        {
        case 'x':
            snprintf(filter + strlen(filter), sizeof(filter) - strlen(filter),
                     first == 0 ? "icmp" : " and icmp");
            first = 1;
            break;

        case 'y':
            snprintf(filter + strlen(filter), sizeof(filter) - strlen(filter),
                     first == 0 ? "igmp" : " and igmp");
            first = 1;
            break;

        case 'a':
            snprintf(filter + strlen(filter), sizeof(filter) - strlen(filter),
                     first == 0 ? "arp" : " and arp");
            first = 1;
            break;

        case 't':
            snprintf(filter + strlen(filter), sizeof(filter) - strlen(filter),
                     first == 0 ? "tcp" : " and tcp");
            first = 1;
            if (optarg)
            {
                optarg = strlwr(optarg);
                if (strcmp(optarg, "default") == 0)
                {
                    global_flag = 0;
                    break;
                }
                else if (strcmp(optarg, "http") == 0)
                {
                    global_flag = 1;
                    break;
                }
                else if (strcmp(optarg, "telnet") == 0)
                {
                    global_flag = 2;
                    break;
                }
                else if (strcmp(optarg, "ftp") == 0)
                {
                    global_flag = 3;
                    break;
                }
                else if (strcmp(optarg, "packet") == 0)
                {
                    global_flag = 4;
                    break;
                }
                else
                {
                    printf("You may enter a wrong command!!!");
                    exit(-1);
                }
            }
            else
            {
                global_flag = 4;
                break;
            }

        case 'u':
            printf("udp\n");
            snprintf(filter + strlen(filter), sizeof(filter) - strlen(filter),
                     first == 0 ? "udp" : " and udp");
            first = 1;
            break;

        case 'e':
            if (optarg)
            {   
                char *name;
                if (isMINGW)
                {
                    name = _choose_the_devices(atoi(optarg), 1);
                    g_name = name;
                    break;
                }
                else
                {
                    name = _choose_the_devices(atoi(optarg), 1);
                    g_name = name;
                    break;
                }
            }
            else
            {
                if (isMINGW)
                {
                    GetNameOfNetServiceWin();
                    _choose_the_devices(atoi(optarg), 0);
                }
                else
                {
                    _choose_the_devices(atoi(optarg), 0);
                }
                //snprintf(name, sizeof(name), "%s", optarg);
                //printf("%s",name);
                //g_name = name;
                exit(-1);
            }

        case 'i':
            snprintf(filter + strlen(filter), sizeof(filter) - strlen(filter),
                     first == 0 ? "host %s" : " and host %s", optarg);
            first = 1;
            break;

        case 'p':
            snprintf(filter + strlen(filter), sizeof(filter) - strlen(filter),
                     first == 0 ? "port %d" : " and port %d", atoi(optarg));
            first = 1;
            break;

        case 'd':
            g_debug = 1;
            break;

        case 'g':
            if (output_load_dylib(optarg))
            {
                _usage();
                exit(-1);
            }
            break;

        case 'h':
        case '?':
        default:
            _usage();
            exit(-1);
            break;
        };
    }

    g_filter = filter[0] ? filter : NULL;
    return 0;
}

pcap_if_t *
_get_device()
{
    pcap_if_t *dev = NULL;

    _get_all_devices();

    if (strcmp(g_name, "default"))
    {
        for (dev = g_device; dev; dev = dev->next)
        {
            if (strcmp(dev->name, g_name) == 0)
            {
                break;
            }
        }
    }
    else
    {
        dev = g_device;
        printf("listening on device[%s]\n", g_device->name);
    }

    if (!dev)
    {
        printf("dev[%s] not found error\n", g_name);
    }
    return dev;
}

void _get_address(pcap_if_t *dev)
{
    if (dev)
    {
        pcap_addr_t *addr = dev->addresses;
        // maybe multi ip addresses
        while (addr)
        {
            struct sockaddr_in *self = (struct sockaddr_in *)addr->addr;
            printf("self ip: %s\n", inet_ntoa(self->sin_addr));

            assert(g_addrs.num < MAX_LOCAL_IP_ADDR_NUM);
            g_addrs.addrs[g_addrs.num++] = *(uint32_t *)&self->sin_addr;

            addr = addr->next;
        }
    }
}

pcap_t *
_open_device(pcap_if_t *dev)
{
    char err[PCAP_ERRBUF_SIZE];
    pcap_t *adhandle = pcap_open_live(dev->name, 65536, 0, 1000, err);
    if (!adhandle)
    {
        printf("unable to gaze %s: %s\n", dev->name, err);
        return NULL;
    }
    return adhandle;
}

int _filter(pcap_if_t *dev, pcap_t *adhandle)
{
    int initial = 0;
    if (g_filter)
    {
        struct bpf_program fcode;
        if (pcap_compile(adhandle, &fcode, g_filter, 1, 0) < 0)
        {
            printf("unable to compile filter: %s\n", g_filter);
            return -1;
        }
        if (pcap_setfilter(adhandle, &fcode) < 0)
        {
            printf("unable to set filter: %s\n", g_filter);
            return -1;
        }
    }
    printf("listening on %s", dev->description ? dev->description : dev->name);
    if (g_filter)
    {
        printf("(%s)", g_filter);
    }
    printf(" ...\n");
    /*
    if (g_filter != NULL)
    {
        if (strstr(g_filter, "tcp") != NULL)
        {
            while (1)
            {
                printf("You can point the Apllication Protol You Want To Sniff !!!\n");
                printf("0 --- Default(Http Telnet Ftp)\n"
                       "1 --- Http\n"
                       "2 --- Telnet\n"
                       "3 --- Ftp\n"
                       "4 --- Packet Analysis\n");
                printf("Enter the Option:");
                scanf("%d", &global_flag);
                printf("\n");
                if (global_flag > 4 || global_flag < 0)
                {
                    fflush(stdin);
                    fflush(stdin);
                    printf("You Enter a wrong number!!!\n");
                }
                */
    if (global_flag == 2)
    {
        head = (telnet_capture_t *)malloc(sizeof(telnet_capture_t));
        //head->write_flag=0;
        head->peer_port = 0;
        head->local_port = 0;
        head->write_flag = -1;
        head->username_finish_flag = -1;
        head->password_finish_flag = -1;
        for (; initial < 128; initial++)
        {
            head->username[initial] = '\x20';
            head->password[initial] = '\x20';
        }
        head->next = NULL;
    }
    return 0;
}


void _poll_device(pcap_t *adhandle)
{
    struct pcap_pkthdr *header;
    const unsigned char *data;
    int res;
    while ((res = pcap_next_ex(adhandle, &header, &data)) >= 0)
    {
        if (0 == res)
            continue; // timeout
        errno = eth_parse(data);
        if (errno != GAZE_OK)
        {
            printf("MySniff not soupport this packet's potocol: %d\n\n", errno);
        }
    }
    printf("reading packet: %s\n", pcap_geterr(adhandle));
}

int main(int argc, char **argv)
{

    int ret = _parse(argc, argv);
    if (ret < 0)
        return -1;

    pcap_if_t *dev = _get_device();
    if (!dev)
        goto EXIT;

    memset(&g_addrs, 0, sizeof(g_addrs));
    _get_address(dev);

    pcap_t *adhandle = _open_device(dev);
    if (!adhandle)
        goto EXIT;

    ret = _filter(dev, adhandle);
    if (ret < 0)
        goto EXIT;

    ret = link_create();
    if (ret < 0)
        goto EXIT;

    _poll_device(adhandle);

EXIT:
    if (!g_device)
    {
        pcap_freealldevs(g_device);
    }
    link_release();
    return 0;
}
