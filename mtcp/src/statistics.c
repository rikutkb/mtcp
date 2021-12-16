#include "statistics.h"
#include "debug.h"
#include "fhash.h"
#include "tcp_stream.h"
#include "tcp_in.h"
#if defined(USE_DDOSPROT)

struct ip_hashtable *CreateIPHashtable(unsigned int (*hashfn) (const void *), int (*eqfn) (const void *, const void *),int bins){
	int i;
	struct ip_hashtable* ht = calloc(1, sizeof(struct ip_hashtable));
	if (!ht){
		TRACE_ERROR("calloc: CreateIPHashtable");
		return 0;
	}

	ht->hashfn = hashfn;
	ht->eqfn = eqfn;
	ht->bins = bins;
	/* creating bins */
	if (IS_IP_TABLE(hashfn)) {
		ht->ht_table = calloc(bins, sizeof(hash_ip_head));
		if (!ht->ht_table) {
			TRACE_ERROR("calloc: CreateHashtable bins!\n");
			free(ht);
			return 0;
		}
		/* init the tables */
		for (i = 0; i < bins; i++)
			TAILQ_INIT(&ht->ht_table[i]);
	} 
	return ht;
}
void DestroyIPHashtable(struct ip_hashtable *ht){
	if (IS_IP_TABLE(ht->hashfn))
		free(ht->ht_table);
	else /* IS_LISTEN_TABLE(ht->hashfn) */
		free(ht->lt_table);
	free(ht);
}


int IPHTInsert(struct ip_hashtable *ht, void *it){
	/* create an entry*/ 
	int idx;
	ip_statistic *item = (ip_statistic *)it;

	assert(ht);

	idx = ht->hashfn(item);
	assert(idx >=0 && idx < NUM_BINS_FLOWS);

	TAILQ_INSERT_TAIL(&ht->ht_table[idx], item, links);
	
	return 0;
}
void* IPHTRemove(struct ip_hashtable *ht, void *it){

	hash_ip_head *head;
	ip_statistic *item = (ip_statistic *)it;
	int idx = ht->hashfn(item);

	head = &ht->ht_table[idx];
	TAILQ_REMOVE(head, item, links);	

	return (item);
}
void *IPHTSearch(struct ip_hashtable *ht, const void *it){
	int idx;
	const ip_statistic *item = (const ip_statistic *)it;
	ip_statistic *walk;
	hash_ip_head *head;
	idx = ht->hashfn(item);
	head = &ht->ht_table[ht->hashfn(item)];
	TAILQ_FOREACH(walk, head, links) {
		if (ht->eqfn(walk, item)){
			return walk;
		}
	}
	UNUSED(idx);
	return NULL;
}


int JudgeDropbyIp(struct ip_hashtable *ht,uint32_t saddr){//if drop return 0
	//if is attacking
	struct ip_statistic *cur_ip_stat = NULL;
	struct ip_statistic ip_stat;
	ip_stat.ip = saddr;
	if (!(cur_ip_stat = IPHTSearch(ht, &ip_stat))) {// not in hashtable
		return 0;
	}
	switch(cur_ip_stat->priority){
		case 4://1%
			return rand()%100+1<=99?1:-4;
		case 3://10%
			return rand()%10+1<=9?1:-3;
		case 2://30%
			return rand()%10+1<=8?1:-2;
		case 1://50%
			return rand()%10+1<=5?1:-1;
		case 0://90%
			return rand()%10+1<=1?1:0;
	}
	
}
int EqualIP(const void *ip1, const void *ip2){
	struct ip_statistic *ip_1 = (const ip_statistic *)ip1;
	struct ip_statistic *ip_2 = (const ip_statistic *)ip2;

	return (ip_1->ip==ip_2->ip);
}
unsigned int IPHash(const void *saddr){
    uint32_t *saddr_ = (uint32_t *)saddr;
	unsigned int hash, i;
	char *key = (char *)&saddr_;

	for (hash = i = 0; i < 12; ++i) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash & (3 - 1);

}
ip_statistic* CreateIPStat(mtcp_manager_t mtcp, uint32_t ip){
	ip_statistic *ip_stat = NULL;
	pthread_mutex_lock(&mtcp->ctx->ip_pool_lock);
	ip_stat = (ip_statistic *)MPAllocateChunk(mtcp->ip_pool);
	if(!ip_stat){
		TRACE_ERROR("Cannot allocate memory for the ip_stat. "
				"CONFIG.max_concurrency: %d, concurrent: %u\n", 
				CONFIG.max_concurrency, mtcp->flow_cnt);
		pthread_mutex_unlock(&mtcp->ctx->ip_pool_lock);
		return NULL;
	}
	memset(ip_stat, 0, sizeof(ip_statistic));
	ip_stat->ip = ip;
	ip_stat->priority = 2;
	ip_stat->packet_recv_num = 0;
	ip_stat->throughput_send_num=0;
	ip_stat->packet_rtt=0;
	int ret = IPHTInsert(mtcp->ip_stat_table, ip_stat);
	if(ret<0){
		TRACE_ERROR("ip %d: "
				"Failed to insert the ip into hash table.\n", ip_stat->ip);
		MPFreeChunk(mtcp->ip_pool, ip_stat);
		pthread_mutex_unlock(&mtcp->ctx->ip_pool_lock);
		return NULL;
	}
	pthread_mutex_unlock(&mtcp->ctx->ip_pool_lock);
	return ip_stat;

}
void AddedPacketStatistics(mtcp_manager_t mtcp, struct ip_hashtable *ht,uint32_t saddr,int ip_len){
	ip_statistic *cur_ip_stat = NULL;
	ip_statistic ip_stat;
	ip_stat.ip = saddr;
	if (!(cur_ip_stat = IPHTSearch(ht, &ip_stat))) {
		TRACE_INFO("create ip stat");
		cur_ip_stat = CreateIPStat(mtcp,saddr);
	}
	cur_ip_stat->packet_recv_num++;
	cur_ip_stat->pps++;
	if(cur_ip_stat->pps>ATTACKER_TH_1){
		if(cur_ip_stat->pps>ATTACKER_TH_2){
			cur_ip_stat->priority=0;
			TRACE_INFO("fast drop");
			return;
		}
		cur_ip_stat->priority=1;
	}
}

