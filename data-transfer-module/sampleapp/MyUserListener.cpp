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
   * when full data about one node was received
   */
  virtual void notifyRouteConfig(RouteConfig);
  /**
   * Callback function which will be called
   * when full raw data packet was received
   */
  virtual void notifyRawData(RawDataPacket*);
};

void MyUserListener::notifyRouteConfig(RouteConfig inf)
{
  printf("Info: \ncar_id: %x \ndst: [%d:%d] \nsrc: [%d:%d] \nspeed: %d \ntime: %u\n", 
    inf.id, inf.coords_dst[0], inf.coords_dst[1], inf.coords_src[0], inf.coords_src[1],
    inf.speed, inf.time);
}

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
