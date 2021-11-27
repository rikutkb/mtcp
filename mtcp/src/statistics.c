#include "statistics.h"
#include "debug.h"

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


int StreamIPHTInsert(struct ip_hashtable *ht, void *it){
	/* create an entry*/ 
	int idx;
	ip_statistic *item = (ip_statistic *)it;

	assert(ht);

	idx = ht->hashfn(item);
	assert(idx >=0 && idx < NUM_BINS_FLOWS);

	TAILQ_INSERT_TAIL(&ht->ht_table[idx], item, links);
	
	return 0;
}
void* StreamIPHTRemove(struct ip_hashtable *ht, void *it){

	hash_ip_head *head;
	ip_statistic *item = (ip_statistic *)it;
	int idx = ht->hashfn(item);

	head = &ht->ht_table[idx];
	TAILQ_REMOVE(head, item, links);	

	return (item);
}
void *StreamIPHTSearch(struct ip_hashtable *ht, const void *it){
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


struct ip_statistic* CreateNewIpEntry(mtcp_manager_t mtcp, uint32_t saddr){
	struct ip_statistic *cur_ip_stat;
	struct ip_statistic a;
	cur_ip_stat = &a;
	//do alloc memory
	cur_ip_stat->ip = saddr;
	cur_ip_stat->priority  = 1;
	return cur_ip_stat;

}
int JudgeDropbyIp(struct ip_hashtable *ht,uint32_t saddr){
	//if is attacking
	struct ip_statistic *cur_ip_stat = NULL;
	struct ip_statistic ip_stat;
	ip_stat.ip = saddr;
	if (!(cur_ip_stat = IpWhiteHTSearch(ht, &ip_stat))) {// not in hashtable
		return 0;
	}
	TRACE_INFO("found in ht");
	return 0;
	switch(cur_ip_stat->priority){
		case 2:
			return 0;
		case 1://10%
			return rand()%10+1>=1;
		case 0://40%
			return rand()%10+1>=4;
	}
	
}
int EqualIP(const void *ip1, const void *ip2)
{
	TRACE_INFO("equal");
	struct ip_statistic *ip_1 = (const ip_statistic *)ip1;
	struct ip_statistic *ip_2 = (const ip_statistic *)ip2;

	return (ip_1->ip==ip_2->ip);
}
unsigned int IPHash(const void *saddr)
{
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
void AddedPacketStatistics(struct ip_hashtable *ht,uint32_t saddr,int ip_len){
	ip_statistic *cur_ip_stat = NULL;
	ip_statistic ip_stat;
	ip_stat.ip = saddr;
	if (!(cur_ip_stat = IpWhiteHTSearch(ht, &ip_stat))) {
		//cur_ip_stat = CreateNew
		if(!cur_ip_stat){
			printf("cannot created ht");
			return;
		}
	}

}

int get_average(struct ip_hashtable *ht, statistic *stat_ave){
	uint8_t valid_ips=0;
	ip_statistic *walk;

	for (int i = 0; i < ht->bins; i++){
		TAILQ_FOREACH(walk, &ht->ht_table[i], links) {
			if(walk->packet_recv_num>0){
				stat_ave->packet_recv_num+=walk->packet_recv_num;
				valid_ips++;
			}
		}
	}
	if(valid_ips>0){
		stat_ave->packet_recv_num/=valid_ips;
	}else{
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
				stat_sum.packet_recv_num+=POW2(walk->packet_recv_num - stat_ave.packet_recv_num);
			}
		}
	}
	if(valid_ips>1){
		stat_sum.packet_recv_num/=(valid_ips-1);
		stat_dis->packet_recv_num = (uint8_t)sqrt(stat_sum.packet_recv_num);
	}else{
		return 0;
	}

	return 1;

    //ホワイトリストのスループットをリセット

}

void get_statistics(mtcp_manager_t mtcp){
    statistic stat_ave;
	TRACE_INFO("check statistcs avaiable");
    // if(get_average(mtcp->ip_stat_table,&stat_ave)){
	// 	statistic stat_dis;
	// 	if(get_dispresion(mtcp->ip_stat_table,stat_ave,&stat_dis)){
	// 		update_priority(mtcp->ip_stat_table,stat_ave,stat_dis);
	// 		return;
	// 	}
	// }
	//reset_priority???

}
void* IpWhiteHTSearch(struct ip_hashtable *ht, const void *it){
	int idx;

	const ip_statistic *item = (const ip_statistic *)it;

	ip_statistic *walk;
	hash_ip_head *head;
	idx = ht->hashfn(item);
	head = &ht->ht_table[ht->hashfn(item)];

	TAILQ_FOREACH(walk, head, links) {
		TRACE_INFO("--");

		if (ht->eqfn(walk, item)) 
			return walk;
	}

	UNUSED(idx);
	return NULL;
}

void update_priority(struct ip_hashtable *ht, statistic stat_ave, statistic stat_dis){
	ip_statistic *walk;
	for (int i = 0; i < ht->bins; i++){
		TAILQ_FOREACH(walk, &ht->ht_table[i], links) {
			if(walk->packet_recv_num > MAX(THROUGHPUT_TH,stat_ave.packet_recv_num+stat_dis.packet_recv_num*2)){
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


