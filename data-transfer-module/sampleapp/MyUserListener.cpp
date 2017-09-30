/*
 * Example of user listener
 *
 *
 */


#include "../protocol/UserListener.hpp"
#include <cstdio>

class MyUserListener : public UserListener
{
public:
  MyUserListener() {}
  virtual ~MyUserListener() {}
  /**
   * Callback function which will be called
   * when full raw data packet was received
   */
  virtual void notifyRawData(RawDataPacket*);
};

void MyUserListener::notifyRawData(RawDataPacket * packet)
{
  int i;
  for (i = 0; i < packet->size; i ++)
  {
    if(packet->data[i] < 32 || packet->data[i] > 126)
      break;
  }
  packet->data[i] = 0;
  printf("Got some string with size %ld (%d readable bytes): %s\n", packet->size, i, packet->data);
  delete packet;
}
