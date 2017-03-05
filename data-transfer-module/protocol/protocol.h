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
#define FLAGS_SIZE 2
#ifndef RF24
#define FRAME_SIZE 74
#else
#define FRAME_SIZE 24
#endif
#define MESSAGE_NUMBER_SIZE 1
#define OP_SIZE 1
#define COMPRESS_SIZE 4
#define OP_INFO 1
#define SIZE_SIZE 2
#define FIELD_ID_SIZE 1

#define HEADER_SIZE (ID_SIZE+FLAGS_SIZE+MESSAGE_NUMBER_SIZE+OP_SIZE)
#define HEADER_COMPRESSED_SIZE (HEADER_SIZE+COMPRESS_SIZE)
#define FRAME_WITHOUT_HEADER (FRAME_SIZE-HEADER_SIZE)

#define LAST_MESSAGE 0b01
#define COMPRESS_5000 0b10

typedef enum
{
  NO_FIELD,
  COORDS_START,
  COORDS_END,
  TIME,
  SPEED,
  WAY,
  END_OF_FIELDS
} FIELDS;

typedef struct {
  uint8_t number;
  char** data;
  char* raw_data;
} packets; 

typedef struct {
  uint16_t size;
  char* data;
} raw_field;

typedef struct {
  uint32_t *way;
  uint32_t length;
} way;

typedef struct {
  uint32_t id;
  uint32_t coords_src[2];
  uint32_t coords_dst[2];
  uint32_t time;
  uint32_t speed;
  // uint32_t *way;
  // uint32_t way_length;
} RouteConfig;

typedef struct
{
  uint8_t header_flags;
  uint8_t number;
  uint32_t id;
  uint8_t size;
  uint8_t op;
  char eui[9];
  char packet_data[FRAME_SIZE];
} zigbee_packet;

typedef enum
{
  NO_COMMAND,
  N,
  EN,
  JN,
  DASSL,
  BCASTB,
  ATI,
  END_OF_ENUM
} AtCommands;

#if defined (__cplusplus)
}
#endif
#endif /* PROTOCOL_H */