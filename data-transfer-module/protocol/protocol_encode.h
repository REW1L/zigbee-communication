#ifndef PROTOCOL_ENCODE_H
#define PROTOCOL_ENCODE_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "protocol.h"

packets pack_info(RouteConfig inf, int8_t flags);
packets compress_packets(packets packets, uint32_t id, uint8_t flags);
packets make_packets(char* data, size_t size, uint8_t flags, uint32_t id, uint8_t op);

#if defined (__cplusplus)
}
#endif
#endif /* PROTOCOL_ENCODE_H */
