#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <cstdlib>
#include <cstdint>

class Device {
public:
  Device() { this->mac = 0xFFFFFFFF; }
  virtual ~Device() {}
  Device(const char *device) { this->configure_device(device); }

  int configure_device(const char *device);

  int send_frame(char *array, int size);

  int read_from_device(char *buffer, int size);

  int send(char*, int);

  uint64_t get_id() { return mac; }
// private:
  uint64_t mac;
  int fd;
};

#endif /* DEVICE_HPP */
