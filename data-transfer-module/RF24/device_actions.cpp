#include "device_actions.hpp"
#include <stdio.h>
#include "RF24Mesh/RF24Mesh.h" 
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <list>


RF24 radio(RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);
RF24Mesh mesh(radio,network);

static std::list<uint16_t> addresses;


int configure_device(int nodeID)
{
  // network.setup_watchdog(0);
  bool master = join_network(nodeID);

  if(!master)
    master = create_network(nodeID);
  else
    master = false;
  // network.multicastLevel(1);
  return master;
}

int send_frame(int master, char *array, int size)
{

  int attempts, ret;

  // uint16_t to = 0x0000; // Send to master
  RF24NetworkHeader header;

  for(uint16_t to : addresses)
  {
    header = RF24NetworkHeader(to, 'F'); // Send header type 'F'

    ret = 0;
    // attempts = 10;
    // printf("to: %d\n", to);

    // while(attempts--)
    // {
      mesh.update();
      ret = mesh.write(array, 'F', size, to);
      // if(!ret)
      // {
        // If a write fails, check connectivity to the mesh network
        // if( ! mesh.checkConnection() )
        // {
          // The address could be refreshed per a specified timeframe or only when sequential writes fail, etc.
          // printf("Renewing Address\n");
          // if(!master)
          //   mesh.renewAddress(1000);
        // }
      // }
      // else
      // {
        // printf("Send OK to node %d\n", to);
        // break;
      // }
    // }
  }
  return ret;
}

size_t read_from_device(int master, char *buffer, size_t size)
{
  RF24NetworkHeader header;
  mesh.update();
  if(master)
    mesh.DHCP();

  if(network.available())
  {
    network.peek(header);
    if(header.type == 'A')
    {
      uint16_t new_addr = 0;
      bool add = true;
      network.read(header,&new_addr,sizeof(uint16_t));
      printf("Header type 'A', new address: %d\n", new_addr);
      if(master)
      {
        for(uint16_t addr : addresses)
        {
          // header = RF24NetworkHeader(mesh.getAddress(addr), 'A');
          // network.write(header, &new_addr, sizeof(new_addr));
          mesh.write(&new_addr, 'A', sizeof(new_addr), addr);

          // header = RF24NetworkHeader(mesh.getAddress(new_addr), 'A');
          // network.write(header, &addr, sizeof(addr));
          mesh.write(&addr, 'A', sizeof(addr), new_addr);
        }
      }

      for(uint16_t addr : addresses)
      {
        if(addr == new_addr)
        {
          add = false;
          break;
        }
      }
      if(add)
        addresses.push_back(new_addr);

      printf("Addresses: ");
      for(uint16_t addr : addresses)
      {
        printf("%d ", addr);
      }
      printf("\n");
      return 0;
    }
    else
    {
      return network.read(header, buffer, size);
    }
  }
  else
  {
    return 0;
  }
}

int get_network_info(int master)
{
  return network.available();
}

int join_network(int id)
{
  mesh.setNodeID(id);
  addresses.clear();
  bool connected = mesh.begin( MESH_DEFAULT_CHANNEL, RF24_1MBPS, 4000 );
  if(connected)
  {
    mesh.update();
    uint16_t addr = id;
    mesh.write(&addr, 'A', sizeof(addr));
    addresses.push_back(0);
  }
  return connected;
}

int exit_network(int master)
{
  return 0;
}

int create_network(int master)
{
  mesh.setNodeID(0);

  bool connected = mesh.begin( MESH_DEFAULT_CHANNEL, RF24_1MBPS, 4000 );
  mesh.DHCP();
  return connected;
}
