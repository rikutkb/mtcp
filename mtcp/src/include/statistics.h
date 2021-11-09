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
void get_average();

void get_statistics(mtcp_manager mtcp);
void get_dispresion();
void update_priority();
#endif