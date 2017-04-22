#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

static int parse_coords(const char* raw_data, size_t size, uint32_t *coords);
static uint32_t parse_time(const char* raw_data, size_t size);
static uint32_t parse_speed(const char* raw_data, size_t size);
// static int parse_way(const char* raw_data, size_t size, uint32_t **way, uint32_t *way_length);

#ifdef ZIGBEE
ParsedData parse_received_data(const char* packet, int size)
{
  int ps, j; // packet start

  ParsedData ret = { .pevent = { .ev = NO_EVENT , .data = NULL }, .parsed = 0 };

  zigbee_packet* zgbp;

  if (packet == NULL)
  {
    // printf("Packet is NULL.\n");
    return ret;
  }
  // printf("Packet is not NULL.\n");

  for(ps = 0; ps < size && ( packet[ps] == '\r' || packet[ps] == '\n' || packet[ps] == '\0' ); ps++);

  if(ps < size)
  {
    // printf("Start of the packet was found. First 6 chars: ");
    // for(j = 0; j < 6; j++)
    //   putchar(packet[ps+j]);
    // printf("\n");
    if (packet[ps] == '>')
    {
      return ret;
    }
    else if((ps+3 < size) && (strncmp(&(packet[ps]), "AT", 2) == 0))
    {
      // printf("This is AT command.\n");
      for(j = ps; j < size && packet[j] != '\r'; j++); // AT command ends with \r\n

      if (j != size)
      {
        ret.pevent.ev = COMMAND_SENT;
        ret.pevent.data = calloc(0, j-ps);
        memcpy(ret.pevent.data, &(packet[ps]), j-ps);
        ret.parsed = j + 1;
      }
    }
    else if((ps+2 < size) && (strncmp(&(packet[ps]), "OK", 2) == 0))
    {
      // printf("This is OK.\n");
      ret.pevent.ev = COMMAND_RESPONSE;
      ret.pevent.data = malloc(2);
      strncpy(ret.pevent.data, "OK", 2);
      ret.parsed = ps + 2;
    }
    else if((ps+19 < size) && (strncmp(&(packet[ps]), "BCAST:", 6) == 0))
    {
      // printf("This is message from another device.\n");
      ret.pevent.ev = NEW_PACKET;
      zgbp = malloc(sizeof(zigbee_packet));
      memset(zgbp->eui, 0, 9);
      memcpy(zgbp->eui, &(packet[ps+6]), 8);
      ps += 15;
      char temp[3];
      temp[2] = '\0';
      memcpy(temp, &(packet[ps]), 2);
      zgbp->size = (int)strtol(temp, NULL, 16);
      ret.parsed = ps + zgbp->size + 3;
      zgbp->header_flags = packet[ps+3];
      zgbp->number = (uint8_t)packet[ps+5];
      ps += 6;
      zgbp->id = 0;
      for(j = 0; j < ID_SIZE; j++)
      {
        zgbp->id += ((packet[ps+j])&0xff) << (8*j);
      }

      ps += ID_SIZE;

      zgbp->op = packet[ps];

      memset(zgbp->packet_data, 0, FRAME_SIZE);

      memcpy(zgbp->packet_data, &(packet[ps+2]), zgbp->size);

      ret.pevent.data = zgbp;
    }
    else if((ps+8 > size) && (strncmp(&(packet[ps]), "ERROR:", 6) == 0))
    {
      // printf("This is error.\n");
      ret.pevent.ev = COMMAND_RESPONSE;
      ret.pevent.data = malloc(8);
      strncpy(ret.pevent.data, &(packet[ps]), 8);
      ret.parsed = ps + 8;
    }
  }
  else
  {
    // printf("Packet start was not found.\n");
  }
  return ret;
}
#else
ParsedData parse_received_data(const char* packet, int size)
{
  int j; // packet start

  ParsedData ret = { .pevent = { .ev = NO_EVENT , .data = NULL }, .parsed = 0 };

  zigbee_packet* zgbp;

  if (packet == NULL)
  {
    printf("Packet is NULL.\n");
    return ret;
  }

  if (size > HEADER_SIZE)
  {
    ret.pevent.ev = NEW_PACKET;
    zgbp = malloc(sizeof(zigbee_packet));
    zgbp->op = 0;
    for(j = 0; j < OP_SIZE; j++)
    {
      zgbp->op += ((packet[FLAGS_SIZE + MESSAGE_NUMBER_SIZE + ID_SIZE + j]&0xff) << (8*j));
    }

    if (zgbp->op == 0)
    {
      ret.pevent.ev = NO_EVENT;
      free(zgbp);
      return ret;
    }

    memset(zgbp->eui, 0, 9);
    zgbp->size = size-HEADER_SIZE;
    ret.parsed = size;
    zgbp->header_flags = 0;
    for(j = 0; j < FLAGS_SIZE; j++)
    {
      zgbp->header_flags += ((packet[j]&0xff) << (8*j));
    }
    zgbp->number = (uint8_t)packet[FLAGS_SIZE];
    zgbp->id = 0;

    for(j = 0; j < ID_SIZE; j++)
    {
      zgbp->id += ((packet[j+FLAGS_SIZE+MESSAGE_NUMBER_SIZE])&0xff) << (8*j);
    }
    snprintf(zgbp->eui, 9, "%d", zgbp->id);

    memset(zgbp->packet_data, 0, FRAME_SIZE);

    memcpy(zgbp->packet_data, &packet[HEADER_SIZE], zgbp->size);

    ret.pevent.data = zgbp;
  }
  return ret;
}

