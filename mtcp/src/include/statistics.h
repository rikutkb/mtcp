#ifdef STATISTICS_H
#define STATISTICS_H
#include "mtcp.h"


struct ip_statistic
{
    uint32_t packet_recv_num;
    uint32_t throughput_send_num;
    uint32_t packet_rtt;
    uint16_t priority;

};
unsigned int IPHashFlow(const void *saddr);
int EqualIP(const void *ip1, const void *ip2);
void AddedPacketStatistics(struct hashtable *ht,ip_addr saddr,int ip_len);
void get_average(struct hashtable *ht);
void get_statistics(mtcp_manager mtcp);
void get_dispresion(struct hashtable *ht);
void update_priority(struct hashtable *ht);
bool JudgeDropbyIp(struct hashtable *ht, ip_addr saddr);
#endif