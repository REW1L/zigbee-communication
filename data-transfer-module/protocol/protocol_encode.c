#include <string.h>
#include "protocol_encode.h"

static raw_field pack_coords(char* dst, 
                             uint32_t* coords, 
                             char to_dst, 
                             uint8_t field_num)
{
  raw_field ret;
  uint32_t loc_coords[2] = {coords[0], coords[1]};
  uint8_t i, j, offset = 0;
  ret.size = COORD_SIZE * 2;
  if (to_dst)
    ret.data = dst;
  else
    ret.data = (char*)malloc(ret.size + SIZE_SIZE + FIELD_ID_SIZE);

  *(ret.data) = field_num;
  *(ret.data+1) = ret.size & 0xff;
  *(ret.data+2) = (ret.size / 0x100) & 0xff;
  ret.size += SIZE_SIZE + FIELD_ID_SIZE;
  offset += SIZE_SIZE + FIELD_ID_SIZE;
  for (j = 0; j < 2; j++)
  {
    for (i = 0; i < COORD_SIZE; i++)
    {
      *(ret.data+offset+i) = (char)((loc_coords[j])&0xff);
      loc_coords[j] /= 0x100;
    }
    offset += COORD_SIZE;
  }
  return ret;
}

static raw_field pack_time(char* dst, 
                           uint32_t* ptime, 
                           char to_dst, 
                           uint8_t field_num)
{
  raw_field ret;
  uint8_t i, offset;
  uint32_t time = *ptime;
  ret.size = TIME_SIZE;
  if (to_dst)
    ret.data = dst;
  else
    ret.data = (char*)malloc(ret.size + SIZE_SIZE + FIELD_ID_SIZE);

  *(ret.data) = field_num;
  *(ret.data+1) = ret.size & 0xff;
  *(ret.data+2) = (ret.size / 0x100) & 0xff;
  ret.size += SIZE_SIZE + FIELD_ID_SIZE;
  offset = SIZE_SIZE + FIELD_ID_SIZE;
  for (i = 0; i < TIME_SIZE; i++)
  {
    *(ret.data+offset+i) = (char)((time)&0xff);
    time /= 0x100;
  }

  return ret;
}

static raw_field pack_speed(char* dst, 
                            uint32_t* pspeed, 
                            char to_dst, 
                            uint8_t field_num)
{
  raw_field ret;
  uint8_t i, offset;
  uint32_t speed = *pspeed;
  ret.size = SPEED_SIZE;
  if (to_dst)
    ret.data = dst;
  else
    ret.data = (char*)malloc(ret.size + 3);
  *(ret.data) = field_num;
  *(ret.data+1) = ret.size & 0xff;
  *(ret.data+2) = (ret.size / 0x100) & 0xff;
  offset = SIZE_SIZE + FIELD_ID_SIZE;
  ret.size += SIZE_SIZE + FIELD_ID_SIZE;
  for (i = 0; i < SPEED_SIZE; i++)
  {
    *(ret.data+offset+i) = (char)(speed&0xff);
    speed /= 0x100;
  }

  return ret;
}

static raw_field pack_direction(char* dst, 
                                uint8_t* pdirection, 
                                char to_dst, 
                                uint8_t field_num)
{
  raw_field ret;
  uint8_t i, offset;
  uint8_t direction = *pdirection;
  ret.size = DIRECTION_SIZE;
  if (to_dst)
    ret.data = dst;
  else
    ret.data = (char*)malloc(ret.size + 3);
  *(ret.data) = field_num;
  *(ret.data+1) = (uint8_t)(ret.size & 0xff);
  *(ret.data+2) = (uint8_t)((ret.size / 0x100) & 0xff);
  offset = SIZE_SIZE + FIELD_ID_SIZE;
  ret.size += SIZE_SIZE + FIELD_ID_SIZE;
  for (i = 0; i < DIRECTION_SIZE; i++)
  {
    *(ret.data+offset+i) = (char)(direction&0xff);
    direction /= 0x100;
  }

  return ret;
}

