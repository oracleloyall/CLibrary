

#ifndef PACKET_PROTOCOL_H
#define PACKET_PROTOCOL_H

typedef struct _PacketLayer Protocol;

typedef enum
{
    PROTO_ETH,
    PROTO_PPP,
    PROTO_VLAN,
    PROTO_MPLS,
    PROTO_PPPOE,
    PROTO_IPX,
    PROTO_SPX,
    PROTO_IP4,
    PROTO_IP6,
    PROTO_IP6_RTE,
    PROTO_IP6_FRAG,
    PROTO_IP6_EXT, /* DST OPTS and HBH */
    PROTO_GRE,
    PROTO_TCP,
    PROTO_UDP,
    PROTO_SCTP,
    PROTO_ICMP,
    PROTO_ICMP6,
    PROTO_MAX
} PROTOCOL;

#endif /* PACKET_PROTOCOL_H */
