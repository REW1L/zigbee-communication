#include <stdio.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include "../protocol/Reader.hpp"
#include "../protocol/Sender.hpp"
#include "../protocol/WorkerThread.hpp"
#include "MyUserListener.cpp"

int main(int argc, char const *argv[])
{
  char buf[100], input[100];
  time_t timer; // only first 4 bytes will be used

  RouteConfig inf, infp;

  if(argc < 2)
  {
    printf("Usage: sudo sampleapp <id>\n id: id of car (must be from 1 to 8)\n");
    return 1;
  }

  printf("Configuring...\n");

  inf.id = atoi(argv[1]); // id must be unique number (1-8)

  if(inf.id > 8 || inf.id < 1)
  {
    printf("Id must be from 1 to 8\n");
    return 1;
  }

// configure protocol

  Reader rt(inf.id, 500LL); // configuring node with id and reading timeout in nanoseconds
  Sender sr;
  WorkerThread wt;

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

  printf("%s was configured.\n", (rt.getId() == 1)? "Master" : "Node" );
  
  while (1)
  {
    memset(buf, 0, 100);
    memset(input, 0, 100);
    fgets(input, 100, stdin);
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

    if(strcmp(input, "create_network\n") == 0)
    {
      sr.create_net();
    }

    if(strcmp(input, "join_network\n") == 0)
    {
      sr.join_net();
    }

    if(strcmp(input, "network_info\n") == 0)
    {
      sr.net_info();
    }
    if(strcmp(input, "exit_network\n") == 0)
    {
      sr.exit_net();
    }

    // send data
    if(strcmp(input, "do_this\n") == 0)
    {
      sr.send(inf);
    }    

    // send predefined data
    if(strcmp(input, "drdrdr\n") == 0)
    {
      infp.time = time(&timer);
      sr.send(infp);
    }

    if(strcmp(input, "print_info\n") == 0)
    {
      std::cout << "ID: " << inf.id << "\nSpeed: " << inf.speed << 
      "\nCoords src: [ " << inf.coords_src[0] << " " <<
      inf.coords_src[1] << " ]" << "\nCoords dst: [ " << 
      inf.coords_dst[0] << " " << inf.coords_dst[1] << " ]\nTime: " <<
      inf.time << "\n"; //"\nWay length: " << inf.way_length << "\n";
    }
    if(strcmp(input, "help\n") == 0)
    {
      std::cout << "set_speed\nset_coords_start\nset_coords_end\nset_id" <<
        "\nexit\nprint_info\ndrdrdr - send predefined data\ndo_this - send current data\n";
    }

    std::cin.sync();
    std::this_thread::sleep_for(std::chrono::microseconds(500000));
  }

  return 0;
}
