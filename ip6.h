#ifndef DECODE_IP6_H
#define DECODE_IP6_H

int decode_ip6_rte(const uint8_t *, const uint32_t, Packet *);
int decode_ip6_frag(const uint8_t *, const uint32_t, Packet *);
int decode_ip6_ext(const uint8_t *, const uint32_t, Packet *);
int decode_ip6(const uint8_t *, const uint32_t, Packet *);

#endif /* DECODE_IP6_H */
