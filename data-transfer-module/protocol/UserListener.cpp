#include "UserListener.hpp"

void UserListener::notify(Event ev)
{
  switch(ev.ev)
  {
    case INFO:
    {
      RouteConfig inf = *(RouteConfig*)(ev.data);
      delete (RouteConfig*)ev.data;
      this->notifyRouteConfig(inf);
    }
    default: return;
  }
}