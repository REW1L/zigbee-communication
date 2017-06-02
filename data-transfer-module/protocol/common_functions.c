#include <stdint.h>
#include "common_functions.h"
#include "checksum.h"

uint16_t pr_crc16(void* data, int size)
{
  return crc_16(data, size);
}