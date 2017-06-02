#include "Sender.hpp"
#include "ProtocolLogger.hpp"
#include "common_functions.h"

#if !defined(RF24)
  #include "../zigbee/device.hpp"
#else
  #include "../RF24/device.hpp"
#endif

#include <string.h>
#include <stdio.h>

#ifndef RF24
Sender::Sender(int fd)
{
  this->fd = fd;
}

Sender::Sender(Device *dev)
{
  this->device = dev;
  LOG_INFO("SENDER", "Configured with MAC: 0x%llX", this->device->mac);
}

Sender::Sender(const char *path)
{
  this->configure(path);
}

int Sender::configure(const char *path)
{
  this->path = path;
  this->fd = this->device->configure_device(path);
  return this->fd;
}
#endif

int Sender::send(RouteConfig inf)
{
  char temp_frame[FRAME_SIZE+PREAMBLE_SIZE+2+sizeof(uint64_t)*2+30];
  // if (fd < 0)
  // {
  //   LOG_ERROR("SENDER", "Device was not configured. fd: %d", fd);
  //   return 1;
  // }
  LOG_INFO("SENDER", "Send RouteConfig id: %u coords_src: [%u, %u], "
               "coords_dst: [%u, %u], speed: %u, time: %u", 
               inf.id, inf.coords_src[0], inf.coords_src[1], inf.coords_dst[0],
               inf.coords_dst[1], inf.speed, inf.time);

  packets ep = pack_info(inf, 0);

  LOG_INFO("SENDER", "Sending %d packets", ep.number);

  // TODO: make sending packets more stable
  int temp;
  memcpy(temp_frame, PREAMBLE, PREAMBLE_SIZE);
  *(uint64_t*)(&temp_frame[PREAMBLE_SIZE]) = this->device->mac;
  *(uint64_t*)(&temp_frame[PREAMBLE_SIZE+sizeof(uint64_t)]) = 0xFFFFFFFFFFFFFFFFLL; // broadcast forever
  for (int i = 0; i < ep.number; i++)
  {
    memcpy(&(temp_frame[PREAMBLE_SIZE+sizeof(uint64_t)*2]), ep.data[i], FRAME_SIZE);
    *(uint16_t*)(&temp_frame[PREAMBLE_SIZE+sizeof(uint64_t)*2+FRAME_SIZE]) = pr_crc16(temp_frame, FRAME_SIZE+PREAMBLE_SIZE);
    temp = this->device->send_frame(temp_frame, FRAME_SIZE+PREAMBLE_SIZE+2+sizeof(uint64_t)*2);
    LOG_INFO("SENDER", "Sent bytes: %d", temp);
  }
  free(ep.data);
  free(ep.raw_data);

  return 0;
}

