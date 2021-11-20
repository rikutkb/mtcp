
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

#include "statistics.h"

void AddedPacketStatistics(struct hashtable *ht,iph->s_addr,len){

}

void get_average(struct hashtable *ht){
    //全てのホワイトリストのパケット数、スループットを計算
    //ホワイトリストのスループットをリセット

}
void get_dispresion(struct hashtable *ht){

}
void update_priority(struct hashtable *ht){

}
void get_statistics(mtcp_manager mtcp){
    uint32_t packet_ave,throughput_ave;
    get_average();
    uint32_t packet_dis,throughput_dis;
    get_dispresion();
    update_priority();
}

bool JudgeDropbyIp(struct hashtable *ht,u_int32_t s_addr){
    return false;
}
unsigned int
IPHashFlow(const void *saddr)
{
    tcp_stream *saddr = (ip_addr * )saddr;
	unsigned int hash, i;
	char *key = (char *)&saddr;

	for (hash = i = 0; i < 12; ++i) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash & (NUM_BINS_FLOWS - 1);
#endif
}

int EqualIP(const void *ip1, const void *ip2)
{
	ip_addr *ip_1 = (ip_addr *)ip1;
	ip_addr *ip_2 = (ip_addr *)ip2;

	return (*ip_1==*ip_2);
}
