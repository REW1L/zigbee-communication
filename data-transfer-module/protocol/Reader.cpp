#include "Reader.hpp"
#include "ProtocolLogger.hpp"

#include <cstring>

#include "Events.h"
#include "protocol.h"

#include "WorkerThread.hpp"
#include "common_functions.h"

// #include <stdio.h>

Reader::Reader(const char *path, long long timeout)
{
  this->path = path;
  this->timeout = std::chrono::nanoseconds(timeout);
  this->device = new Device(path);
  this->stopped = 0;
}

Reader::Reader(int fd, long long timeout)
{
  this->path = NULL;
  this->timeout = std::chrono::nanoseconds(timeout);
  this->fd = fd;
  this->stopped = 0;
}

Reader::~Reader()
{
  this->stopped = 1;
}

int Reader::run()
{
  if(this->fd < 0)
  {
    LOG_ERROR("READER", "Impossible to connect with device fd: %d", fd);
    return 1;
  }
  this->reader = std::thread(&Reader::reader_function, this);

  return 0;
}

int Reader::restart(const char *path, long long timeout)
{
  this->stopped = 1;
  this->reader.join();
  this->path = path;
  this->timeout = std::chrono::nanoseconds(timeout);
  this->fd = this->device->configure_device(path);
  this->stopped = 0;

  return this->run();
}

int Reader::restart(int fd, long long timeout)
{
  this->stopped = 1;
  this->reader.join();
  this->path = NULL;
  this->timeout = std::chrono::nanoseconds(timeout);
  this->fd = fd;
  this->stopped = 0;

  return this->run();
}

int Reader::restart(long long timeout)
{
  this->stopped = 1;
  this->reader.join();
  this->timeout = std::chrono::nanoseconds(timeout);
  this->stopped = 0;
  return this->run();
}

int Reader::stop()
{
  this->stopped = 1;
  this->reader.join();
  return 0;
}

void Reader::reader_function()
{
  char* buffer = new char[300];
  int read_frame_rezult, size;
  proto_frame *pf;
  while(!stopped)
  {
//    std::this_thread::sleep_for(this->timeout);
    read_frame_rezult = this->device->read_frame(buffer, 255);
    if(read_frame_rezult)
      continue;

    size = (((uint16_t)(buffer[1] & 0xFF) << 8) + (buffer[2] & 0xFF));
    if (buffer[3] == (char)0x80)
    {
      Event ev = {.ev = NEW_FRAME, .data = new proto_frame};
      pf = (proto_frame *) ev.data;
      pf->data = new char[size - 10]; // size - (from address + checksum + XBEE operation + RSSI)
      pf->size = size-10;
      pf->rssi = (uint8_t)buffer[12];
      memcpy(pf->data, &buffer[14], pf->size);
      WorkerThread::add_event(ev);
    }
  }
  delete[] buffer;
}

int Reader::find_subarray(const char* where, int size_of_source, const char* what, int size)
{
  for(int i = 0; i < size_of_source-size; i++)
  {
    if(where[i] == what[0])
    {
      if(memcmp(&(where[i]), what, size) == 0)
      {
        return i;
      }
    }
  }
  return -1;
}
