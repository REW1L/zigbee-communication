#include "Sender.hpp"

#if !defined(RF24)
  #include "../zigbee/serial_transactions.h"
#else
  #include "../RF24/device_actions.hpp"
#endif

#include <stdio.h>

#ifndef RF24
Sender::Sender(int fd)
{
  this->fd = fd;
}

Sender::Sender(const char *path)
{
  this->configure(path);
}

int Sender::configure(const char *path)
{
  this->path = path;
  this->fd = configure_device(path);
  return this->fd;
}
#endif

int Sender::send(RouteConfig inf)
{
  if (fd < 0)
  {
    printf("Device was not configured.\n");
    return 1;
  }
  packets ep = pack_info(inf, 0);

  // TODO: make sending packets more stable

  for (int i = 0; i < ep.number; i++)
  {
    send_frame(fd, ep.data[i], FRAME_SIZE);
  }
  return 0;
}

int Sender::net_info()
{
  return get_network_info(this->fd);
}
int Sender::join_net()
{
  return join_network(this->fd);
}
int Sender::exit_net()
{
  return exit_network(this->fd);
}
int Sender::create_net()
{
  return create_network(1);
}

