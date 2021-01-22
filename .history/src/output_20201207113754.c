#include <stdio.h>
#include <dlfcn.h>
#include <ctype.h>
#include <string.h>

#include "gaze.h"
#include "output.h"
#include "main.h"

char* findStrOfData (const char *packet, char *inputStr)
{
    return strstr(packet, inputStr);
}

int getHttpMethod(const char *packet)
{
    char[5] temp;

}

void 
print(const char *payload, int len, int offset, int maxlen)
{ 
    PRINTF("%.5x  ", offset); //打印偏移量(宽度为5)
    int max = maxlen;         //数据包的有效载荷和长度
    int i;
    for (i = 0; i < 16; i++) //打印16个字节的16进制payload
    {
        if ((len - i) > 0) //还没打完
        {
            PRINTF("%.2x ", (const uint8_t) payload[max - (len - i)]);
        }
        else //已打完，最后一个后面有空格
        {
            PRINTF("   ");
        }
    }
    PRINTF("	");
    for (i = 0; i < 16; i++) //打印16个字节的asciipayload
    {
        if (isprint(payload[max - (len - i)])) //为可打印字符
        {
            PRINTF("%c", payload[max - (len - i)]);
        }
        else //打印不出来的用"."表示
        {
            PRINTF(".");
        }
    }
}

void
_output_bytes(const char* buffer, int len) {
    int line_width = 16; //一行16个字节
    int len_rem = len;   //剩余长度
    int maxlen = len;    //数据包的有效载荷和长度
    int offset = 0;      //偏移量
    while (1)
    {
        if (len_rem < line_width) //最后一次打印
        {
            if (len_rem == 0) //已打印完
            {
		PRINTF("\n");
                break;
            }
            else
            {                                            //还没打印完
                print(buffer, len_rem, offset, maxlen); //调用print函数，传入payload地址、剩余长度、偏移量和数据包的有效载荷和长度
                offset = offset + len_rem;               //偏移量后移
                PRINTF("\n");
		PRINTF("\n");
                break;
            }
        }
        else //不是最后一次打印
        {
            print(buffer, len_rem, offset, maxlen); //调用print函数，传入payload地址、剩余长度、偏移量和数据包的有效载荷和长度
            offset = offset + 16;                    //偏移量后移(由于非最后一次打印，所以固定打16个字节 - 偏移量后移16个字节)
            PRINTF("\n");
        }
        len_rem = len_rem - line_width; //剩余长度减少
    }
}

//打印mac地址
void 
print_mac(int flag, uint8_t *macadd)
{
    int i;
    char *name= (flag==1)?"sender":"receiver";
    printf("%s MAC: ",name);
    for (i = 0; i < 5; i++)
    {
        printf("%.2x:", macadd[i]); 
        //16进制，两位宽度
    }
    printf("%.2x\n", macadd[i]);
}

//打印ip地址
void 
print_ip(int flag, const uint8_t *ipadd)
{
    int i;
    char *name= (flag==1)?"sender":"receiver";
    if (flag==-1)
    {
        printf("IP: ");
    }else
    {
        printf("%s IP: ",name);
    }
    
    
    for (i = 0; i < 3; ++i)
    {
        printf("%d.", ipadd[i]);
    }
    printf("%d\n", ipadd[i]);
}

void
_output_send(link_key_t* key, const char* buffer, int len) {
    _output_bytes(buffer, len);
}

void
_output_recv(link_key_t* key, const char* buffer, int len) {
    _output_bytes(buffer, len);
}

void
_output_build(link_key_t* key) {
}

void
_output_finish(link_key_t* key) {
}

// local default hook functions
send_hook g_send_hook = _output_send;
recv_hook g_recv_hook = _output_recv;
build_hook g_build_hook = _output_build;
finish_hook g_finish_hook = _output_finish;

int
output_load_dylib(const char* file) {
    void* handle = dlopen(file, RTLD_NOW|RTLD_GLOBAL);
    if (!handle) {
        printf("load %s fail: %s\n", file, dlerror());
        printf("load %s fail: %s\n", file, dlerror());
        return GAZE_DYLIB_FAIL;
    }

    void* dlsend = dlsym(handle, DYLIB_SEND_SYMBOL);
    void* dlrecv = dlsym(handle, DYLIB_RECV_SYMBOL);
    void* dlbuild = dlsym(handle, DYLIB_BUILD_SYMBOL);
    void* dlfinish = dlsym(handle, DYLIB_FINISH_SYMBOL);
    if (!dlsend || !dlrecv || !dlbuild || !dlfinish) {
        printf("load %s symble not found\n", file);
        return GAZE_DYLIB_SYMBLE_FAIL;
    }
    g_send_hook = (send_hook)dlsend;
    g_recv_hook = (recv_hook)dlrecv;
    g_build_hook = (build_hook)dlbuild;
    g_finish_hook = (finish_hook)dlfinish;
    return 0;
}
