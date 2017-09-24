#include "WorkerThread.hpp"
#include "ProtocolLogger.hpp"
#include "Events.h"
#include <thread>
#include <chrono>

std::mutex WorkerThread::events_mtx;
std::list<Event> WorkerThread::events;

int WorkerThread::run()
{
  this->stopped = 0;
  working_thread = std::thread(&WorkerThread::work, this);
  return 0;
}

WorkerThread::~WorkerThread()
{
  this->stopped = 1;
  this->working_thread.join();
  delete this->pl;
}

WorkerThread::WorkerThread()
{
  this->pl = new ProtocolListener();
  this->add_listener(pl);
}

int WorkerThread::add_event(Event ev)
{
  LOG_INFO("WORKER", "New event [%d]", ev.ev);
  WorkerThread::events_mtx.lock();
  WorkerThread::events.push_back(ev);
  WorkerThread::events_mtx.unlock();
  return 0;
}

void WorkerThread::work()
{
  Event ev;
  while(!this->stopped)
  {
    if(!WorkerThread::events.empty())
    {
      WorkerThread::events_mtx.lock();
      ev = WorkerThread::events.front();
      WorkerThread::events.pop_front();
      WorkerThread::events_mtx.unlock();

      LOG_INFO("WORKER", "Notify event [%d]", ev.ev);

      for(Listener* lst : listeners)
      {
        lst->notify(ev);
      }
      if(ev.data != NULL)
        delete ev.data;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(10000));
  }
}

void WorkerThread::add_listener(Listener* lst)
{
  LOG_INFO("WORKER", "Added listener [%p]", lst);
  this->listeners.push_back(lst);
}

void WorkerThread::stop()
{
  LOG_INFO("WORKER", "Stopped (%d)", 1);
  this->stopped = 1;
}