int get_average(struct ip_hashtable *ht, statistic *stat_ave){
	uint8_t valid_ips=0;
	ip_statistic *walk;
	for (int i = 0; i < ht->bins; i++){
		TAILQ_FOREACH(walk, &ht->ht_table[i], links) {
			if(walk->packet_recv_num>=0){
				TRACE_INFO("packet recv iiiiii%d",stat_ave->packet_recv_num);
				valid_ips++;
				stat_ave->packet_recv_num+=walk->packet_recv_num;

			}
		}
	}
	TRACE_INFO("valid ip num is %d",valid_ips);
	if(valid_ips>0){
		TRACE_INFO("packet rec ave is %d",stat_ave->packet_recv_num);

		stat_ave->packet_recv_num/=valid_ips;
		TRACE_INFO("packet rec ave is %d",stat_ave->packet_recv_num);
	}else{
		TRACE_INFO("cannto get ave");
		return 0;
	}
    //全てのホワイトリストのパケット数、スループットを計算
	return 1;
}
int get_dispresion(struct ip_hashtable *ht, statistic stat_ave, statistic *stat_dis){
	statistic stat_sum;
	int valid_ips = 0;
	ip_statistic *walk;

	for (int i = 0; i < ht->bins; i++){
		TAILQ_FOREACH(walk, &ht->ht_table[i], links) {
			if(walk->packet_recv_num>0){
				valid_ips++;
				stat_sum.packet_recv_num+=POW2(walk->packet_recv_num - stat_ave.packet_recv_num);
			}
		}
	}
	if(valid_ips>1){
		stat_sum.packet_recv_num/=(valid_ips-1);
		stat_dis->packet_recv_num = (uint8_t)sqrt(stat_sum.packet_recv_num);
	}else if(valid_ips==1){
		return 1;
	}else{
		return 0;
	}

	return 1;

    //ホワイトリストのスループットをリセット

}

void get_statistics(struct ip_hashtable *ht){
    statistic stat_ave;
    if(get_average(ht,&stat_ave)){
		statistic stat_dis;
		if(get_dispresion(ht,stat_ave,&stat_dis)){
			TRACE_INFO("average is %d, dispression is %d",stat_ave.packet_recv_num,stat_dis.packet_recv_num);
			update_priority(ht,stat_ave,stat_dis);
			return;
		}else{
			TRACE_INFO("failed to get stat");
		}
	}else{
		TRACE_INFO("failed to get stat");

	}

}

void reset_ip_pps(struct ip_hashtable *ht){
	ip_statistic *walk;

	for (int i = 0; i < ht->bins; i++){
		TAILQ_FOREACH(walk, &ht->ht_table[i], links) {
			walk->pps = 0;
		}
	}
}
void update_priority(struct ip_hashtable *ht, statistic stat_ave, statistic stat_dis){
	ip_statistic *walk;
	TRACE_INFO("average is %d, dispression is %d",stat_ave.packet_recv_num,stat_dis.packet_recv_num);
	for (int i = 0; i < ht->bins; i++){
		TAILQ_FOREACH(walk, &ht->ht_table[i], links) {
			if(walk->packet_recv_num > MIN(THROUGHPUT_TH,stat_ave.packet_recv_num+stat_dis.packet_recv_num*2)){
				if(walk->priority>0){
					walk->priority--;
				}
			}else{
				if(walk->priority<MAX_PRIORITY){
					walk->priority++;
				}
			}
			walk->packet_recv_num = 0;


		}
		
	}
}

void ProcessRstTCPPacket(mtcp_manager_t mtcp, const struct iphdr *iph, uint32_t cur_ts, 
const struct tcphdr *tcph, uint32_t seq, int payloadlen ){
	TRACE_INFO("droped packet");

	if (tcph->syn && !tcph->ack) {
		SendTCPPacketStandalone(mtcp, 
		iph->daddr, tcph->dest, iph->saddr, tcph->source, 
		0, seq + payloadlen + 1, 0, TCP_FLAG_RST | TCP_FLAG_ACK, 
		NULL, 0, cur_ts, 0);
	}else if(tcph->rst){
		return ;
	}else if(tcph->ack){
		uint32_t ack_seq = ntohl(tcph->ack_seq);
		SendTCPPacketStandalone(mtcp, 
		iph->daddr, tcph->dest, iph->saddr, tcph->source, 
		ack_seq, 0, 0, TCP_FLAG_RST, NULL, 0, cur_ts, 0);
	}else{
		SendTCPPacketStandalone(mtcp, 
		iph->daddr, tcph->dest, iph->saddr, tcph->source, 
		0, seq + payloadlen, 0, TCP_FLAG_RST | TCP_FLAG_ACK, 
		NULL, 0, cur_ts, 0);
	}



}


#endif