int8_t make_header(char* packet, 
                   uint16_t flags, 
                   uint32_t id, 
                   uint8_t message_number, 
                   uint8_t op,
                   uint16_t size)
{
  uint8_t offset, i;
  uint16_t flags_temp = flags;

  // TODO: Add version of protocol

  for (i = 0; i < FLAGS_SIZE; i++)
  {
    packet[i] = (uint8_t)((flags_temp)&0xff);
    flags_temp /= 0x100;
  }

  offset = FLAGS_SIZE;

  for (i = 0; i < MESSAGE_NUMBER_SIZE; i++)
  {
    packet[offset+i] = (uint8_t)((message_number)&0xff);
    message_number /= 0x100;
  }

  offset += MESSAGE_NUMBER_SIZE;

  for (i = 0; i < ID_SIZE; i++)
  {
    packet[offset+i] = (uint8_t)((id)&0xff);
    id /= 0x100;
  }

  offset += ID_SIZE;

  for (i = 0; i < OP_SIZE; i++)
  {
    packet[offset+i] = (uint8_t)((op)&0xff);
    op /= 0x100;
  }

  offset += OP_SIZE;
  return offset;
}

packets pack_info(RouteConfig inf, int8_t flags)
{
  char* packet;
  int8_t offset = 0, temp;
  int32_t size;
  raw_field field;
  packets ret;

  size = (SIZE_SIZE+FIELD_ID_SIZE)*5 + SPEED_SIZE+TIME_SIZE+COORD_SIZE*4+DIRECTION_SIZE;

  packet = (char*)calloc(size, 1);

  field = pack_speed(&packet[offset], &(inf.speed), 1, SPEED);

  offset = field.size;

  field = pack_coords(&packet[offset], inf.coords_src, 1, COORDS_START);

  offset += field.size;

  field = pack_time(&packet[offset], &(inf.time), 1, TIME);

  offset += field.size;

  field = pack_coords(&packet[offset], inf.coords_dst, 1, COORDS_END);

  offset += field.size;

  field = pack_direction(&packet[offset], &(inf.direction), 1, DIRECTION);

  offset += field.size;

  ret = make_packets(packet, size, flags, inf.id, OP_INFO);
  free(packet);

  return ret;
}

packets make_packets(char* data, 
                      size_t size, 
                      uint8_t flags, 
                      uint32_t id, 
                      uint8_t op)
{
  uint16_t i, offset;
  char *flat_packets;
  packets ret = { .raw_data = NULL, .data = NULL, .number = 0};

  if(size/(FRAME_WITHOUT_HEADER) < 256)
  {
    ret.number = (uint8_t)(size/(FRAME_WITHOUT_HEADER));
    if(size % FRAME_WITHOUT_HEADER)
      ret.number++;
    flat_packets = (char*)calloc((ret.number)*(FRAME_SIZE), 1);
    ret.raw_data = flat_packets;
    ret.data = (char**)calloc(ret.number, sizeof(char*));
    offset = 0;

    for(i = 0; i < ret.number; i++)
    {
      if(i == ret.number - 1)
        flags |= LAST_MESSAGE;
      ret.data[i] = &(flat_packets[offset]);

      make_header(&(flat_packets[offset]), flags, id, i, op, 0);

      if(size > FRAME_WITHOUT_HEADER)
      {
        memcpy(&(flat_packets[offset+HEADER_SIZE]), 
          &(data[i*FRAME_WITHOUT_HEADER]), FRAME_WITHOUT_HEADER);
        size -= FRAME_WITHOUT_HEADER;
      }
      else
      {
        memcpy(&(flat_packets[offset+HEADER_SIZE]), 
          &(data[i*FRAME_WITHOUT_HEADER]), size);
      }

      offset += FRAME_SIZE;
    }
  }
  return ret; 
}
