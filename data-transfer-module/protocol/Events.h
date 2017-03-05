#ifndef EVENTS_H
#define EVENTS_H

typedef enum
{
  NO_EVENT,
  INFO,
  COMMAND_SENT,
  COMMAND_RESPONSE,
  NEW_PACKET,
  END_OF_PARSEREVENTNUMBER_ENUM
} EventNumber;

typedef struct
{
  EventNumber ev;
  void* data;
} Event;

typedef struct 
{
  Event pevent;
  int parsed;
} ParsedData;

#endif // EVENTS_H