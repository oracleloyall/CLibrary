#ifndef PACKET_H
#define PACKET_H

/*
 * Requires C99 types
 */
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/time.h>

#include "ipaddr.h"
#include "protocol.h"
#include "options.h"
#include "stats.h"

typedef struct _Packet Packet;

struct pcap_pkthdr;

/* Packet Functions
 */
Packet *packet_create( );

void packet_destroy(Packet *);

int packet_set_datalink(unsigned datalink);

int packet_decode(Packet *, const unsigned char *, unsigned);

int packet_decode_pcap(Packet *, const uint8_t *,
    const struct pcap_pkthdr *);

const char * packet_lib_version(void);

void packet_stats(const struct packet_stats **ps);


/* Packet layer functions
 * XXX Defining these here because there is a dependency on type Packet.
 */
Protocol *packet_proto_first(Packet *packet, unsigned *);

Protocol *packet_proto_next(Packet *packet, unsigned *);

unsigned packet_proto_count(Packet *packet);

int packet_proto_size(Protocol *proto);

const uint8_t *packet_proto_data(Protocol *proto);

PROTOCOL packet_proto_proto(Protocol *proto);

const char *packet_proto_name(Protocol *proto);


/* Packet Type Accessors
 */
int packet_version(Packet *packet);

struct ipaddr packet_srcaddr(Packet *packet);

struct ipaddr packet_dstaddr(Packet *packet);

bool packet_is_fragment(Packet *packet);

bool packet_frag_mf(Packet *packet);

bool packet_frag_df(Packet *packet);

uint16_t packet_frag_offset(Packet *packet);

uint8_t packet_protocol(Packet *packet);

uint32_t packet_id(Packet *packet);

uint8_t packet_ttl(Packet *packet);

uint8_t packet_tos(Packet *packet);

uint16_t packet_srcport(Packet *packet);

uint16_t packet_dstport(Packet *packet);

uint16_t packet_mss(Packet *packet);

uint16_t packet_win(Packet *packet);

uint16_t packet_winscale(Packet *packet);

uint32_t packet_seq(Packet *packet);

uint32_t packet_ack(Packet *packet);

int packet_tcpflags(Packet *packet);

bool packet_tcp_fin(Packet *packet);

bool packet_tcp_syn(Packet *packet);

bool packet_tcp_rst(Packet *packet);

bool packet_tcp_push(Packet *packet);

bool packet_tcp_ack(Packet *packet);

bool packet_tcp_urg(Packet *packet);

void packet_set_payload(Packet *packet, void *payload,
    uint32_t paysize);

bool packet_has_alt_payload(Packet *packet);

uint32_t packet_raw_paysize(Packet *packet);

const uint8_t *packet_raw_payload(Packet *packet);

uint32_t packet_paysize(Packet *packet);

const uint8_t *packet_payload(Packet *packet);

#endif /* PACKET_H */
