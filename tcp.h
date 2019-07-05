#ifndef DECODE_TCP_H
#define DECODE_TCP_H
 typedef struct  {
            uint8_t fin:1,
                syn:1,
                rst:1,
                psh:1,
                ack:1,
                urg:1,
                ece:1,
                cwr:1;
        } flags_type;

    struct tcp_header {
        uint16_t sport;
        uint16_t dport;
        uint32_t seq;
        uint32_t ack_seq;
        uint8_t doff:4,
            res1:4;
        union {
            flags_type flags;
            uint8_t flags_8;
        };
        uint16_t	window;
        uint16_t	check;
        uint16_t	urg_ptr;
    } ;
int decode_tcp(const uint8_t *pkt, const uint32_t len, Packet *p);

#endif /* DECODE_TCP_H */
