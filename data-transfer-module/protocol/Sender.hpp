#ifndef SENDER_HPP
#define SENDER_HPP

#include <cstddef>

#include "protocol_encode.h"
#include "../zigbee/device.hpp"

class Sender
{
public:
  ~Sender() {}
  Sender() {}
  Sender(const char *path);
  Sender(int fd);
  Sender(Device*);
  int configure(const char *path);
  int send(const char* array, size_t size, uint32_t from) const;
  int getId() { return fd; }
  Device *device;
  const char *path;
  int fd;
private:
  int send_packets(packets) const;
};

#endif // SENDER_HPP
