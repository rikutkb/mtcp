
#include <assert.h>
#include <time.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
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
static inline ip_statistic * CreateNewIpEntry(mtcp_manager_t mtcp, ip_addr s_addr){
	ip_statistic *cur_ip_stat;
	//do alloc memory
	cur_ip_stat->ip = saddr;
	cur_ip_stat->priority  = 1;
	return cur_ip_stat;

}
bool JudgeDropbyIp(struct hashtable *ht,ip_addr saddr){
	//if is attacking
	ip_statistic *cur_ip_stat = NULL;
	ip_statistic ip_stat;
	ip_stat.ip = saddr;
	if (!(cur_ip_stat = IpHTSearch(ht, &ip_stat))) {// not in hashtable
		return true;
	}
	switch(ip_stat->priority){
		case 2:
			return true;
		case 1://10%
			return rand()%10+1<=1;
		case 0://40%
			return rand()%10+1<=4;
	}
	
}
int EqualIP(const void *ip1, const void *ip2)
{
	ip_statistic *ip_1 = (ip_statistic *)ip1;
	ip_statistic *ip_2 = (ip_statistic *)ip2;

	return (ip_1->ip==ip_2->ip);
}
unsigned int IPHashFlow(const void *saddr)
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

}
void AddedPacketStatistics(struct hashtable *ht,ip_addr saddr,int ip_len){
	ip_statistic *cur_ip_stat = NULL;
	ip_statistic ip_stat;
	ip_stat.ip = saddr;
	if (!(cur_ip_stat = IpHTSearch(ht, &ip_stat))) {
		cur_ip_stat = CreateNew
		if(!cur_ip_stat){
			printf("cannot created ht");
			return;
		}
	}

}

statistic get_average(struct hashtable *ht){
	statistic stat_ave;

	for (int i = 0; i < bins; i++){

	}
    //全てのホワイトリストのパケット数、スループットを計算
	return stat_ave;
}
statistic get_dispresion(struct hashtable *ht, statistics stat_ave){
	statistic stat_sum;
	int valid_ips = 0;
	for (int i = 0; i < bins; i++){

	}
	if(valid_ips>0){
		stat_sum.packet_recv_num /= valid_ips;
		stat_sum.packet_recv_num = sqrt(stat_sum.packet_recv_num);

	}else{
	}

	return stat_sum;

    //ホワイトリストのスループットをリセット

}

void get_statistics(mtcp_manager mtcp,statistic stat_dis){
    statistic stat_ave;
    stat_ave = get_average(mtcp->ip_stat_table);
	statistic stat_dis;
    stat_dis = get_dispresion(mtcp->ip_stat_table,stat_ave);
    update_priority(mtcp->ip_stat_table,stat_dis);
}
void * IpHTSearch(struct hashtable *ht, const void *it){
	int idx;
	const ip_statistic *item = (const ip_statistic *)it;
	ip_statistic *walk;
	hash_bucket_head *head;

	idx = ht->hashfn(item);

	head = &ht->ht_table[ht->hashfn(item)];
	TAILQ_FOREACH(walk, head, links) {
		if (ht->eqfn(walk, item)) 
			return walk;
	}

	//UNUSED(idx);
	return NULL;
}

void update_priority(struct hashtable *ht, statistic stat_ave, statistic stat_dis){
	for (int i = 0; i < bins; i++){//do all parameter
		if(ip_statistic->packet_recv_num > max(throughput_def,stat_ave->packet_recv_num+stat_dis->packet_recv_num*2)){
			ip_statistic->priority--;
			ip_statistic->packet_recv_num = 0;
		}
	}
}


