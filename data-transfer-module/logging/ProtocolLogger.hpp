#ifndef PROTOCOL_LOGGER_HPP
#define PROTOCOL_LOGGER_HPP

#include <string>
#include <list>
#include <mutex>
#include <cstdio>
#include <thread>

#define MAX_LOG_FILE_NAME_SIZE 50
#define MAX_LOG_SIZE 4096
#define LOGGING_SLEEP 10

#define LOG_INFO(prefix, format, ...) ProtocolLogger::log("[I] [" prefix "]", format, __VA_ARGS__)
#define LOG_DEBUG(prefix, format, ...) ProtocolLogger::log("[D] [" prefix "]", format, __VA_ARGS__)
#define LOG_ERROR(prefix, format, ...) ProtocolLogger::log("[E] [" prefix "]", format, __VA_ARGS__)
#define LOG_WARNING(prefix, format, ...) ProtocolLogger::log("[W] [" prefix "]", format, __VA_ARGS__)

class ProtocolLogger
{
public:
  ProtocolLogger();
  virtual ~ProtocolLogger() {thread_run = false;};
  static void log(const char *prefix, const char *format, ...);
  void stop() {thread_run = false; log_thread.join();};
private:
  void flush();
  void out_thread();
  void push_output(std::string str);
  bool thread_run;
  std::thread log_thread;
};

#endif // PROTOCOL_LOGGER_HPP