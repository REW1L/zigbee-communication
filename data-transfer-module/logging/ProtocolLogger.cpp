#include "ProtocolLogger.hpp"
#include <string>
#include <cstdarg>
#include <ctime>
#include <cstdio>
#include <thread>
#include <chrono>

std::string log_file_name;
std::list<std::string> out_list;
std::mutex log_mtx;

void ProtocolLogger::log(const char *prefix, const char *format, ...)
{
  va_list arguments;
  char buffer[MAX_LOG_SIZE];
  auto timestamp = std::time(NULL);
  va_start(arguments, format);
  int offset = std::strftime(buffer, MAX_LOG_SIZE, "[%H:%M:%S] ", 
                             std::localtime(&timestamp));
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
    ProtocolLogger::flush();
    std::this_thread::sleep_for(std::chrono::seconds(2));
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

  FILE* descriptor = fopen(log_file_name.c_str(), "a");
  for(auto it = temp_str.begin(); it != temp_str.end(); it++)
  {
    fprintf(descriptor, "%s\n", (*it).c_str());
  }
  temp_str.clear();
  fclose(descriptor);
}

ProtocolLogger::ProtocolLogger()
{
  this->thread_run = true;
  char file_name[MAX_LOG_FILE_NAME_SIZE];
  auto timestamp = std::time(NULL);
  std::strftime(file_name, MAX_LOG_FILE_NAME_SIZE, 
                "ProtocolLog_%d_%m_%Y_%H_%M_%S.log", 
                std::localtime(&timestamp));
  log_file_name = std::string(file_name);
  this->log_thread = std::thread(&ProtocolLogger::out_thread, this);
}