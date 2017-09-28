#include <stdio.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include "../protocol/Reader.hpp"
#include "../protocol/Sender.hpp"
#include "../protocol/WorkerThread.hpp"
#include "../logging/ProtocolLogger.hpp"
#include "MyUserListener.cpp"

#include "../zigbee/device.hpp"

int main(int argc, char const *argv[])
{
  char buf[500], input[500];
  size_t number;
  time_t timer; // only first 4 bytes will be used

  RouteConfig inf, infp;

  if(argc < 2)
  {
    printf("Usage: sudo sampleapp <device>\n device: device to be used\n");
    return 1;
  }

  printf("Configuring...\n");

  inf.id = 1;

  if(inf.id > 8 || inf.id < 1)
  {
    printf("Id must be from 1 to 8\n");
    return 1;
  }

// configure protocol

  // this is only right sequence
  Reader rt(argv[1], 500LL); // configuring reading timeout in nanoseconds
  Sender sr(rt.device);
  LOG_INFO("SA", "Configured with device: %s", argv[1]);
  WorkerThread wt;
  ProtocolLogger pl;

// run threads
  rt.run();
  wt.run();

// adding custom listener
  MyUserListener mul;
  wt.add_listener(&mul);

// initialize 
  inf.speed = 365;
  inf.coords_src[0] = 123;
  inf.coords_src[1] = 123;
  inf.coords_dst[0] = 256;
  inf.coords_dst[1] = 256;
  inf.time = time(&timer);

  infp = inf;
  printf("Data transfer module was configured.\n");
  
  while (1)
  {
    memset(buf, 0, 500);
    memset(input, 0, 500);
    fgets(input, 500, stdin);
    fflush(stdin);

    if(strcmp(input, "exit\n") == 0)
    {
      rt.stop();
      wt.stop();
      return 0;
    }

    if(strcmp(input, "set_speed\n") == 0)
    {
      std::cout << "Input speed: " << std::endl;
      std::cin >> inf.speed;
    }

    if(strcmp(input, "set_coords_start\n") == 0)
    {
      std::cout << "Input 1 coord: " << std::endl;
      std::cin >> inf.coords_src[0];
      std::cout << "Input 2 coord: " << std::endl;
      std::cin >> inf.coords_src[1];
    }

    if(strcmp(input, "set_coords_end\n") == 0)
    {
      std::cout << "Input 1 coord: " << std::endl;
      std::cin >> inf.coords_dst[0];
      std::cout << "Input 2 coord: " << std::endl;
      std::cin >> inf.coords_dst[1];
    }

    if(strcmp(input, "set_id\n") == 0)
    {
      std::cout << "Input id: " << std::endl;
      std::cin >> inf.id;
    }

    // send data
    if(strcmp(input, "do_this\n") == 0)
    {
      sr.send(inf);
    }    

    // send predefined data
    if(strncmp(input, "drdrdr", 6) == 0)
    {
      infp.time = time(&timer);
      sr.send(infp);
    }

    if(strncmp(input, "send_text", 9) == 0)
    {
      std::cout << "Input some string" << std::endl;
      scanf("%500[^\n]", input);
      for(number = 0; input[number] != '\n' && input[number] != '\r' && input[number] != '\0'; number++);
      sr.send(input, number, inf.id);
    }

    if(strcmp(input, "print_info\n") == 0)
    {
      std::cout << "ID: " << inf.id << "\nSpeed: " << inf.speed << 
      "\nCoords src: [ " << inf.coords_src[0] << " " <<
      inf.coords_src[1] << " ]" << "\nCoords dst: [ " << 
      inf.coords_dst[0] << " " << inf.coords_dst[1] << " ]\nTime: " <<
      inf.time << "\n";
    }
    if(strcmp(input, "help\n") == 0)
    {
      std::cout << "set_speed\nset_coords_start\nset_coords_end\nset_id" <<
        "\nexit\nprint_info\ndrdrdr - send predefined data\ndo_this - send current data\n";
    }
    if(strncmp(input, "send_file", 9) == 0)
    {
      printf("OPENING\n");
      FILE* to_send = fopen("something", "rb");
      fseek(to_send, SEEK_SET, SEEK_END);
      printf("COUNTING\n");
      size_t offset = ftell(to_send);
      char* buffer = new char[offset];
      printf("READING\n");
      fread(buffer, 1, offset-1, to_send);
      printf("SENDING\n");
      sr.device->send(buffer, offset);
      // send(sr.getId(), buffer, offset-1);
      delete[] buffer;
      fclose(to_send);
    }


    std::cin.sync();
    std::this_thread::sleep_for(std::chrono::microseconds(500000));
  }

  return 0;
}
