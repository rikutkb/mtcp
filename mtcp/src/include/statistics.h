#ifndef STATISTICS_H
#define STATISTICS_H
#include "mtcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <pthread.h>
#include "tcp_stream.h"
#include "fhash.h"
#include "tcp_util.h"
#include <math.h>
#define POW2(x) (x*x)
typedef struct statistic{
    uint8_t packet_recv_num;
    uint32_t throughput_send_num;
    uint8_t packet_rtt;
}statistic;
typedef struct ip_statistic{
    uint32_t ip;
    uint8_t packet_recv_num;
    uint32_t throughput_send_num;
    uint8_t packet_rtt;
    uint8_t priority;
    TAILQ_ENTRY(ip_statistic) links;
}ip_statistic;

struct ip_statistic* CreateNewIpEntry(mtcp_manager_t mtcp, uint32_t saddr);
int JudgeDropbyIp(struct hashtable *ht, uint32_t saddr);
int EqualIP(const void *ip1, const void *ip2);
unsigned int IPHashFlow(const void *saddr);
void AddedPacketStatistics(struct hashtable *ht,uint32_t saddr,int ip_len);
int get_average(struct hashtable *ht, statistic *stat_ave);
int get_dispresion(struct hashtable *ht,  statistic stat_ave, statistic *stat_dis);
void get_statistics(mtcp_manager_t mtcp);
void* IpWhiteHTSearch(struct hashtable *ht, const void *it);
void update_priority(struct hashtable *ht, statistic stat_ave, statistic stat_dis);
#endif