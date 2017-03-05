#ifndef SENDER_HPP
#define SENDER_HPP

#include <cstddef>
#include "protocol_encode.h"

class Sender
{
public:
  ~Sender() {}
  #ifndef RF24
  Sender() {}
  Sender(const char *path);
  Sender(int fd);
  int configure(const char *path);
  #else
  Sender() { this->fd = 1; }
  #endif
  int send(RouteConfig inf);
  int net_info();
  int join_net();
  int exit_net();
  int create_net();
  int getId() { return fd; }
private:
  const char *path;
  int fd;
};

#endif // SENDER_HPP