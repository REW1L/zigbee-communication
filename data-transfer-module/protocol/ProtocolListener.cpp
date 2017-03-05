#include "WorkerThread.hpp"
#include "ProtocolListener.hpp"
#include "parser.h"

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
    default: return;
  }
}


int ProtocolListener::new_packet(zigbee_packet* zgbp)
{
  // TODO: make compression
  if(zgbp->header_flags & COMPRESS_5000)
  {
    // printf("Compression is not done yet.\n");
    return 1;
  }
  if(packets_lists.empty())
  {
    packets_lists.push_back(new std::list<zigbee_packet*>());
    packets_lists.back()->push_back(zgbp);
  }
  for(std::list<zigbee_packet*> *lst : packets_lists)
  {
    if(strncmp(lst->front()->eui, zgbp->eui, 8) == 0)
    {
      if(lst->front() != zgbp)
        lst->push_back(zgbp);
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
        packets_lists.remove(lst);
        if (sum*2 == (i * (i-1)))
        {
          lst->sort([](zigbee_packet *zp1, zigbee_packet *zp2) -> bool
            {
              return zp1->number < zp2->number;
            });
          char *raw_data = new char[i*FRAME_SIZE];

          // TODO: make compression
          i = 0;
          for(zigbee_packet *zg : *lst)
          {
            memcpy(&(raw_data[FRAME_WITHOUT_HEADER*i]),
              zg->packet_data, FRAME_WITHOUT_HEADER);
            i++;
            delete zg;
          }

          this->parse_op(lst->front(), raw_data, i*FRAME_SIZE);
        }
        else
        {
          // TODO: make ask for resend
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

int ProtocolListener::parse_op(zigbee_packet* zgbp, char* data, size_t size)
{
  switch(zgbp->op)
  {
    case INFO:
    {
      RouteConfig *inf = new RouteConfig;
      *inf = parse_info(data, size, zgbp->id);
      delete data;
      Event ev = { .ev = INFO, .data = inf };
      WorkerThread::add_event(ev);
      break;
    }
    default: return 1;
  }
  return 0;
}




