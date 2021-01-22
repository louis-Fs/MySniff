#ifdef MINGW
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "link.h"
#include "hash.h"
#include "main.h"
#include "output.h"
#include "gaze.h"
#include "protocol_based_tcp.h"

/*
#define MAX_SLICE_SIZE 65536
typedef struct slice_t {
    uint32_t seq;
    int offset;
    char buffer[MAX_SLICE_SIZE];
} slice_t;

typedef struct slab_t {
    slice_t slice;
    struct slab_t* next;
} slab_t;

typedef struct link_value_t {
    uint8_t flow;
    uint32_t start_send_seq;
    uint32_t start_recv_seq;
    uint32_t acked_send_seq;
    uint32_t acked_recv_seq;

    uint32_t send_fin_seq;
    uint32_t recv_fin_ack;

    uint32_t recv_fin_seq;
    uint32_t send_fin_ack;

    slab_t* send;
    slab_t* recv;
    slab_t* freelst;
} link_value_t;

typedef struct link_t {
    link_key_t key;
    link_value_t value;
} link_t;

static struct hash_t* g_links = NULL;
*/
static struct hash_t* g_links = NULL;

uint32_t
_link_hash(const void* data) {
    const link_t* link = (const link_t*)data;
    return hash_jhash((const void*)&link->key, sizeof(link_key_t));
}

int32_t
_link_cmp(const void* data1, const void* data2) {
    const link_t* link1 = (const link_t*)data1;
    const link_t* link2 = (const link_t*)data2;
    return memcmp(&link1->key, &link2->key, sizeof(link_key_t));
}

void
_link_value_slab_gc(link_value_t* val, slab_t* slab) {
    if (val && slab) {
        if (val->freelst) {
            slab->next = val->freelst;
            val->freelst = slab;
        } else {
            slab->next = NULL;
            val->freelst = slab;
        }
    }
}

slab_t*
_link_value_slab_alloc(link_value_t* val) {
    if (val && val->freelst) {
        slab_t* get = val->freelst;
        val->freelst = val->freelst->next;
        memset(get, 0, sizeof(slab_t));
        return get;
    }
    return (slab_t*)calloc(sizeof(slab_t), 1);
}

int
link_create() {
    g_links = hash_create(_link_hash, _link_cmp, GAZE_MAX_LINK_NUM);
    return g_links ? 0 : -1;
}

link_value_t*
link_find(link_key_t* key) {
    link_t link;
    link.key = *key;
    void* dst = hash_find(g_links, &link);
    if (dst) {
        return &((link_t*)dst)->value;
    }
    return NULL;
}

link_value_t*
link_insert(link_key_t* key) {
    link_t* link = (link_t*)calloc(sizeof(link_t), 1);
    link->key = *key;
    if (hash_insert(g_links, link)) {
        free(link);
        return NULL;
    }
    return &link->value;
}

#define PKG_SEND 0
#define PKG_RECV 1

void
link_key_init(link_key_t* key, uint32_t sip, uint32_t dip, uint16_t sport, uint16_t dport) {
    if (is_local_address(sip) == 0) {
        key->local_ip = sip;
        key->peer_ip = dip;
        key->local_port = sport;
        key->peer_port = dport;
    } else {
        key->peer_ip = sip;
        key->local_ip = dip;
        key->peer_port = sport;
        key->local_port = dport;
    }
}

link_value_t*
link_find_insert(link_key_t* key, int is_send) {
    int flow = (is_send == 0 ? PKG_SEND : PKG_RECV);
    link_value_t* val = link_find(key);
    if (!val) {
        val = link_insert(key);
        if (!val) {
            return NULL;
        }

        g_build_hook(key);

        memset(val, 0, sizeof(link_value_t));
    }
    val->flow = flow;
    return val;
}

void
_link_release(void* data, void* args) {
    link_t* dst = (link_t*)data;
    if (dst) {
        slab_t* tmp;
        while (dst->value.send) {
            tmp = dst->value.send;
            dst->value.send = dst->value.send->next;
            _link_value_slab_gc(&dst->value, tmp);
        }
        while (dst->value.recv) {
            tmp = dst->value.recv;
            dst->value.recv = dst->value.recv->next;
            _link_value_slab_gc(&dst->value, tmp);
        }
        while (dst->value.freelst) {
            tmp = dst->value.freelst;
            dst->value.freelst = dst->value.freelst->next;
            free(tmp);
        }
        free(dst);
    }
}

void
link_erase(link_key_t* key) {
    link_t link;
    link.key = *key;
    link_t* dst = (link_t*)hash_find(g_links, &link);
    if (dst) {

        g_finish_hook(key);

        hash_remove(g_links, &link);
        _link_release(dst, NULL);
    }
}

