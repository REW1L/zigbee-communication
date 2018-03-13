#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <cstdlib>
#include <cstdint>
#include "../protocol/protocol.h"

class Device {
public:
  Device() { this->mac = 0xFFFFFFFF; }
  virtual ~Device() {}
  Device(const char *device) { this->configure_device(device); }

  int configure_device(const char *device);

  int send_frame(char *array, int size);

  int read_from_device(char *buffer, int size);

  int send(char*, int);

  int get_available_bytes();

  int read_frame(char* buffer, int max_size);

  uint64_t mac;
  int fd;

private:
  int _send_command(const char* cmd, uint8_t cmd_size);
  char calc_checksum(const char* data, int size);
};

#endif /* DEVICE_HPP */
