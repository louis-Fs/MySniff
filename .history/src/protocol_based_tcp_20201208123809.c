#ifdef MINGW
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "protocol_based_tcp.h"

int protocol_based_tcp(link_key_t *tcp, const uint8_t *start)
{
    if (http_justify(start) == 1)
    {
        return 1; //http
    }

    else if ((tcp->local_port) == 21 || (tcp->peer_port == 21))
    {

        //printf("ftp*******\n");
        return 3; //ftp
    }
    else if ((tcp->local_port == 23) || (tcp->peer_port == 23))
    {
        return 2; //telnet
    }
    else
    {
        return 0;
    }
}

void protocol_process(link_key_t *tcp, int flag, const uint8_t *start, int length)
{
    // 1 http 2 telnet 3 ftp 0 三个都要
    //int global_flag = 0;

    int offset = 0;
    // tcp port
    uint16_t sport, dport;
    sport = /**/ (tcp->local_port);
    dport = /**/ (tcp->peer_port);

    printf("%d global_flag\n",global_flag);

    switch (global_flag)
    {
    case 1:
        /* code http*/
        if (flag == 1)
        {
            /* do*/
            /* code http*/
            offset = http_print(start, length);
            printf("offset %d length %d\n", offset, length);
            //printf("%c\n", start[offset]);
            if (offset == -1)
            {
                printf("HTTP resolve error!\n");
                break;
            }
            else
            {
                if (http_methods(start) == 2)
                {
                    http_txt_write((start + offset));
                }
            }
        }

        break;
    case 2:
        /* code telnet*/
        if (flag == 2)
        {
            /* do */;
        }
        break;
    case 3:
        /* code ftp*/
        if (flag == 3)
        {
            /* do*/;
            /* code ftp*/
            ftp_print(start, length);
            ftp_txt_write(start, sport, dport, length);
        }

        break;
    case 0:
        /*http*/
        /*telnet*/
        /*ftp*/
        switch (flag)
        {
        case 1:
            /* code http*/
            offset = http_print(start, length);
            printf("offset %d length %d\n", offset, length);
            //printf("%c\n", start[offset]);
            if (offset == -1)
            {
                printf("HTTP resolve error!\n");
                break;
            }
            else
            {
                if (http_methods(start) == 2)
                {
                    http_txt_write((start + offset));
                }
            }

            break;
        case 2:
            /* code telnet*/
            break;
        case 3:
            /* code ftp*/
            ftp_print(start, length);
            ftp_txt_write(start, sport, dport, length);
            break;

        default:
            printf("unknown protocol based on tcp\n");
            break;
        }
        break;
    default:
        /*http*/
        /*telnet*/
        /*ftp*/
        switch (flag)
        {
        case 1:
            /* code http*/
            offset = http_print(start, length);
            printf("offset %d length %d\n", offset, length);
            //printf("%c\n", start[offset]);
            if (offset == -1)
            {
                printf("HTTP resolve error!\n");
                break;
            }
            else
            {
                if (http_methods(start) == 2)
                {
                    http_txt_write((start + offset));
                }
            }

            break;
        case 2:
            /* code telnet*/
            break;
        case 3:
            /* code ftp*/
            ftp_print(start, length);
            ftp_txt_write(start, sport, dport, length);
            break;

        default:
            printf("unknown protocol based on tcp\n");
            break;

        }
    }

    void ftp_txt_write(const uint8_t *data, uint16_t sport, uint16_t dport, int totlen)
    {
        FILE *fp;
        uint16_t port;
        int len = totlen - 2;
        const char text[len];
        //printf("FTP ************\n");
        memcpy((void *)text, (void *)data, len);
        if (memcmp((void *)text, "PASS", 4) == 0 || memcmp((void *)text, "USER", 4) == 0)
        {
            fp = fopen("ftp.txt", "a+");
            port = ((sport) != 21) ? (sport) : (dport);
            /*
        a+ 以附加方式打开可读写的文件。若文件不存在，则会建立该文件，
        如果文件存在，写入的数据会被加到文件尾后，即文件原先的内容会被保留。
        原来的EOF符不保留）
        */
            //printf("++++++++++++++%s %d+++++++++++++++++\n", text, len);
            fprintf(fp, "%s %d", text, port);
            fputc(32, fp);
            if (memcmp((void *)text, "PASS", 4) == 0)
            {
                fputc(10, fp);
            }

            fclose(fp);
        }
    }
    void ftp_print(const uint8_t *start, int length)
    {
        int offset = 0;
        for (offset = 0; offset < length; offset++)
        {
            printf("%c", start[offset]);
        }
        printf("\n");
    }
    void http_txt_write(const uint8_t *data)
    {
        FILE *fp;
        fp = fopen("http.txt", "a+");
        /*
    a+ 以附加方式打开可读写的文件。若文件不存在，则会建立该文件，
    如果文件存在，写入的数据会被加到文件尾后，即文件原先的内容会被保留。
     （原来的EOF符不保留）
    */
        //printf("%s\n", data);
        fprintf(fp, "%s ", data);
        fputc(10, fp);
        //fputs(data,fp);
        //printf("555");
        fclose(fp);
    }
    int http_print(const uint8_t *start, int length)
    {
        int offset = 0;
        for (offset = 0; offset < length; offset++)
        {
            if (memcmp((void *)(start + offset + 1), "\x0d\x0a", 2) == 0)
            {
                if (memcmp((void *)(start + offset + 1), "\x0d\x0a\x0d\x0a", 4) == 0)
                {
                    if (isprint(start[offset]))
                    {
                        printf("%c\n", start[offset]);
                        return offset + 5;
                    }
                    else
                    {
                        printf(".\n");
                        return offset + 5;
                    }
                }
                else
                {
                    if (isprint(start[offset]))
                    {
                        printf("%c\n", start[offset]);
                    }
                    else
                    {
                        //printf(".\n");
                    }
                }
            }
            else
            {
                if (isprint(start[offset]))
                {
                    printf("%c", start[offset]);
                }
                else
                {
                    //printf(".");
                }
            }
        }
        return -1;
    }
    int three_str_cmp(const char *src, const char *str1, const char *str2, int len)
    {
        if ((memcmp((void *)src, (void *)str1, len) * memcmp((void *)src, (void *)str2, len)) == 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    int http_methods(const uint8_t *start)
    {
        char get[3];
        char post[4];
        char put[3];
        char delete[6];
        char head[4];
        char trace[5];
        char options[7];
        memcpy((void *)get, (void *)start, 3);
        memcpy((void *)post, (void *)start, 4);
        memcpy((void *)put, (void *)start, 3);
        memcpy((void *)delete, (void *)start, 6);
        memcpy((void *)head, (void *)start, 4);
        memcpy((void *)trace, (void *)start, 5);
        memcpy((void *)options, (void *)start, 7);
        if (three_str_cmp(get, "GET", "get", 3))
        {
            return 1;
        }
        if (three_str_cmp(post, "POST", "post", 4))
        {
            return 2;
        }
        if (three_str_cmp(put, "PUT", "put", 3))
        {
            return 3;
        }
        if (three_str_cmp(delete, "DELETE", "delete", 6))
        {
            return 4;
        }
        if (three_str_cmp(head, "HEAD", "head", 4))
        {
            return 5;
        }
        if (three_str_cmp(trace, "TRACE", "trace", 5))
        {
            return 6;
        }
        if (three_str_cmp(options, "OPTIONS", "options", 7))
        {
            return 7;
        }
        return 0;
    }

    int http_justify(const uint8_t *start)
    {

        if (strstr((char *)start, "\x48\x54\x54\x50\x2f\x31\x2e") != NULL || strstr((char *)start, "\x48\x54\x54\x50\x2f\x31\x2e") != NULL)
        {
            //printf("http**********\n");
            return 1;
        }
        return 0;
    }