void
link_release() {
    if (g_links) {
        hash_loop(g_links, _link_release, NULL);
        hash_release(g_links);
        g_links = NULL;
    }
}

void
link_value_on_seq(link_value_t* val, uint32_t seq) {
    if (val && val->flow == PKG_SEND && val->start_send_seq == 0) {
        val->start_send_seq = seq;
        PRINTF("\tS = %u\n", seq);
    }
    if (val && val->flow == PKG_RECV && val->start_recv_seq == 0) {
        val->start_recv_seq = seq;
        PRINTF("\tR = %u\n", seq);
    }

    if (val->flow == PKG_SEND) {
        PRINTF("\tSEQ[S + %u] \n", seq - val->start_send_seq);
    } else {
        PRINTF("\tSEQ[R + %u] \n", seq - val->start_recv_seq);
    }
}
//打印tcppayload*************************************************************
void
_link_value_on_ack_notify(link_key_t* key, link_value_t* val, uint32_t ack) {


    //int protocol_based_tcp =1;

    slab_t* slab = (val->flow == PKG_SEND ? val->recv : val->send);
    while (slab) {
        if (val->flow == PKG_SEND) {
            PRINTF("\tacked recv slice[%d]\n", slab->slice.offset);
        } else {
            PRINTF("\tacked send slice[%d]\n", slab->slice.offset);
        }
        if (ack == slab->slice.seq + slab->slice.offset) {
            break;
        }
        slab = slab->next;
    }

    if (!slab) return;

    slab_t* from;
    if (val->flow == PKG_SEND) {
        from = val->recv;
        val->recv = slab->next;
    } else {
        from = val->send;
        val->send = slab->next;
    }
    slab->next = 0;
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
    while (from) {
        if (val->flow == PKG_SEND) {
            g_recv_hook(key, from->slice.buffer, from->slice.offset);

            //protocol_justfy(protocol_based_tcp);  <<<<<----------------这里

            //PRINTF("send\n");
        } else {
            g_recv_hook(key, from->slice.buffer, from->slice.offset);
            //PRINTF("receive\n");
        }
    ///////////////////////////////////*********************//////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////
        slab_t* tmp = from;
        from = from->next;
        _link_value_slab_gc(val, tmp);
    }
}

void
link_value_on_ack(link_key_t* key, link_value_t* val, uint32_t ack) {
    if (val->flow == PKG_SEND) {
        val->acked_recv_seq = ack;
        if (val->start_recv_seq == 0) {
            PRINTF("\tACK[%u]\n", ack);
        } else {
            PRINTF("\tACK[R + %u]\n", ack - val->start_recv_seq);
        }

        if (val->recv_fin_seq > 0 && ack >= val->recv_fin_seq) {
            val->send_fin_ack = ack;
        }

    } else {

        val->acked_send_seq = ack;
        if (val->start_send_seq == 0) {
            PRINTF("\tACK[%u]\n", ack);
        } else {
            PRINTF("\tACK[S + %u]\n", ack - val->start_send_seq);
        }

        if (val->send_fin_seq > 0 && ack >= val->send_fin_seq) {
            val->recv_fin_ack = ack;
        }
    }
    _link_value_on_ack_notify(key, val, ack);
}

void
link_value_on_fin(link_value_t* val, uint32_t seq) {
    PRINTF("\tFIN\n");
    if (val->flow == PKG_SEND) {
        val->send_fin_seq = seq;
    } else {
        val->recv_fin_seq = seq;
    }
}

void
link_value_on_psh(link_value_t* val, uint32_t seq, int bytes, const char* data) {
    if (!val || !data || bytes <= 0) return;
    slab_t* slab;
    slab = (val->flow == PKG_SEND ? val->send : val->recv);
    if (!slab) {
        slab = _link_value_slab_alloc(val);
        if (val->flow == PKG_SEND) {
            val->send = slab;
        } else {
            val->recv = slab;
        }
    } else {
        // insert by ascending
        while (slab->next && slab->next->slice.seq < seq) {
            slab = slab->next;
        }
        slab_t* newslab = _link_value_slab_alloc(val);
        if (slab->next) {
            newslab->next = slab->next;
        }
        slab->next = newslab;
        slab = slab->next;
    }
    slab->slice.seq = seq;
    slab->slice.offset = bytes;
    memcpy(slab->slice.buffer, data, bytes);
}

int
link_value_is_finish(struct link_value_t* val) {
    if (val && val->send_fin_seq > 0 && val->recv_fin_seq > 0
        && val->recv_fin_ack >= val->send_fin_seq
        && val->send_fin_ack >= val->recv_fin_seq) {
        return 0;
    }
    return -1;
}

