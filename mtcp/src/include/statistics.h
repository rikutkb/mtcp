#ifndef STATISTICS_H
    #define STATISTICS_H
    #if defined(USE_DDOSPROT)

    #include "mtcp.h"
    #include <stdio.h>
    #include <stdlib.h>
    #include <sys/time.h>
    #include <sys/queue.h>
    #include <linux/if_ether.h>
    #include <linux/tcp.h>
    #include <linux/udp.h>
    #include <netinet/ip.h>
    #include <pthread.h>
    #include <math.h>
    #include "memory_mgt.h"
    #define MAX_PRIORITY 2
    #define THROUGHPUT_TH 100
    #define NUM_BINS_IPS 1021
    #define POW2(x) (x*x)
    #define MAX(a, b) ((a)>(b)?(a):(b))
    #define IS_IP_TABLE(x)	(x == IPHash)
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
    typedef struct hash_ip_head {
        ip_statistic *tqh_first;
        ip_statistic **tqh_last;
    } hash_ip_head;
    typedef struct list_ip_head {
        struct ip_statistic *tqh_first;
        struct ip_statistic **tqh_last;
    } list_ip_head;

    /* hashtable structure */
    struct ip_hashtable {
        uint32_t bins;
        union{
            hash_ip_head *ht_table;
            list_ip_head *lt_table;
        };
        // functions
        unsigned int (*hashfn) (const void *);
        int (*eqfn) (const void *, const void *);
    };

    //struct ip_statistic* CreateNewIpEntry(mtcp_manager_t mtcp, uint32_t saddr);
    struct ip_hashtable *CreateIPHashtable(unsigned int (*hashfn) (const void *), int (*eqfn) (const void *, const void *), int bins);
    void DestroyIPHashtable(struct ip_hashtable *ht);
    int IPHTInsert(struct ip_hashtable *ht, void *it);
    void* IPHTRemove(struct ip_hashtable *ht, void *it);
    void *IPHTSearch(struct ip_hashtable *ht, const void *it);
    //unsigned int HashIPListener(const void *hbo_port_ptr);
    ip_statistic* CreateIPStat(mtcp_manager_t mtcp, uint32_t ip);

    int JudgeDropbyIp(struct ip_hashtable *ht, uint32_t saddr);
    int EqualIP(const void *ip1, const void *ip2);
    unsigned int IPHash(const void *saddr);
    void AddedPacketStatistics(mtcp_manager_t mtcp, struct ip_hashtable *ht,uint32_t saddr,int ip_len);
    int get_average(struct ip_hashtable *ht, statistic *stat_ave);
    int get_dispresion(struct ip_hashtable *ht,  statistic stat_ave, statistic *stat_dis);
    void get_statistics(struct ip_hashtable *ht);
    void update_priority(struct ip_hashtable *ht, statistic stat_ave, statistic stat_dis);
    void ProcessRstTCPPacket(mtcp_manager_t mtcp, const struct iphdr *iph, uint32_t cur_ts,
     const struct tcphdr *tcph, uint32_t seq, int payloadlen );
    #endif
#endif