
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
#include "tcp_stream.h"
#include "siphash.h"
#include "tcp_syncookie.h"
static siphash_key_t syncookie_secret[1] = { 0x0706050403020100ULL };
#define COOKIEBITS 24	/* Upper bits store count */
#define COOKIEMASK (((uint32_5)1 << COOKIEBITS) - 1)
#define HZ 1000
#define TCP_SYNCOOKIE_PERIOD	(60)

uint32_t tcp_cookie_time(void){
    uint32_t time_ = time(NULL);
	return time_/TCP_SYNCOOKIE_PERIOD;
}

uint32_t cookie_hash(uint32_t saddr, uint32_t daddr, uint16_t sport, uint16_t dport, uint32_t count, int c){
    return siphash_4u32(saddr,daddr,sport<<16|dport,count,&syncookie_secret[c]);
}
// static uint32_t check_tcp_syn_cookie(uint32_t cookie, uint32_t saddr, uint32_t daddr, uint16_t sport, uint16_t dport,uint32_t sseq){
//     uint32_t diff, count = tcp_cookie_time();
//     cookie -= cookie_hash(saddr,daddr,sport,dport,0,0)+ sseq;
//     diff = (count -(cookie >> COOKIEBITS)) & ((uint32_t)-1 >>COOKIEBITS);
//     if(diff >=MAX_SYNCOOKIE_AGE){
//         return (uint32_t)-1;
//     }
//     return (cookie - cookie_hash(saddr,daddr,sport,dport,count-diff,1)& COOKIEMASK);
// }


int IpHTSearch(struct hashtable *ht,const void *it){
	int idx;
	const tcp_stream *item = (const tcp_stream *)it;
	tcp_stream *walk;
	hash_bucket_head *head;

	idx = ht->hashfn(item);

	head = &ht->ht_table[ht->hashfn(item)];
	TAILQ_FOREACH(walk, head, rcvvar->he_link) {
		if(walk->saddr == item->saddr){
			return 1;
		}
	}

	UNUSED(idx);
	return 0;
 }

tcp_stream* CreateNewFlowHTEntry_SC(mtcp_manager_t mtcp, uint32_t cur_ts, const struct iphdr *iph, 
		int ip_len, const struct tcphdr* tcph, uint32_t seq, uint32_t ack_seq,
		int payloadlen, uint16_t window)
{

	//cur_stream = HandlePassiveOpen(mtcp,cur_ts, iph, tcph, seq, window);
	tcp_stream *cur_stream = NULL;
	/* create new stream and add to flow hash table */
	cur_stream = CreateTCPStream(mtcp, NULL, MTCP_SOCK_STREAM, 
			iph->daddr, tcph->dest, iph->saddr, tcph->source);
	if (!cur_stream) {
		TRACE_ERROR("INFO: Could not allocate tcp_stream!\n");
		return NULL;
	}
	// cur_stream->rcvvar->irs = seq;
	// cur_stream->sndvar->peer_wnd = window;
	// cur_stream->rcv_nxt = cur_stream->rcvvar->irs;
	// cur_stream->sndvar->cwnd = 1;
	//ParseTCPOptions(cur_stream, cur_ts, (uint8_t *)tcph + TCP_HEADER_LEN, (tcph->doff << 2) - TCP_HEADER_LEN);

	return cur_stream;

	
}