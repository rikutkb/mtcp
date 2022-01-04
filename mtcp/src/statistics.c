#include "statistics.h"
#include "mtcp_api.h"

#include "debug.h"
#include "fhash.h"
#include "tcp_stream.h"
#include "tcp_in.h"
#include "logger.h"
#include "mtcp_api.h"
#include "eventpoll.h"
#include "debug.h"

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
	
	idx = ht->hashfn(item->ip);
	assert(idx >=0 && idx < NUM_BINS_FLOWS);

	TAILQ_INSERT_TAIL(&ht->ht_table[idx], item, links);
	
	return 0;
}
void* IPHTRemove(struct ip_hashtable *ht, void *it){

	hash_ip_head *head;
	ip_statistic *item = (ip_statistic *)it;
	int idx = ht->hashfn(item->ip);

	head = &ht->ht_table[idx];
	TAILQ_REMOVE(head, item, links);	

	return (item);
}
void *IPHTSearch(struct ip_hashtable *ht, uint32_t ip){
	int idx;
	ip_statistic it;
	it.ip=ip;
	const ip_statistic *item = &it;
	ip_statistic *walk;
	hash_ip_head *head;
	idx = ht->hashfn(item->ip);
	head = &ht->ht_table[idx];
	TAILQ_FOREACH(walk, head, links) {
		if (walk->ip==ip){
			return walk;
		}
	}
	UNUSED(idx);
	return NULL;
}


int JudgeDropbyIp(struct ip_hashtable *ht,uint32_t saddr,int is_attacking){//if drop return 0
	//if is attacking
	struct ip_statistic *cur_ip_stat = NULL;
	struct ip_statistic ip_stat;
	ip_stat.ip = saddr;
	if (!(cur_ip_stat = IPHTSearch(ht, saddr))) {// not in hashtable
		return 3;
	}
	cur_ip_stat->pps++;
	cur_ip_stat->send_packet_sum++;

	if(is_attacking){
		if(cur_ip_stat->pps>10000&&!cur_ip_stat->priority==0){
			cur_ip_stat->priority=0;
			return -10;
		}
		switch(cur_ip_stat->priority){
			case 4://0%
				return 1;
			case 3://20%
				return 1;
			case 2://10%
				return rand()%10+1<=9?1:-2;
			case 1://60%
				return rand()%10+1<=8?1:-1;
			case 0://100%
				return rand()%10+1<=5?1:-0;
		}


	}else{
		return 1;
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

	return hash & (NUM_BINS_IPS - 1);

}
ip_statistic* CreateIPStat(mtcp_manager_t mtcp, uint32_t ip){
	ip_statistic *ip_stat = NULL;
	pthread_mutex_lock(&mtcp->ctx->ip_pool_lock);
	ip_stat = (ip_statistic *)MPAllocateChunk(mtcp->ip_pool);

	if(!ip_stat){
		TRACE_ERROR("Cannot allocate memory for the ip_stat. "
				"CONFIG.max_concurrency: %d, created: %u\n", 
				CONFIG.max_concurrency, mtcp->created_ip);
		pthread_mutex_unlock(&mtcp->ctx->ip_pool_lock);
		return NULL;
	}
	mtcp->created_ip++;
	memset(ip_stat, 0, sizeof(ip_statistic));
	ip_stat->ip = ip;
	ip_stat->priority = 3;
	ip_stat->packet_recv_num = 0;
	ip_stat->send_packet_sum = 0;
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
	if (!(cur_ip_stat = IPHTSearch(ht, saddr))) {
		TRACE_INFO("create ip stat");
		cur_ip_stat = CreateIPStat(mtcp,saddr);
		return;
	}
	cur_ip_stat->packet_recv_num++;

	if(cur_ip_stat->pps>ATTACKER_TH_1){
		if(cur_ip_stat->pps>ATTACKER_TH_2){
			cur_ip_stat->priority=0;
			TRACE_INFO("fast drop");
			return;
		}
		cur_ip_stat->priority=1;
	}
}
void get_p_list(struct ip_hashtable *ht,int p_list[5]){
	ip_statistic *walk;

	for (int i = 0; i < ht->bins; i++){
		TAILQ_FOREACH(walk, &ht->ht_table[i], links) {
			if(walk->send_packet_sum>0){
				p_list[walk->priority]++;

			}
		}
	}

}
int get_average(struct ip_hashtable *ht, statistic *stat_ave){
	uint32_t valid_ips=0;
	ip_statistic *walk;
	stat_ave->packet_recv_num=0;
	for (int i = 0; i < ht->bins; i++){
		TAILQ_FOREACH(walk, &ht->ht_table[i], links) {
			if(walk->send_packet_sum>0){
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
	return valid_ips;
}
void get_dispresion(struct ip_hashtable *ht, statistic stat_ave, statistic *stat_dis, int ips){
	statistic stat_sum;
	int valid_ips = 0;
	ip_statistic *walk;
	stat_dis->packet_recv_num=0;
	stat_sum.packet_recv_num=0;
	
	for (int i = 0; i < ht->bins; i++){
		TAILQ_FOREACH(walk, &ht->ht_table[i], links) {
			if(walk->packet_recv_num>0){
				int diff=(int)walk->packet_recv_num- (int)stat_ave.packet_recv_num;

				stat_sum.packet_recv_num+=(POW2(diff)/ips);
			}
		}
	}
	stat_dis->packet_recv_num=(uint32_t)sqrt(stat_sum.packet_recv_num);


    //ホワイトリストのスループットをリセット

}

int get_statistics(struct ip_hashtable *ht,statistic *stat_ave, statistic *stat_dis){
	int valid_ips=get_average(ht,stat_ave);
    if(valid_ips>2){
		get_dispresion(ht,*stat_ave,stat_dis,valid_ips);

	}
	return valid_ips;

}
void get_moving_statistics(uint32_t stat_cal_times,statistic ave_arr[], statistic dis_arr[],statistic *ave_cur,statistic *dis_cur){
	int times = MIN(stat_cal_times,MOVING_AVE_TIMES);
	ave_cur->packet_recv_num=0;
	dis_cur->packet_recv_num=0;
	for(int i=0;i<times;i++){
		ave_cur->packet_recv_num+=ave_arr[i].packet_recv_num;
		dis_cur->packet_recv_num+=dis_arr[i].packet_recv_num;
	}
	ave_cur->packet_recv_num/=MOVING_AVE_TIMES;
	dis_cur->packet_recv_num/=MOVING_AVE_TIMES;
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
				walk->priority=0;
			}else if(walk->packet_recv_num > MIN(THROUGHPUT_TH,stat_ave.packet_recv_num+stat_dis.packet_recv_num)){
				walk->priority=walk->priority-2>=0?walk->priority-2:0;
			}else if(walk->packet_recv_num<stat_ave.packet_recv_num){
				if(walk->priority<MAX_PRIORITY){
					walk->priority++;
				}
			}
			walk->packet_recv_num = 0;
			walk->send_packet_sum = 0;


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