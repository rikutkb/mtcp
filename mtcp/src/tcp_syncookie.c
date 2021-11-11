
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
#include "siphash.h"
#include "tcp_syncookie.h"

static siphash_key_t syncookie_secret[2];//readmostlyにする
#define COOKIEBITS 24	/* Upper bits store count */
#define COOKIEMASK (((uint32_5)1 << COOKIEBITS) - 1)


//tcp.h
static uint32_t tcp_cookie_time(void){
    uint64_t val = get_jiffies_64();
    //do_div(val,TCP_SYNCOOKIE_PERIOD);
    return val;
}

//tcp.h
static uint32_t cookie_hash(uint32_t saddr, uint32_t daddr, uint16_t sport, uint16_t dport, uint32_t count, int c){
    return siphash_4u32(saddr,daddr,sport<<16|dport,count,&syncookie_secret[c]);
}

uint64_t cookie_init_timestamp(uint64_t now){

}

void get_statistics(mtcp_manager mtcp){

}

static uint32_t check_tcp_syn_cookie(uint32_t cookie, uint32_t saddr, uint32_t daddr, uint16_t sport, uint16_t dport,uint32_t sseq){
    uint32_t diff, count = tcp_cookie_time();
    cookie -= cookie_hash(saddr,daddr,sport,dport,0,0)+ sseq;
    diff = (count -(cookie >> COOKIEBITS)) & ((uint32_t)-1 >>COOKIEBITS);
    if(diff >=MAX_SYNCOOKIE_AGE){
        return (uint32_t)-1;
    }
    return (cookie - cookie_hash(saddr,daddr,sport,dport,count-diff,1)& COOKIEMASK);
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

bool IpHTSearch(struct hashtable *ht,const void *it){
	int idx;
	const tcp_stream *item = (const tcp_stream *)it;
	tcp_stream *walk;
	hash_bucket_head *head;

	idx = ht->hashfn(item);

	head = &ht->ht_table[ht->hashfn(item)];
	TAILQ_FOREACH(walk, head, rcvvar->he_link) {
		if(walk->saddr == item->saddr){
			return true;
		}
	}

	UNUSED(idx);
	return false;
// }