#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define __FAVOR_BSD
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>

#include "packet_private.h"


extern struct packet_stats s_stats;



//static int flags(struct tcp_header * header_){
//    return (header_.res1 << 8) | header_.flags_8;
//}
//
//bool has_flags(int check_flags)  {
//    return (flags() & check_flags) == check_flags;
//}

static const int8_t tcpopt_len_tbl[] =
{
     0, 0, 4, 3, 2, -1, 6, 6, 10
};

#define TCPOPT_MAX (sizeof(tcpopt_len_tbl)/sizeof(tcpopt_len_tbl[0]))

/* Some sort of state machine for parsing tcp options. */
/* This was allot harder than it should have been. */
static inline void 
decode_tcp_options(Packet *p, const uint8_t *start, const unsigned len)
{
    if (len > MAX_TCPOPTLEN)
        return;

    const uint8_t *opt_start = start;
    unsigned depth = 0;

    while (opt_start < start + len)
    {
        uint8_t jmp_len, opt_len;

        /* Useless option code detected */
        if (*opt_start >= TCPOPT_MAX)
        {
            if (!((len-depth) > 0))
                return;

            jmp_len = *(opt_start + 1);

            /* skip processing and retrieve next option, abort otherwise */
            if (len < jmp_len)
                goto nxt_tcp_opt;
            else
                return;
        }

        int8_t expected = tcpopt_len_tbl[*opt_start];

        switch (expected)
        {
            /* Variable length option */
            case -1:
                if (!((len-depth) > 0)) return;
                jmp_len = *(opt_start + 1);
                opt_len = jmp_len - 2;
                break;

            /* EOL or NOP */
            case 0:
                jmp_len = 1;
                opt_len = expected;
                break;

            /* Fixed size */
            default:
                if (!((len-depth) > 0)) return;
                jmp_len = *(opt_start + 1);

                /* Validate it is the correct length */
                if (jmp_len != expected) return;
                opt_len = jmp_len - 2;
                break;
        }

        /* Add this option to the tcp opts array */
        p->tcp_option[p->tcpopt_count].type = *opt_start;
        p->tcp_option[p->tcpopt_count].len = opt_len;
        p->tcp_option[p->tcpopt_count].value = opt_len ? (opt_start + 2): NULL;
        p->tcpopt_count++;

nxt_tcp_opt:
        depth += jmp_len;
        opt_start += jmp_len;
    }

    return;
}

int
decode_tcp(const uint8_t *pkt, const uint32_t len, Packet *p)
{
    struct tcp_header *tcp = (struct tcp_header *)pkt;

    struct tcphdr *tcp_head = (struct tcphdr *)pkt;
    s_stats.tcps_packets++;
    s_stats.tcps_bytes += len;

    if (len < sizeof *tcp_head)
    {
        s_stats.tcps_tooshort++;
        return -1;
    }

    unsigned hlen = tcp_head->th_off * 4;
    packet_layer_ins(p, pkt, hlen, PROTO_TCP);
    p->transport = packet_layer_current(p);

    p->srcport = ntohs(tcp_head->th_sport);
    p->dstport = ntohs(tcp_head->th_dport);

    p->payload += hlen;
    p->paysize -= hlen;

    /* decode tcp options */
    unsigned short optlen = hlen - sizeof *tcp_head;
    if (optlen)
    {
        decode_tcp_options(p, pkt + hlen - optlen, optlen);
    }

    return 0;
}
