#ifndef USERLISTENER_H
#define USERLISTENER_H

#include "Events.h"
#include "Listener.hpp"
#include "protocol.h"

class UserListener : public Listener
{
public:
  virtual void notifyRouteConfig(RouteConfig inf) = 0;
  void notify(Event);
};

#endif // USERLISTENER_H
