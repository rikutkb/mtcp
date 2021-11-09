
#include <assert.h>
#include <time.h>
#include <inttypes.h>

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


void get_average(){
    //全てのホワイトリストのパケット数、スループットを計算
    //ホワイトリストのスループットをリセット

}
void get_dispresion(){

}
void update_priority(){

}
void get_statistics(mtcp_manager mtcp){
    uint32_t packet_ave,throughput_ave;
    get_average();
    uint32_t packet_dis,throughput_dis;
    get_dispresion();
    update_priority();
}