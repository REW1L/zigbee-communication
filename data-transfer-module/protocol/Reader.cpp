#include "Reader.hpp"
#include "ProtocolLogger.hpp"

#include <cstring>

#include "Events.h"
#if !defined(RF24)
  #include "../zigbee/serial_transactions.h"
#else
  #include "../RF24/device_actions.hpp"
#endif
#include "protocol.h"
#include "parser.h"

#include "WorkerThread.hpp"

#include <stdio.h>

#if !defined(RF24)
Reader::Reader(const char *path, long long timeout)
{
  this->path = path;
  this->timeout = std::chrono::nanoseconds(timeout);
  this->fd = configure_device(path);
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
  this->fd = configure_device(nodeID);
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
  this->fd = configure_device(path);
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
  int bytes_received = 0, offset = 0, old_end = 0;
  char read_forward = 1, trigger = 1;
  ParsedData pd;
  while(!stopped)
  {
    // if(read_forward)
    // {
    //   bytes_received = 0;
    //   if(trigger)
    //   {
    //     trigger = 0;
    //   }
      
    //   bytes_received = read_from_device(this->fd, &(buffer[old_end]), 500);

    //   if(bytes_received)
    //   {
    //     old_end += bytes_received;
    //     if(old_end > 500)
    //     {
    //       memcpy(buffer, &(buffer[offset]), old_end-offset);
    //       old_end = bytes_received;
    //       offset = 0;
    //     }
    //   }
    // }
    // else
    // {
    //   if(trigger)
    //   {
    //     trigger = 0;
    //   }
    // }
    bytes_received = read_from_device(this->fd, buffer, 500);
    if(bytes_received)
    {
      trigger = 1;
      read_forward = 0;
      LOG_INFO("READER", "Bytes received: %d", bytes_received);

      pd = parse_received_data(buffer, bytes_received);

      if(pd.pevent.ev != NO_EVENT)
      {
        WorkerThread::add_event(pd.pevent);
      }
      // else
      //   read_forward = 1;

      // if(pd.parsed < (1000-offset))
      // {
      //   offset += pd.parsed;
      // }
      // else
      // {
      //   old_end = 0;
      //   offset = 0;
      // }
    }
    std::this_thread::sleep_for(this->timeout);
  }
  delete[] buffer;
}