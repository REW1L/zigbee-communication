#include "Sender.hpp"

#include <string.h>


#include "ProtocolLogger.hpp"
#include "common_functions.h"
#include "protocol.h"

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

int Sender::send(const char* array, size_t size, uint32_t from) const
{
  LOG_INFO("SENDER", "Sending size: %d", size);
  packets ep = make_packets(array, size, 0, from, OP_RAW_DATA);
  int successful = this->send_packets(ep);
  free(ep.packet_array[0].data);
  free(ep.packet_array);
  return successful;
}

int Sender::send_packets(packets ep) const
{
  char temp_frame[FRAME_SIZE+2+sizeof(uint64_t)*2+30];

  LOG_INFO("SENDER", "Sending %d packets", ep.number);

  // TODO: make sending packets more stable
  int temp;
  *(uint64_t*)(temp_frame) = this->device->mac;
  *(uint64_t*)(&temp_frame[sizeof(uint64_t)]) = 0xFFFFFFFFFFFFFFFFLL; // broadcast forever
  for (int i = 0; i < ep.number; i++)
  {
    temp = ep.packet_array[i].size;
    LOG_INFO("SENDER", "Payload size: %d", temp);
    LOG_DEBUG("SENDER", "First 3 bytes of data: %02hhX %02hhX %02hhX",
              ep.packet_array[i].data[0], ep.packet_array[i].data[1], ep.packet_array[i].data[2]);
    memcpy(&(temp_frame[sizeof(uint64_t) * 2]), ep.packet_array[i].data, (uint16_t)temp);
    *(uint16_t*)(&temp_frame[sizeof(uint64_t) * 2 + temp])= pr_crc16(temp_frame, (uint16_t)temp + sizeof(uint64_t) * 2);
    temp = this->device->send_frame(temp_frame, temp + 2 + sizeof(uint64_t) * 2);
    LOG_INFO("SENDER", "Sent bytes: %d", temp);
  }

  return 0;
}