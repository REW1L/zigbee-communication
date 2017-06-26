#ifndef PARSER_H
#define PARSER_H

#include "protocol.h"
#include "Events.h"

#if defined (__cplusplus)
extern "C" {
#endif

ParsedData parse_received_data(const char* packet, int size);
RouteConfig parse_info(char* data, uint32_t size, uint32_t id);

#if defined (__cplusplus)
}
#endif
#endif /* PARSER_H */
