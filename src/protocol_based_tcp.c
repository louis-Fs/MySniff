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
    else if ((tcp->local_port == 22223) || (tcp->peer_port == 22223))
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

    //printf("%d global_flag\n",global_flag);

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
            /* do */
            telnet_print(start, length);
            telnet_packet_handle(start, sport, dport, length);
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
            telnet_print(start, length);
            telnet_packet_handle(start, sport, dport, length);
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

void telnet_print(const uint8_t *start, int length)
{
    int offset = 0;
    for (offset = 0; offset < length; offset++)
    {
        printf("%c", start[offset]);
    }
    printf("\n");
}
void telnet_txt_write(telnet_capture_t *me)
{
    FILE *fp;
    int length_username;
    int i= 0;
    char name[128];
    length_username = strlen(me->username);
    for ( i = 0; i < length_username; i += 2)
    {
        name[i/2]=me->username[i];
    }
    
    fp = fopen("telnet.txt", "a+");

    printf("U:%s  P:%s\n", name, me->password);

    fprintf(fp, "%s %s ", name, me->password);
    fputc(10, fp);
    fclose(fp);
}
void telnet_packet_handle(const uint8_t *start, uint16_t sport, uint16_t dport, int length)
{
    telnet_capture_t *telnet = NULL;
    telnet_capture_t *temp = NULL;
    telnet_capture_t *delete_temp = NULL;
    int find_flag = 0;
    int initial = 0;
    for (temp = head; temp->next != NULL; temp = temp->next)
    {
        if (temp->next->peer_port == dport)
        {
            if (length == 1 || (length ==2 && start[0] != '\x0d'))
            //if (temp->next->write_flag == 1  && start[0] != '\x0d' )
            {
                if (temp->next->write_flag == 1)
                {

                    if (temp->next->username_finish_flag == 0)
                    {
                        strncat(temp->next->username, (char *)start, length);
                        printf("username %s++++++++\n", temp->next->username);
                        //printf("%c *************\n", start[0]);
                    }
                    else if (temp->next->password_finish_flag == 0)
                    {
                        strncat(temp->next->password, (char *)start, length);
                        printf("%c password*************\n", start[0]);
                    }
                    else
                    {
                        temp->next->write_flag = 0;
                    }
                }

                find_flag = 1;
                break;
            }
            else if (length == 2)
            {
                if (temp->next->username_finish_flag == 0)
                {
                    if (start[0] == '\x0d')
                    {
                        temp->next->username_finish_flag = 1;
                    }
                }
                else if (temp->next->password_finish_flag == 0)
                {
                    if (start[0] == '\x0d')
                    {
                        temp->next->username_finish_flag = 1;
                        printf("begin to write\n");
                        telnet_txt_write(temp->next);
                        delete_temp = temp->next->next;
                        temp->next = delete_temp;
                    }
                }
                else
                {
                    temp->next->write_flag = 0;
                }

                find_flag = 1;
                break;
            }
            else if (length > 7)
            {
                //printf("start %s\n------------", start);
                if (strstr((char *)(start + length - 8), "login") != NULL)
                {
                    temp->next->write_flag = 1;
                    temp->next->username_finish_flag = 0;
                    printf("检测到login\n");

                    find_flag = 1;
                    break;
                }

                else if (memcmp((char *)start, "Password:\x20", 10) == 0)
                {
                    temp->next->write_flag = 1;
                    temp->next->password_finish_flag = 0;
                    printf(" password checked\n");

                    find_flag = 1;
                    break;
                }
                find_flag = 1;
                break;
            }

            find_flag = 1;
            break;
        }
    }
    if (find_flag == 0)
    {
        telnet = (telnet_capture_t *)malloc(sizeof(telnet_capture_t));
        printf(" 检测到telnet登录\n" );
        telnet->peer_port = dport;
        telnet->local_port = sport;
        telnet->write_flag = 0;
        telnet->username_finish_flag = 1;
        telnet->password_finish_flag = 1;
        for (; initial < 128; initial++)
        {
            telnet->username[initial] = '\x00';
            telnet->password[initial] = '\x00';
        }
        telnet->next = NULL;

        for (temp = head; temp != NULL; temp = temp->next)
        {
            if (temp->next == NULL)
            {
                temp->next = telnet;
                break;
            }
        }
    }
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
