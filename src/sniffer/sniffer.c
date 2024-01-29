#include "sniffer.h"

// 接口名
// 方向
// 过滤器
// 是否存pcap

sniffer_t* sniffer_new(char* device_name, enum direction_t direction) {
    sniffer_t* sniff = calloc(1, sizeof(sniffer_t));

    sniff->psock = packet_socket_new(device_name);
    sniff->packet = packet_new(PACKET_READ_BYTES);
    
}