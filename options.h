#ifndef PACKET_OPTIONS_H
#define PACKET_OPTIONS_H

typedef struct Option
{
    uint8_t type;
    uint8_t len;
    const uint8_t *value;
} Option;

#endif /* PACKET_OPTIONS_H */
