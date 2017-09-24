#include "ProtocolLogger.hpp"
#include <string>
#include <cstdarg>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <thread>
#include <chrono>

#ifndef LOGGING_NONE

std::string log_file_name;
std::list<std::string> out_list;
std::mutex log_mtx;

void ProtocolLogger::log(const char *prefix, const char *format, ...)
{
  va_list arguments;
  char buffer[MAX_LOG_SIZE];
  memset(buffer, 0, MAX_LOG_SIZE);
  auto timestamp = std::chrono::system_clock::now();
  auto time_c = std::chrono::system_clock::to_time_t(timestamp);
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()) 
    - std::chrono::duration_cast<std::chrono::seconds>(timestamp.time_since_epoch());
  int offset = std::strftime(buffer, MAX_LOG_SIZE, "[%H:%M:%S", 
                             std::localtime(&time_c));
  offset += snprintf(buffer+offset, MAX_LOG_SIZE, ".%lld] ", millis.count());

  va_start(arguments, format);
  offset += std::snprintf(buffer+offset, MAX_LOG_SIZE-offset, "%s ", prefix);
  vsnprintf(buffer+offset, MAX_LOG_SIZE-offset, format, arguments);
  std::string str(buffer);
  va_end(arguments);
  log_mtx.lock();
  out_list.push_back(str);
  log_mtx.unlock();
}

void ProtocolLogger::out_thread()
{
  while(thread_run)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(LOGGING_SLEEP));
    ProtocolLogger::flush();
  }
}

void ProtocolLogger::flush()
{
  std::list<std::string> temp_str;
  
  log_mtx.lock();
  for(auto it = out_list.begin(); it != out_list.end(); it++)
  {
    temp_str.push_back(*it);
  }
  out_list.clear();
  log_mtx.unlock();

#ifndef LOGGING_STDOUT
  FILE* descriptor = fopen(log_file_name.c_str(), "a");
  for(auto it = temp_str.begin(); it != temp_str.end(); it++)
  {
    fprintf(descriptor, "%s\n", (*it).c_str());
  }
  fclose(descriptor);
#else
  for(auto it = temp_str.begin(); it != temp_str.end(); it++)
  {
    printf("%s\n", (*it).c_str());
  }
#endif
  temp_str.clear();
}

ProtocolLogger::ProtocolLogger()
{
  this->thread_run = true;
  char file_name[MAX_LOG_FILE_NAME_SIZE];
  auto timestamp = std::time(NULL);
  std::strftime(file_name, MAX_LOG_FILE_NAME_SIZE, 
                "ProtocolLog_%Y_%m_%d_%H_%M_%S.log", 
                std::localtime(&timestamp));
  log_file_name = std::string(file_name);
  this->log_thread = std::thread(&ProtocolLogger::out_thread, this);
}

#else
void ProtocolLogger::log(const char *prefix, const char *format, ...) {}
void ProtocolLogger::out_thread() {}
void ProtocolLogger::flush() {}
ProtocolLogger::ProtocolLogger() {}
#endif