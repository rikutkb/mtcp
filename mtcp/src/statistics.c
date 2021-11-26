#include "statistics.h"


struct ip_statistic* CreateNewIpEntry(mtcp_manager_t mtcp, uint32_t saddr){
	struct ip_statistic *cur_ip_stat;
	struct ip_statistic a;
	cur_ip_stat = &a;
	//do alloc memory
	cur_ip_stat->ip = saddr;
	cur_ip_stat->priority  = 1;
	return cur_ip_stat;

}
int JudgeDropbyIp(struct hashtable *ht,uint32_t saddr){
	//if is attacking
	struct ip_statistic *cur_ip_stat = NULL;
	struct ip_statistic ip_stat;
	ip_stat.ip = saddr;
	if (!(cur_ip_stat = IpWhiteHTSearch(ht, &ip_stat))) {// not in hashtable
		return 1;
	}
	switch(cur_ip_stat->priority){
		case 2:
			return 1;
		case 1://10%
			return rand()%10+1<=1;
		case 0://40%
			return rand()%10+1<=4;
	}
	
}
int EqualIP(const void *ip1, const void *ip2)
{
	struct ip_statistic *ip_1 = (ip_statistic *)ip1;
	struct ip_statistic *ip_2 = (ip_statistic *)ip2;

	return (ip_1->ip==ip_2->ip);
}
unsigned int IPHashFlow(const void *saddr)
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
void AddedPacketStatistics(struct hashtable *ht,uint32_t saddr,int ip_len){
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

int get_average(struct hashtable *ht, statistic *stat_ave){
	uint8_t valid_ips=0;

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
int get_dispresion(struct hashtable *ht, statistic stat_ave, statistic *stat_dis){
	statistic stat_sum;
	int valid_ips = 0;
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
    if(get_average(mtcp->ip_stat_table,&stat_ave)){
		statistic stat_dis;
		if(get_dispresion(mtcp->ip_stat_table,stat_ave,&stat_dis)){
			update_priority(mtcp->ip_stat_table,stat_ave,stat_dis);
			return;
		}
	}
	//reset_priority???

}
void* IpWhiteHTSearch(struct hashtable *ht, const void *it){
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

	UNUSED(idx);
	return NULL;
}

void update_priority(struct hashtable *ht, statistic stat_ave, statistic stat_dis){
	ip_statistic *ip_stat;
	uint32_t throughput_def = 100;
	for (int i = 0; i < ht->bins; i++){
		TAILQ_FOREACH(walk, &ht->ht_table[i], links) {
			if(walk->packet_recv_num > MAX(throughput_def,stat_ave.packet_recv_num+stat_dis.packet_recv_num*2)){
				walk->priority--;
			}else{
				walk->priority++;
			}
			walk->packet_recv_num = 0;


		}
		
	}
}


