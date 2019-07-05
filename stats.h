#ifndef PACKET_STATS_H
#define PACKET_STATS_H
#include<stdint.h>
/* Protocol decoding stats
 */
struct packet_stats {

    uint32_t total_bytes;       /* total bytes received */
    uint32_t total_errors;      /* bad packet received */

    uint32_t mplss_packets;     /* mpls packets */
    uint32_t mplss_bytes;       /* mpls bytes */
    uint32_t mplss_tooshort;    /* packet too short */

    uint32_t pppoes_packets;    /* pppoe packets */
    uint32_t pppoes_bytes;      /* pppoe bytes */
    uint32_t pppoes_tooshort;   /* packet too short */

    uint32_t ppps_packets;      /* ppp packets */
    uint32_t ppps_bytes;        /* ppp bytes */
    uint32_t ppps_tooshort;     /* packet too short */

    uint32_t ipxs_packets;      /* total number of ipx packets */
    uint32_t ipxs_bytes;        /* total number of ipx bytes */
    uint32_t ipxs_badsum;       /* checksum errors */
    uint32_t ipxs_tooshort;     /* packet too short */

    uint32_t ips_packets;       /* total number of ip packets */
    uint32_t ips_bytes;         /* total number of ip bytes */
    uint32_t ips_badsum;        /* checksum errors */
    uint32_t ips_tooshort;      /* packet too short */
    uint32_t ips_toosmall;      /* not enough data */ // XXX UNUSED
    uint32_t ips_badhlen;       /* ip hlen < data size */ // XXX UNUSED
    uint32_t ips_badlen;        /* ip len < ip hlen */ // XXX UNUSED
    uint32_t ips_fragments;     /* fragments received */

    uint32_t ip6s_packets;       /* total number of ip packets */
    uint32_t ip6s_bytes;         /* total number of ip bytes */
    uint32_t ip6s_ext;           /* hop by hop headers */
    uint32_t ip6s_rte;           /* routing headers */
    uint32_t ip6s_tooshort;      /* packet too short */
    uint32_t ip6s_toosmall;      /* not enough data */ // XXX UNUSED
    uint32_t ip6s_badlen;        /* ip len < ip hlen */ // XXX UNUSED
    uint32_t ip6s_fragments;     /* fragments received */

    uint32_t tcps_packets;      /* total tcp packets */
    uint32_t tcps_bytes;        /* total tcp bytes */
    uint32_t tcps_badsum;       /* checksum errors */
    uint32_t tcps_badoff;       /* bad offset */ // XXX UNUSED
    uint32_t tcps_tooshort;     /* not enough data */

    uint32_t udps_packets;      /* total udp packets */
    uint32_t udps_bytes;        /* total udp bytes */
    uint32_t udps_badsum;       /* checksum errors */
    uint32_t udps_nosum;        /* no checksum */
    uint32_t udps_tooshort;     /* not enough data */

    uint32_t icmps_packets;     /* total icmp packets */
    uint32_t icmps_bytes;       /* total icmp bytes */
    uint32_t icmps_badsum;      /* checksum errors */
    uint32_t icmps_badtype;     /* bad icmp code */
    uint32_t icmps_badcode;     /* bad icmp type */
    uint32_t icmps_tooshort;    /* not enough data */

    uint32_t sctps_packets;     /* total sctp packets */
    uint32_t sctps_bytes;       /* total sctp bytes */
    uint32_t sctps_badsum;      /* checksum errors */
    uint32_t sctps_badtype;     /* bad chunk type */ // XXX UNUSED 
    uint32_t sctps_tooshort;    /* not enough data */
};
typedef enum State {
	UNKNOWN, SYN_SENT, ESTABLISHED, FIN_SENT, RST_SENT
} STATE;
typedef enum Flags {
	FIN = 1, SYN = 2, RST = 4, PSH = 8, ACK = 16, URG = 32, ECE = 64, CWR = 128
} FLAGS;

struct TcpFlow{

	 STATE state_;
	 FLAGS Flags_;
	// client syc time
	//server ack and
	//
};
#endif /* PACKET_STATS_H */
