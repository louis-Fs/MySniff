#ifndef LINK_H_
#define LINK_H_

#include "gaze.h"
#include "hash.h"


extern int global_flag;

struct link_value_t;

#define GAZE_MAX_LINK_NUM 19997
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



int link_create();
struct link_value_t* link_find(link_key_t*);
struct link_value_t* link_insert(link_key_t*);
struct link_value_t* link_find_insert(link_key_t*, int is_send);
void link_erase(link_key_t*);
void link_release();

void link_key_init(link_key_t* key, uint32_t sip, uint32_t dip, uint16_t sport, uint16_t dport);

void link_value_on_seq(struct link_value_t*, uint32_t seq);
void link_value_on_ack(link_key_t*, struct link_value_t*, uint32_t ack);
void link_value_on_psh(struct link_value_t*, uint32_t seq, int bytes, const char*);
void link_value_on_fin(struct link_value_t*, uint32_t seq);

int link_value_is_finish(struct link_value_t*);

#endif

