#ifndef READER_THREAD_H
#define READER_THREAD_H

#include <thread>
#include <chrono>
#include <cstdint>

#include "../zigbee/device.hpp"

class Reader
{
public:
  Reader(const char *path, long long timeout = 1000);
  Reader(int fd, long long timeout = 1000);
  virtual ~Reader();
  int run();
  int restart(const char *path, long long timeout);
  int restart(int fd, long long timeout);
  int restart(long long timeout);
  int stop();
  int getId() { return fd; }
  Device *device;
private: 
  int find_subarray(const char*, int, const char*, int);
  void reader_function();
  std::chrono::nanoseconds timeout;
  const char *path;
  int fd;
  char stopped;
  std::thread reader;
};



#endif // READER_THREAD_H
