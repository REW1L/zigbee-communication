#include <string.h>
#include "protocol_encode.h"

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

packets make_packets(const char* data,
                      size_t size, 
                      uint8_t flags, 
                      uint32_t id, 
                      uint8_t op)
{
  uint16_t i, offset;
  char *flat_packets;
  packets ret = { .packet_array = NULL, .number = 0};

  if(size/(FRAME_WITHOUT_HEADER) < 256)
  {
    ret.number = (uint8_t)(size/(FRAME_WITHOUT_HEADER));
    if(size % FRAME_WITHOUT_HEADER)
      ret.number++;
    ret.packet_array = (raw_field*)calloc(ret.number, sizeof(raw_field));
    flat_packets = (char*)calloc((ret.number)*(FRAME_SIZE), 1);
    offset = 0;

    for(i = 0; i < ret.number; i++)
    {
      if(i == ret.number - 1)
        flags |= LAST_MESSAGE;
      ret.packet_array[i].data = &(flat_packets[offset]);

      make_header(&(flat_packets[offset]), flags, id, (uint8_t)i, op, 0);

      if(size > FRAME_WITHOUT_HEADER)
      {
        ret.packet_array[i].size = FRAME_SIZE;
        memcpy(&(flat_packets[offset+HEADER_SIZE]), 
          &(data[i*FRAME_WITHOUT_HEADER]), FRAME_WITHOUT_HEADER);
        size -= FRAME_WITHOUT_HEADER;
        offset += FRAME_SIZE;
      }
      else
      {
        ret.packet_array[i].size = (uint16_t)(size+HEADER_SIZE);
        memcpy(&(flat_packets[offset+HEADER_SIZE]), 
          &(data[i*FRAME_WITHOUT_HEADER]), size);
        offset += size;
      }
    }
  }
  return ret; 
}
