
#include <assert.h>
#include <time.h>
#include <inttypes.h>

#include "tcp_util.h"
#include "tcp_in.h"
#include "tcp_out.h"
#include "tcp_ring_buffer.h"
#include "eventpoll.h"
#include "debug.h"
#include "timer.h"
#include "ip_in.h"
#include "clock.h"
#include "tcp_syncookie.h"
#if USE_CCP
#include "ccp.h"
#endif

static uint32_t cookie_hash(uint32_t saddr, uint32_t daddr, uint16_t sport, uint16_t dport, uint32_t count, int c){
    
}

uint64_t cookie_init_timestamp(uint64_t now){

}
void 
ParseSYNTCPOptions(tcp_stream *cur_stream, 
		uint32_t cur_ts, uint8_t *tcpopt, int len){


        }

extern inline int 
ProcessSYNTCPUplink(mtcp_manager_t mtcp, uint32_t cur_ts, tcp_stream *cur_stream, 
		const struct tcphdr *tcph, uint32_t seq, uint32_t ack_seq, 
		uint8_t *payload, int payloadlen, uint32_t window){

        }

int
ProcessSYNTCPPacket(struct mtcp_manager *mtcp, uint32_t cur_ts, const int ifidx,
					const struct iphdr* iph, int ip_len){

                    }
uint16_t 
SYNTCPCalcChecksum(uint16_t *buf, uint16_t len, uint32_t saddr, uint32_t daddr){

}
