#include "WorkerThread.hpp"
#include "ProtocolListener.hpp"
#include "ProtocolLogger.hpp"
#include "parser.h"
#include "protocol.h"
#include "RawDataPacket.hpp"

#include <cstdio>

void ProtocolListener::notify(Event ev)
{
  switch(ev.ev)
  {
    case NEW_PACKET:
    {
      this->new_packet((zigbee_packet*)(ev.data));
      break;
    }
    case NEW_FRAME:
    {
      this->new_frame((proto_frame*)ev.data);
      break;
    }
    default: return;
  }
}


int ProtocolListener::new_packet(zigbee_packet* zgbp)
{
  LOG_INFO("PC_LISTENER", "New packet: flags: %02hhX number: %d id: %u size: %d op: %d",
    zgbp->header_flags, zgbp->number, zgbp->id, zgbp->size, zgbp->op);

  zigbee_packet* packet = new zigbee_packet;
  memcpy(packet, zgbp, sizeof(zigbee_packet));
  memcpy(packet->packet_data, zgbp->packet_data, zgbp->size);

  // TODO: make compression
  if(packet->header_flags & COMPRESS_5000)
  {
    LOG_WARNING("PC_LISTENER", "Compression is not done yet. %d", 1);
    return 1;
  }
  if(packets_lists.empty())
  {
    packets_lists.push_back(new std::list<zigbee_packet*>());
    packets_lists.back()->push_back(packet);
  }
  for(std::list<zigbee_packet*> *lst : packets_lists)
  {
    if(lst->front()->from == packet->from)
    {
      LOG_INFO("PC_LISTENER", "GOT PACKET FROM: 0x%llX", packet->from);
      if(lst->front() != packet)
        lst->push_back(packet);
      long sum = 0, i = 0; // first message has zero in number
      zigbee_packet* max = lst->front();
      char end_found = 0;
      for (zigbee_packet *zg : *lst)
      {
        i++;
        // if(max->number < zg->number)
        //   max = zg;
        sum += zg->number;
        end_found |= (zg->header_flags & LAST_MESSAGE);
      }
      if(end_found)
      {
        LOG_INFO("PC_LISTENER", "Got end of list of packets, id: %d", max->id);
        packets_lists.remove(lst);
        if (sum*2 == (i * (i-1)))
        {
          LOG_INFO("PC_LISTENER", "Got full list of packets, id: %d", max->id);
          lst->sort([](zigbee_packet *zp1, zigbee_packet *zp2) -> bool
            {
              return zp1->number < zp2->number;
            });
          char *raw_data = new char[i*FRAME_SIZE];

          // TODO: make compression
          zigbee_packet to_parse = *(lst->front());
          i = 0;
          for(zigbee_packet *zg : *lst)
          {
            memcpy(&(raw_data[FRAME_WITHOUT_HEADER*i]),
              zg->packet_data, FRAME_WITHOUT_HEADER);
            i++;
            delete zg;
          }

          this->parse_op(to_parse, raw_data, i*FRAME_SIZE);
        }
        else
        {
          // TODO: make ask for resend
          LOG_INFO("PC_LISTENER", "Corrupted packet list id: %d", max->id);
          for(zigbee_packet *zg : *lst)
          {
            delete zg;
          }
        }
        delete lst;
        return 0;
      }
    }
  }
  return 0;
}

int ProtocolListener::parse_op(zigbee_packet zgbp, char* data, size_t size)
{
  switch(zgbp.op)
  {
    case OP_INFO:
    {
      LOG_INFO("PC_LISTENER", "Received full packet for RouteConfig, size: %d, id: %d", size, zgbp.id);
      RouteConfig *inf = new RouteConfig;
      *inf = parse_info(data, size, zgbp.id);
      LOG_INFO("PC_LISTENER", "Parsed RouteConfig id: %u coords_src: [%u, %u], "
         "coords_dst: [%u, %u], speed: %u, time: %u", 
         inf->id, inf->coords_src[0], inf->coords_src[1], inf->coords_dst[0],
         inf->coords_dst[1], inf->speed, inf->time);
      Event ev = { .ev = INFO, .data = inf };
      WorkerThread::add_event(ev);
      break;
    }
    case OP_RAW_DATA:
    {
      LOG_INFO("PC_LISTENER", "Received full packet with raw data, size: %d, id: %d", size, zgbp.id);
      char* array = new char[size];
      memcpy(array, data, size);
      RawDataPacket *packet = new RawDataPacket(zgbp.id, size, array);
      Event ev = { .ev = RECEIVED_RAW_DATA_PACKET, .data = packet };
      WorkerThread::add_event(ev);
      break;
    }
    default: return 1;
  }
  delete data;
  return 0;
}

int ProtocolListener::new_frame(proto_frame* data)
{
  proto_frame* pf = data;
  zigbee_packet* zgbp;
  int offset;

  if (data == NULL)
  {
    // printf("Packet is NULL.\n");
    return -1;
  }

  LOG_INFO("PC_LISTENER", "Got proto frame with size %d", pf->size);

  if (pf->size > HEADER_SIZE)
  {
    zgbp = (zigbee_packet*)malloc(sizeof(zigbee_packet));
    zgbp->from = *(uint64_t*)(pf->data);
    offset = sizeof(uint64_t);
    zgbp->to = *(uint64_t*)((char*)(pf->data)+offset);
    offset += sizeof(uint64_t);
    zgbp->header_flags = *(uint8_t*)((char*)(pf->data)+offset);
    offset += sizeof(uint16_t);
    zgbp->number = *(uint8_t*)((char*)(pf->data)+offset);
    offset += sizeof(uint8_t);
    zgbp->id = *(uint32_t*)((char*)(pf->data)+offset);
    offset += sizeof(uint32_t);
    zgbp->op = *(uint8_t*)((char*)(pf->data)+offset);
    offset += sizeof(uint8_t);

    if (zgbp->op == 0)
    {
      free(zgbp);
      return -1;
    }

    zgbp->size = pf->size-offset;

    memset(zgbp->packet_data, 0, FRAME_SIZE);
    memcpy(zgbp->packet_data, (void*)((char*)(pf->data)+offset), FRAME_SIZE);
    Event ev = { .ev = NEW_PACKET, .data = zgbp };
    WorkerThread::add_event(ev);
  }
  return 0;
}


