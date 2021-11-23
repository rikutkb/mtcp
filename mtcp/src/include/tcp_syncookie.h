
#ifndef SYNCOOKIE_H
#define SYNCOOKIE_H

#include "mtcp.h"
#include "tcp_stream.h"

uint32_t tcp_cookie_time(void);
uint32_t cookie_hash(uint32_t saddr, uint32_t daddr, uint16_t sport, uint16_t dport, uint32_t count, int c);

int IpHTSearch(struct hashtable *ht,const void *it);
tcp_stream* CreateNewFlowHTEntry_SC(mtcp_manager_t mtcp, uint32_t cur_ts, const struct iphdr *iph, 
		int ip_len, const struct tcphdr* tcph, uint32_t seq, uint32_t ack_seq,
		int payloadlen, uint16_t window);

#endif
