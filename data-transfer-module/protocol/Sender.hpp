#ifndef SENDER_HPP
#define SENDER_HPP

#include <cstddef>
#include "protocol_encode.h"
#if !defined(RF24)
  #include "../zigbee/device.hpp"
#else
  #include "../RF24/device.hpp"
#endif

class Sender
{
public:
  ~Sender() {}
  #ifndef RF24
  Sender() {}
  Sender(const char *path);
  Sender(int fd);
  Sender(Device*);
  int configure(const char *path);
  #else
  Sender() { this->fd = 1; }
  #endif
  int send(RouteConfig inf);
  int send(char* array, size_t size, uint32_t from);
  int getId() { return fd; }
// private:
  Device *device;
  const char *path;
  int fd;
};

#endif // SENDER_HPP
