#ifndef USERLISTENER_HPP
#define USERLISTENER_HPP

#include "Events.h"
#include "Listener.hpp"
#include "protocol.h"
#include "RawDataPacket.hpp"

class UserListener : public Listener
{
public:
  virtual void notifyRouteConfig(RouteConfig inf) = 0;
  virtual void notifyRawData(RawDataPacket*) = 0;
  void notify(Event);
};

#endif // USERLISTENER_HPP
