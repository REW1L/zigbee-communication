#include <string.h>
#include "protocol_encode.h"
#include <stdio.h>
#include <time.h>
#include "lz4.h"

static void*** func_array;

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
/*
static raw_field pack_way(char* dst, 
                          way* way, 
                          char to_dst, 
                          uint8_t field_num)
{
  raw_field ret;
  uint16_t i, size;
  uint32_t k, coord, offset = 0;
  uint32_t* loc_way;
  loc_way = way->way;
  ret.size = way->length*2*COORD_SIZE;
  if (to_dst)
    ret.data = dst;
  else
    ret.data = (char*)malloc(ret.size + FIELD_ID_SIZE + SIZE_SIZE);

  *(ret.data) = field_num;
  *(ret.data+1) = (way->length) & 0xff;
  *(ret.data+2) = (way->length / 0x100) & 0xff;

  ret.size += FIELD_ID_SIZE + SIZE_SIZE;
  offset += FIELD_ID_SIZE + SIZE_SIZE;
  size = way->length*2;

  for (k = 0; k < size; k++)
  {
    coord = loc_way[k];
    for (i = 0; i < COORD_SIZE; i++)
    {
      *(ret.data+offset) = (uint8_t)coord&0xff;
      coord /= 0x100;
      offset++;
    }
  }
  return ret;
}
*/
int8_t make_header(char* packet, 
                   uint16_t flags, 
                   uint32_t id, 
                   uint8_t message_number, 
                   uint8_t op,
                   uint16_t real_size,
                   uint16_t compressed_size)
{
  uint8_t offset, i;
  uint16_t flags_temp = flags;

  // TODO: Add version of protocol

  for (i = 0; i < FLAGS_SIZE; i++)
  {
    packet[i] = (flags_temp)&0xff;
    flags_temp /= 0x100;
  }

  offset = FLAGS_SIZE;

  for (i = 0; i < MESSAGE_NUMBER_SIZE; i++)
  {
    packet[offset+i] = (message_number)&0xff;
    message_number /= 0x100;
  }

  offset += MESSAGE_NUMBER_SIZE;

  for (i = 0; i < ID_SIZE; i++)
  {
    packet[offset+i] = (id)&0xff;
    id /= 0x100;
  }

  offset += ID_SIZE;

  for (i = 0; i < OP_SIZE; i++)
  {
    packet[offset+i] = (op)&0xff;
    op /= 0x100;
  }

  offset += OP_SIZE;

  if (flags & COMPRESS_5000)
  {
    for (i = 0; i < COMPRESS_SIZE; i++)
    {
      packet[offset+i] = (real_size)&0xff;
      real_size /= 0x100;
    }

    offset += COMPRESS_SIZE;

    for (i = 0; i < COMPRESS_SIZE; i++)
    {
      packet[offset+i] = (compressed_size)&0xff;
      compressed_size /= 0x100;
    }
    
    offset += COMPRESS_SIZE;
  }

  return offset;
}

packets pack_info(RouteConfig inf, int16_t flags)
{
  char* packet;
  char* raw_data;
  int8_t offset = 0, i, temp;
  int32_t size, field_offset = 0, raw_offset = 0, free_space;
  raw_field field;
  packets ret;
  // way w;

  // if(inf.way != NULL)
  // {
    // size = (SIZE_SIZE+FIELD_ID_SIZE)*4 + 
      // SPEED_SIZE+TIME_SIZE+COORD_SIZE*4+((inf.way_length)*COORD_SIZE*2);
  // }
  // else
  // {
    size = (SIZE_SIZE+FIELD_ID_SIZE)*4 + SPEED_SIZE+TIME_SIZE+COORD_SIZE*4;
  // }

  packet = (char*)calloc(size, 1);

  field = pack_speed(&packet[offset], &(inf.speed), 1, SPEED);

  offset = field.size;

  temp = offset; // DEBUG

  field = pack_coords(&packet[offset], inf.coords_src, 1, COORDS_START);

  offset += field.size;

  field = pack_time(&packet[offset], &(inf.time), 1, TIME);

  offset += field.size;

  field = pack_coords(&packet[offset], inf.coords_dst, 1, COORDS_END);

  offset += field.size;

  // if(inf.way != NULL)
  // {
  //   w.way = inf.way;
  //   w.length = inf.way_length;

  //   field = pack_way(&packet[offset], &w, 1, WAY);
  //   offset += field.size;
  // }
  // else
  // {
  //   // printf("Way is NULL\n");
  // }

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
  uint16_t i, offset, compressed_size = 0;
  char *flat_packets, *compressed_data;
  packets ret = { .raw_data = NULL, .data = NULL, .number = 0};

  if(flags & COMPRESS_5000)
  {
    // printf("Compress is not done yet.\n");
    // TODO: COMPRESSED PACKETS
  }

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

      make_header(&(flat_packets[offset]), flags, id, i, op, 0, 0);

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

packets compress_packets(packets pckts, uint32_t id, uint8_t flags)
{
  packets ep;
  uint16_t i;
  uint8_t offset;
  size_t size;
  size_t max_size = LZ4_compressBound(pckts.number*FRAME_SIZE);
  char* raw_data = calloc(max_size, 1);
  flags |= COMPRESS_5000;
  ep.number = 0;
  for(i = 0; i < 5000; i++)
  {
    size = LZ4_compress_fast(pckts.raw_data, raw_data, 
      pckts.number*FRAME_SIZE, max_size, 5);
    if(size < 1)
    {
      free(raw_data);
      return pckts;
    }
  }

  if (size  <= ( FRAME_SIZE - HEADER_COMPRESSED_SIZE ))
  {
    max_size = 1;
  }
  else
  {
    max_size = size - ( FRAME_SIZE - HEADER_COMPRESSED_SIZE );
    max_size = 1 + (max_size - ( FRAME_WITHOUT_HEADER ))/(FRAME_WITHOUT_HEADER) 
    + ((max_size)%(FRAME_WITHOUT_HEADER))? 1 : 0;
  }

  ep.number = max_size;
  ep.raw_data = calloc(max_size*FRAME_SIZE, 1);

  offset = make_header(ep.raw_data, 
    (--max_size)? flags : (flags | LAST_MESSAGE), id, 0, 0, 
    pckts.number*FRAME_SIZE, size);

  for(i = 1; i <= max_size; i++)
  {
    offset = make_header(&ep.raw_data[ep.number*FRAME_SIZE], 
      (i != max_size)? flags : (flags | LAST_MESSAGE), id, i, 0, 0, 0);
    memcpy(&ep.raw_data[i*FRAME_SIZE], 
      &raw_data[FRAME_WITHOUT_HEADER*i], FRAME_WITHOUT_HEADER);
  }

  free(raw_data);
  return ep;
}
