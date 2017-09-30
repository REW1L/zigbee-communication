#include "UserListener.hpp"
#include "ProtocolLogger.hpp"

void UserListener::notify(Event ev)
{
  switch(ev.ev)
  {
    case RECEIVED_RAW_DATA_PACKET:
    {
      RawDataPacket *rdp;
      rdp = new RawDataPacket(((RawDataPacket*)(ev.data))->size,
                              new char[((RawDataPacket*)(ev.data))->size]);
      memcpy(rdp->data, ((RawDataPacket*)(ev.data))->data, rdp->size);
      LOG_INFO("USR_LIST", "Notify RawData size: %u", rdp->size);

      this->notifyRawData(rdp);
      break;
    }
  }
}
