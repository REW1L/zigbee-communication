#include "UserListener.hpp"
#include "ProtocolLogger.hpp"

void UserListener::notify(Event ev)
{
  switch(ev.ev)
  {
    case INFO:
    {
      RouteConfig inf = *(RouteConfig*)(ev.data);
      LOG_INFO("USR_LIST", "Notify RouteConfig id: %u coords_src: [%u, %u], "
               "coords_dst: [%u, %u], speed: %u, time: %u", 
               inf.id, inf.coords_src[0], inf.coords_src[1], inf.coords_dst[0],
               inf.coords_dst[1], inf.speed, inf.time);
      delete (RouteConfig*)ev.data;
      this->notifyRouteConfig(inf);
    }
    default: return;
  }
}