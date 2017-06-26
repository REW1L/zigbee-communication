#ifndef LISTENER_H
#define LISTENER_H

#include "Events.h"

class Listener
{
public:
  virtual void notify(Event) = 0;
};

#endif // LISTENER_H
