#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdint.h>

#include "packet_private.h"

/* convenient helper */
#define current_layer (packet->layer_count-1)

/* Insert a protocol layer into the packet */
inline int
packet_layer_ins(Packet *packet, const uint8_t *start, unsigned size,
 PROTOCOL proto)
{
    if (packet->layer_count == MAX_LAYERS)
    {
        return -1;
    }

    packet->layer_count++;

    /* index is count - 1 */
    packet->layer[current_layer].protocol = proto;
    packet->layer[current_layer].start = start;
    packet->layer[current_layer].size = size;

    return 0;
}

/* Remove a protocol layer from the packet
 *
 * NOTE: to remove a layer all you need to do is reduce the count.
 * this saves time by not having to memset the structure */
inline int
packet_layer_rem(Packet *packet)
{
    if (packet->layer_count == 0)
    {
        return -1;
    }

    packet->layer_count--;

    return 0;
}

/* Assign a pointer to the current layer
 *
 */
inline Protocol *
packet_layer_current(Packet *packet)
{
    if (packet->layer_count == 0)
    {
        return NULL;
    }

    return &packet->layer[current_layer];
}