#endif

RouteConfig parse_info(char* data, uint32_t size, uint32_t id)
{
  // printf("%d\n", size);
  RouteConfig ret = { .id = id, .speed = 0, .time = 0, //.way = NULL, .way_length = 0, 
    .coords_src = {0,0}, .coords_dst = {0,0}};
  int i;
  for(i = 0; i < size;)
  {
    switch(data[i])
    {
      case NO_FIELD:
        return ret;
      case SPEED:
      {
        ret.speed = parse_speed(&data[i], size-i);
        i += 3+SPEED_SIZE;
        break;
      }
      case TIME:
      {
        ret.time = parse_time(&data[i], size-i);
        i += 3+TIME_SIZE;
        break;
      }
      case COORDS_START:
      {
        if(parse_coords(&data[i], size-i, ret.coords_src) != 0)
          return ret;
        i += 3+2*COORD_SIZE;
        break;
      }
      case COORDS_END:
      {
        if(parse_coords(&data[i], size-i, ret.coords_dst) != 0)
          return ret;
        i += 3+2*COORD_SIZE;
        break;
      }
      // case WAY:
      // {
      //   parse_way(&data[i], size-i, &(ret.way), &(ret.way_length));
      //   i += 3+(ret.way_length)*2*COORD_SIZE;
      //   break;
      // }
      default:
        // printf("Unexpected field in received data %hhd\n", data[i]);
        return ret;
    }
  }
}

static uint32_t parse_speed(const char* raw_data, size_t size)
{
  uint32_t speed = 0;
  int j;
  if(SPEED_SIZE + 3 <= size)
  {
    for (j = 0; j < SPEED_SIZE; j++)
    {
      speed += ((raw_data[j+3])&0xff) << (8*j);
    }
  }
  else
  {
    return -1;
  }
  return speed;
}

static uint32_t parse_time(const char* raw_data, size_t size)
{
  uint32_t time = 0;
  int j;
  if (TIME_SIZE + 3 <= size)
  {
    for (j = 0; j < TIME_SIZE; j++)
    {
      time += ((raw_data[j+3])&0xff) << (8*j);
    }
  }
  else
  {
    return -1;
  }
  return time;
}

static int parse_coords(const char* raw_data, size_t size, uint32_t *coords)
{
  int j, k;
  if (COORD_SIZE*2 <= size)
  {
    for (k = 0; k < 2; k++)
    {
      coords[k] = 0;
      for (j = 0; j < COORD_SIZE; j++)
      {
        coords[k] += (((raw_data[3+k*COORD_SIZE+j])&0xff)) << (8*j);
      }
    }
  }
  else
  {
    return -1;
  }
  return 0;
}
/*
static int parse_way(const char* raw_data, size_t size, uint32_t **way, uint32_t *way_length)
{
  int j, k;
  if (3 < size)
  {
    *way_length = 0;
    for(j = 0; j < SIZE_SIZE; j++)
    {
      *way_length += ((raw_data[1+j])&0xff) << (8*j);
    }
    if (3 + (*way_length)*2 <= size)
    {
      *way = (uint32_t*)calloc((*way_length)*2, 4);
      for (k = 0; k < (*way_length)*2; k++)
      {
        for (j = 0; j < COORD_SIZE; j++)
        {
          (*way)[k] += (((raw_data[3 + k*COORD_SIZE + j])&0xff)) << (8*j);
        }
      }
    }
    else
    {
      // printf("Error in way length, length: %d size: %ld\n", *way_length, size);
      *way_length = 0;
      return -1;
    }
  }
  else
  {
    // printf("Size of data is smaller than size of standard fields\n");
    return -1;
  }
  return 0;
}
*/
#ifndef RF24
static AtCommands get_command_name(const char* str, size_t size)
{
  const char* name;

  if(size < 0)
    return 0;

  if(str[2] == '+')
  {
    name = &(str[3]); // skipping AT+
    size -= 3;

    switch(size)
    {
      case 1:
        {
          if(strncmp(name, "N", 1) == 0)
            return N;
          break;
        }
      case 2:
        {
          if(strncmp(name, "EN", 2) == 0)
            return EN;
          else if(strncmp(name, "JN", 2) == 0)
            return JN;
          break;
        }
      case 5:
        {
          if(strncmp(name, "DASSL", 5) == 0)
            return DASSL;
          break;
        }
      case 6:
        {
          if(strncmp(name, "BCASTB", 6) == 0)
            return BCASTB;
          break;
        }
    }
  }
  else
  {
    if(size == 3)
    {
      strncmp(str, "ATI", 3);
    }
  }
  return NO_COMMAND;
}
#endif




