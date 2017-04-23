#ifndef PROTOCOL_LISTENER_H
#define PROTOCOL_LISTENER_H

#include <vector>
#include <list>
#include <cstring>
#include <cstddef>
#include "Listener.hpp"
#include "protocol.h"

class ProtocolListener : public Listener
{
public:
  ProtocolListener() {}
  virtual ~ProtocolListener() {}
  virtual void notify(Event);
private:
  int parse_op(zigbee_packet, char*, size_t);
  int new_packet(zigbee_packet* zgbp);
  std::list<std::list<zigbee_packet*>*> packets_lists; // TODO: tree
};

#endif // PROTOCOL_LISTENER_H