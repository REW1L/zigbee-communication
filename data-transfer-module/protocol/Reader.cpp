#include "Reader.hpp"
#include "ProtocolLogger.hpp"

#include <cstring>

#include "Events.h"
#include "protocol.h"
#include "parser.h"

#include "WorkerThread.hpp"
#include "common_functions.h"

// #include <stdio.h>

#if !defined(RF24)
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
#else
Reader::Reader(int nodeID, long long timeout)
{
  LOG_INFO("READER", "Configuring node %d", nodeID);
  this->path = NULL;
  this->timeout = std::chrono::nanoseconds(timeout);
  this->fd = this->device.configure_device(nodeID);
  LOG_INFO("READER", "Configured %s", 
           (fd != 0)? "Master" : "Slave");
  this->stopped = 0;
}
#endif

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
#if !defined(RF24)
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

#else

int Reader::restart(int nodeID, long long timeout)
{
  this->stopped = 1;
  this->reader.join();
  this->path = NULL;
  this->timeout = std::chrono::nanoseconds(timeout);
  this->fd = configure_device(nodeID);
  this->stopped = 0;

  return this->run();
}
#endif

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
  char* buffer = new char[1000];
  const char* preamble = PREAMBLE;
  int bytes_received = 0, offset = 0, preamble_start;
  while(!stopped)
  {
    bytes_received = this->device->read_from_device(buffer+offset, 100);
    offset+=bytes_received;
    if(offset >= PREAMBLE_SIZE)
    {
      preamble_start = find_subarray(buffer, offset, preamble, PREAMBLE_SIZE);
      if( preamble_start != -1
        && offset-preamble_start >= PREAMBLE_SIZE + FRAME_SIZE + CRC16_SIZE + sizeof(uint64_t)*2)
      {
        uint16_t crc = *(uint16_t*)(&(buffer[preamble_start+PREAMBLE_SIZE+FRAME_SIZE+sizeof(uint64_t)*2]));
        if(pr_crc16(&(buffer[preamble_start]), PREAMBLE_SIZE+FRAME_SIZE) 
          == crc)
        {
          LOG_INFO("READER", "Valid frame received, crc: 0x%X", crc);
          Event ev = { .ev = NEW_FRAME, .data = malloc(sizeof(proto_frame)) };
          proto_frame* pf = (proto_frame*)ev.data;
          pf->data = malloc(FRAME_SIZE+sizeof(uint64_t)*2);
          pf->size = FRAME_SIZE+sizeof(uint64_t)*2;
          memcpy(pf->data, &(buffer[preamble_start+PREAMBLE_SIZE]), FRAME_SIZE+sizeof(uint64_t)*2);
          WorkerThread::add_event(ev);

          memcpy(buffer, &(buffer[preamble_start+PREAMBLE_SIZE+FRAME_SIZE+sizeof(uint64_t)*2]), 
            offset-(preamble_start+PREAMBLE_SIZE+FRAME_SIZE+sizeof(uint64_t)*2));
          memset(&(buffer[offset-(preamble_start+PREAMBLE_SIZE+FRAME_SIZE+sizeof(uint64_t)*2)]), 0, 400);
          offset = offset-(preamble_start+PREAMBLE_SIZE+FRAME_SIZE+sizeof(uint64_t)*2);
        }
        else
        {
          LOG_INFO("READER", "Not valid frame received, crc: 0x%X", crc);
          memcpy(buffer, &(buffer[preamble_start+PREAMBLE_SIZE]), 
            offset-(preamble_start+PREAMBLE_SIZE));
          memset(&(buffer[offset-(preamble_start+PREAMBLE_SIZE)]), 0, 400);
          offset = offset-(preamble_start+PREAMBLE_SIZE);
        }
      }
      else
      {
        // LOG_INFO("READER", "PREAMBLE didn't find, offset: %d", offset);
      }
    }
    if(offset >= 400)
    {
      LOG_INFO("READER", "Cleaning buffer, offset: %d", offset);
      memcpy(buffer, &(buffer[offset-200]), 200);
      memset(&(buffer[200]), 0, 200);
      offset = 0;
    }
    std::this_thread::sleep_for(this->timeout);
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
