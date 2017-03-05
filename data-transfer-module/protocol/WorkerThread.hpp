#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <list>
#include <mutex>
#include <thread>
#include "Events.h"
#include "Listener.hpp"
#include "ProtocolListener.hpp"

class WorkerThread
{
  public:
    WorkerThread();
    virtual ~WorkerThread();
    int run();
    void add_listener(Listener*);
    static int add_event(Event ev);
    void stop();
  private:
    void work();
    static std::mutex events_mtx;
    const char *path;
    int file_descriptor;
    long timeout;
    std::thread working_thread;
    char stopped;
    ProtocolListener *pl;
    static std::list<Event> events;
    std::list<Listener*> listeners;
};


#endif // WORKERTHREAD_H