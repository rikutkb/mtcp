
#ifndef SYNCOOKIE_H
#define SYNCOOKIE_H

#include "mtcp.h"
//static uint32_t check_tcp_syn_cookie(uint32_t cookie, uint32_t saddr, uint32_t daddr, uint16_t sport, uint16_t dport,uint32_t sseq);
uint32_t tcp_cookie_time(void);
uint32_t cookie_hash(uint32_t saddr, uint32_t daddr, uint16_t sport, uint16_t dport, uint32_t count, int c);

int IpHTSearch(struct hashtable *ht,const void *it);


#endif