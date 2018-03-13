#include "WorkerThread.hpp"
#include "ProtocolListener.hpp"
#include "ProtocolLogger.hpp"
#include "protocol.h"
#include "RawDataPacket.hpp"
#include "common_functions.h"

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
  LOG_INFO("PC_LISTENER", "New packet: flags: %02hhX number: %d id: %u size: %d op: %d from: %llX",
    zgbp->header_flags, zgbp->number, zgbp->id, zgbp->size, zgbp->op, zgbp->from);

  zigbee_packet* packet = new zigbee_packet;
  memcpy(packet, zgbp, sizeof(zigbee_packet));
  memcpy(packet->packet_data, zgbp->packet_data, zgbp->size);

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
      uint32_t sum = 0, i = 0; // first message has zero in number
      zigbee_packet* max = lst->front();
      char end_found = 0;
      for (zigbee_packet *zg : *lst)
      {
        i++;
        sum += zg->number;
        end_found |= (zg->header_flags & LAST_MESSAGE);
      }
      if(end_found)
      {
        LOG_INFO("PC_LISTENER", "Got end of list of packets, id: %d", max->id);
        packets_lists.remove(lst);
        if (sum << 1 == (i * (i-1)))
        {
          lst->sort([](zigbee_packet *zp1, zigbee_packet *zp2) -> bool
            {
              return zp1->number < zp2->number;
            });
          i = 0;
          for(zigbee_packet *zg : *lst)
            i += zg->size;
          char *raw_data = (char*)calloc(1, i);
          LOG_INFO("PC_LISTENER", "Got full list of packets number: %d id: %d size: %d",
                   lst->size(), max->id, i);

          zigbee_packet to_parse = *(lst->front());
          i = 0;
          for(zigbee_packet *zg : *lst)
          {
            memcpy(&raw_data[i], zg->packet_data, zg->size);
            i += zg->size;
            delete zg;
          }

          this->parse_op(to_parse, raw_data, (size_t)i);
        }
        else
        {
          // TODO: make ask for resend
          LOG_INFO("PC_LISTENER", "Corrupted packet list id: %d sum: %d last: %d", max->id, sum, i);
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
    case OP_RAW_DATA:
    {
      LOG_INFO("PC_LISTENER", "Received full packet with raw data, size: %d, id: %d", size, zgbp.id);
      char* array = new char[size];
      memcpy(array, data, size);
      RawDataPacket *packet = new RawDataPacket(size, array);
      Event ev = { .ev = RECEIVED_RAW_DATA_PACKET, .data = packet };
      WorkerThread::add_event(ev);
      break;
    }
    case OP_POSITIONING:
    {
      LOG_INFO("PC_LISTENER", "Received packet with positioning data (%d), RSSI: %d", size, zgbp.rssi);
      char* array = new char[size+1];
      memcpy(&array[1], data, size);
      array[0] = zgbp.rssi;
      Event ev = { .ev = RECEIVED_POSITIONING_DATA, .data = array };
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
    LOG_ERROR("PC_LISTENER", "Proto frame is null: 0x%X", pf);
    return -1;
  }

  LOG_INFO("PC_LISTENER", "Got proto frame with size %d", pf->size);

  if (pf->size <= HEADER_SIZE)
    return -1;
  LOG_DEBUG("PC_LISTENER", "First 3 bytes of packet: %02hhX %02hhX %02hhX",
            ((char*)(pf->data))[0], ((char*)(pf->data))[1], ((char*)(pf->data))[2]);
  LOG_DEBUG("PC_LISTENER", "Last 6 bytes of packet: %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX",
            ((char*)(pf->data))[pf->size - 6], ((char*)(pf->data))[pf->size - 5], ((char*)(pf->data))[pf->size - 4],
            ((char*)(pf->data))[pf->size - 3], ((char*)(pf->data))[pf->size - 2],
            ((char*)(pf->data))[pf->size - 1]);
  uint16_t expected_crc = pr_crc16(pf->data, pf->size-3);
  uint16_t actual_crc = *((uint16_t*)((uint64_t)pf->data + pf->size - 3));

  if( actual_crc != expected_crc && actual_crc != 0x2121) // magic number for positioning
  {
    LOG_WARNING("PC_LISTENER", "Incorrect crc in packet: 0x%X(expected) 0x%X(actual)",
                expected_crc, actual_crc);
    return 1;
  }

  zgbp = (zigbee_packet*)malloc(sizeof(zigbee_packet));
  zgbp->rssi = pf->rssi;
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
    LOG_WARNING("PC_LISTENER", "Operation id is not supported: %d src: %X dst: %X", zgbp->op, zgbp->from, zgbp->to);
    free(zgbp);
    return -1;
  }

  zgbp->size = pf->size-offset-3; // size - (header + crc)

  memset(zgbp->packet_data, 0, zgbp->size);
  memcpy(zgbp->packet_data, (void*)((char*)(pf->data)+offset), zgbp->size);
  Event ev = { .ev = NEW_PACKET, .data = zgbp };
  WorkerThread::add_event(ev);
  return 0;
}


