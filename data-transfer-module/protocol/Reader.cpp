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

// #include <stdio.h>

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
  ParsedData pd;
  int bytes_received, offset = 0;
  while(!stopped)
  {
    bytes_received = read_from_device(this->fd, buffer+offset, 100);
    if(bytes_received+offset >= FRAME_SIZE)
    {
      LOG_INFO("READER", "Full frame received: %d bytes received", bytes_received);

      pd = parse_received_data(buffer, FRAME_SIZE);

      if(pd.pevent.ev != NO_EVENT)
      {
        WorkerThread::add_event(pd.pevent);
      }
      memcpy(buffer, buffer+FRAME_SIZE, bytes_received+offset-FRAME_SIZE);
      offset = bytes_received+offset-FRAME_SIZE;
    }
    else if(bytes_received)
    {
      LOG_INFO("READER", "Forward reading, received: %d bytes", bytes_received);
      offset+=bytes_received;
    }
    std::this_thread::sleep_for(this->timeout);
  }
  delete[] buffer;
}