#ifndef READER_THREAD_H
#define READER_THREAD_H

#include <thread>
#include <chrono>
#if !defined(RF24)
  #include "../zigbee/device.hpp"
#else
  #include "../RF24/device.hpp"
#endif
#include <cstdint>

class Reader
{
public:
  #ifndef RF24
    Reader(const char *path, long long timeout = 1000);
    Reader(int fd, long long timeout = 1000);
  #else
    Reader(int nodeID, long long timeout = 1000);
  #endif
  virtual ~Reader();
  int run();
  #ifndef RF24
    int restart(const char *path, long long timeout);
    int restart(int fd, long long timeout);
  #else
    int  restart(int nodeID, long long timeout);
  #endif
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
