#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define __FAVOR_BSD
#include <netinet/udp.h>

#include "packet_private.h"


extern struct packet_stats s_stats;

int
decode_udp(const uint8_t *pkt, const uint32_t len, Packet *p)
{
    struct udphdr *udp = (struct udphdr *)pkt;

 //   s_stats.udps_packets++;
//    s_stats.udps_bytes+=len;

    if (len < sizeof *udp)
    {
       // s_stats.udps_tooshort++;
        return -1;
    }

    packet_layer_ins(p, pkt, sizeof *udp, PROTO_UDP);
    p->transport = packet_layer_current(p);

    p->srcport = ntohs(udp->uh_sport);
    p->dstport = ntohs(udp->uh_dport);

    p->payload += sizeof *udp; 
    p->paysize -= sizeof *udp;

    /* UDP checksum is mandatory for ipv6 */
    if (udp->uh_sum == 0 && p->version == 6)
    {
       // s_stats.udps_badsum++;
        return -1;
    }
    else if (udp->uh_sum == 0)
        return 0;


    return 0;
}
