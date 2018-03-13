#ifndef PROTOCOL_H
#define PROTOCOL_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#define ID_SIZE 4
#define COORD_SIZE 4
#define TIME_SIZE 4
#define SPEED_SIZE 4
#define DIRECTION_SIZE 1
#define FLAGS_SIZE 2
#define FRAME_SIZE (uint16_t)73
#define FRAME_DELIMITER 0x7E
#define MESSAGE_NUMBER_SIZE 1
#define OP_SIZE 1
#define OP_RAW_DATA 2
#define OP_POSITIONING 3
#define OP_GET_POSITIONING 4
#define SIZE_SIZE 2
#define FIELD_ID_SIZE 1
#define WAIT_BEFORE_SEND 100

#define HEADER_SIZE (ID_SIZE+FLAGS_SIZE+MESSAGE_NUMBER_SIZE+OP_SIZE)
#define FRAME_WITHOUT_HEADER (FRAME_SIZE-HEADER_SIZE)

#define LAST_MESSAGE 0b01

typedef enum
{
  NO_FIELD,
  COORDS_START,
  COORDS_END,
  TIME,
  SPEED,
  DIRECTION,
  END_OF_FIELDS
} FIELDS;

typedef struct {
  uint16_t size;
  char* data;
} raw_field;

typedef struct {
  uint8_t number;
  raw_field* packet_array;
} packets;

typedef struct
{
  uint8_t rssi;
  uint8_t header_flags;
  uint8_t number;
  uint32_t id;
  uint8_t size;
  uint8_t op;
  uint64_t from;
  uint64_t to;
  char packet_data[FRAME_SIZE];
} zigbee_packet;

typedef struct
{
  void* data;
  int size;
  uint8_t rssi;
} proto_frame;

#if defined (__cplusplus)
}
#endif
#endif /* PROTOCOL_H */
