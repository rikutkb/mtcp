#ifdef STATISTICS_H
#define STATISTICS_H
#include "mtcp.h"
#include <sys/queue.h>

struct statistic{
    uint8_t packet_recv_num;
    uint32_t throughput_send_num;
    uint8_t packet_rtt;
};
struct ip_statistic
{
    ip_addr ip;
    uint8_t packet_recv_num;
    uint32_t throughput_send_num;
    uint8_t packet_rtt;
    uint8_t priority;
    TAILQ_ENTRY(ip_statistic) links;
};
static inline ip_statistic * CreateNewIpEntry(mtcp_manager_t mtcp, ip_addr saddr);
bool JudgeDropbyIp(struct hashtable *ht, ip_addr saddr);
int EqualIP(const void *ip1, const void *ip2);
unsigned int IPHashFlow(const void *saddr);
void AddedPacketStatistics(struct hashtable *ht,ip_addr saddr,int ip_len);
statistic get_average(struct hashtable *ht);
statistic get_dispresion(struct hashtable *ht,  statistics stat_ave);
void get_statistics(mtcp_manager mtcp);
void * IpHTSearch(struct hashtable *ht, const void *it);
void update_priority(struct hashtable *ht, statistic stat_ave, statistic stat_dis);
#endif