
#ifdef SYNCOOKIE_H
#define SYNCOOKIE_H
#define USE_SYNCOOKIE true

#include "mtcp.h"
void 
ParseSYNTCPOptions(tcp_stream *cur_stream, 
		uint32_t cur_ts, uint8_t *tcpopt, int len);

extern inline int 
ProcessSYNTCPUplink(mtcp_manager_t mtcp, uint32_t cur_ts, tcp_stream *cur_stream, 
		const struct tcphdr *tcph, uint32_t seq, uint32_t ack_seq, 
		uint8_t *payload, int payloadlen, uint32_t window);

int
ProcessSYNTCPPacket(struct mtcp_manager *mtcp, uint32_t cur_ts, const int ifidx,
					const struct iphdr* iph, int ip_len);
uint16_t 
SYNTCPCalcChecksum(uint16_t *buf, uint16_t len, uint32_t saddr, uint32_t daddr);


#endif