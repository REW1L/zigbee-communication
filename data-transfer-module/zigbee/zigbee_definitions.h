#ifndef ZIGBEE_DEFINITIONS_H
#define ZIGBEE_DEFINITIONS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <stdint.h>

enum device_type 
{
  NON_TYPE,
  FFD,
  COO,
  ZED,
  SED,
  MED
};

struct network_info 
{
  enum device_type device;
  int16_t channel;
  int16_t power;
  char* pid;
  char* epid;
};

#if defined (__cplusplus)
}
#endif
#endif // ZIGBEE_DEFINITIONS_H
