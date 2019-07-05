
#ifndef PACKET_PRIVATE_H
#define PACKET_PRIVATE_H

#ifdef DEBUG
#define inline
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>
#include <netinet/tcp.h>
#include "packet.h"

struct _PacketLayer
{
    PROTOCOL protocol;
    unsigned size;
    const uint8_t *start;
};

#ifndef MAX_LAYERS
# define MAX_LAYERS 32
#endif

#warning "MAX_TCPOPTLEN value 32 is only a guessed value to fix compilation"
#define MAX_TCPOPTLEN 32

struct _Packet
{
    unsigned version;
    struct ipaddr srcaddr;
    struct ipaddr dstaddr;
    uint16_t srcport;
    uint16_t dstport;
    uint8_t protocol;
    bool mf, df;
    uint16_t offset;
    uint32_t id;
    uint8_t ttl;
    uint8_t tos;
    uint16_t mss;
    uint8_t wscale;
    unsigned paysize;
    const uint8_t *payload;

    /* Alt payload is set outside of libpacket */
    unsigned alt_paysize;
    uint8_t *alt_payload;

    Protocol *transport;
    unsigned layer_count;
    unsigned tcpopt_count;

    /* Start of some static lists */
    Protocol layer[MAX_LAYERS];
    Option tcp_option[MAX_TCPOPTLEN];
};
#define PKT_ZERO_LEN offsetof(Packet, tcpopt_count)

int packet_layer_ins(Packet *packet, const uint8_t *start,
    unsigned size, PROTOCOL proto);

int packet_layer_rem(Packet *packet);

Protocol * packet_layer_current(Packet *packet);

/* Inline packet helpers */
static inline bool
validate_transport_protocol(Packet *packet, PROTOCOL protocol)
{
    if (packet->transport == NULL)
        return false;

    if (packet->transport->protocol != protocol)
        return false;

    return true;
}

#endif /* PACKET_PRIVATE_H */
