#include "device_actions.hpp"
#include <stdio.h>
#include "RF24Mesh/RF24Mesh.h" 
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <list>
#include "ProtocolLogger.hpp"


RF24 radio(RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);
RF24Mesh mesh(radio,network);

static std::list<uint16_t> addresses;


int configure_device(int nodeID)
{
  LOG_INFO("RF24", "Confiruring node id: %d", nodeID);

  bool master = join_network(nodeID);

  if(!master)
    master = create_network(nodeID);
  else
    master = false;
  LOG_INFO("RF24", "Configured %s with id: %d", master? "master": "slave", nodeID);

  return master;
}

int send_frame(int master, char *array, int size)
{

  int attempts, ret;

  // uint16_t to = 0x0000; // Send to master
  RF24NetworkHeader header;

  for(uint16_t to : addresses)
  {
    LOG_INFO("RF24", "Sending packet to %d", to);
    header = RF24NetworkHeader(to, 'F'); // Send header type 'F'

    ret = 0;
    while(!ret)
    {
      // LOG_INFO("RF24", )
      mesh.update();
      if(master)
      {
        mesh.DHCP();
      }
      ret = mesh.write(array, 'F', size, to);
      LOG_INFO("RF24", "Writing to network %s", ret? "successfull" : "failed");
      if(!ret && !master)
      {
        // If a write fails, check connectivity to the mesh network
        if( ! mesh.checkConnection() )
        {
          // The address could be refreshed per a specified timeframe or only when sequential writes fail, etc.
          LOG_WARNING("RF24", "%s", "Connection failed");
          if(!master)
          {
            LOG_WARNING("RF24", "%s", "Renewing Address");
            mesh.renewAddress(1000);
          }
        }
        else
        {
          LOG_WARNING("RF24", "%s", "Connection is stable but writing fails");
        }
      }
    }
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
    LOG_INFO("RF24", "Available new data from network, header: %c", header.type);
    if(header.type == 'A')
    {
      uint16_t new_addr = 0;
      bool add = true;
      network.read(header,&new_addr,sizeof(uint16_t));
      LOG_INFO("RF24", "Header type 'A', new address: %d", new_addr);
      // if(master)
      // {
      //   for(uint16_t addr : addresses)
      //   {
      //     header = RF24NetworkHeader(mesh.getAddress(addr), 'A');
      //     network.write(header, &new_addr, sizeof(new_addr));
      //     // mesh.write(&new_addr, 'A', sizeof(new_addr), addr);

      //     header = RF24NetworkHeader(mesh.getAddress(new_addr), 'A');
      //     network.write(header, &addr, sizeof(addr));
      //     // mesh.write(&addr, 'A', sizeof(addr), new_addr);
      //   }
      // }

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

      LOG_INFO("RF24", "%s", "Known addresses:");
      for(uint16_t addr : addresses)
      {
        LOG_INFO("RF24", "%d", addr);
      }
      return 0;
    }
    else if(header.type == 'F')
    {
      if(master)
      {
        int received_bytes = network.read(header, buffer, size);
        send_frame(master, buffer, received_bytes);
        return received_bytes;
      }
      else
      {
        return network.read(header, buffer, size);
      }
    }
    else
    {
      LOG_WARNING("RF24", "Not expected header type: %c", header.type);
      return 0;
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
  LOG_INFO("RF24", "Join network with id: %d", id);
  mesh.setNodeID(id);
  addresses.clear();
  bool connected = mesh.begin( MESH_DEFAULT_CHANNEL, RF24_1MBPS, 4000 ); // TODO: make configurable
  if(connected)
  {
    LOG_INFO("RF24", "Сonnected to the network %d", connected);
    mesh.update();
    uint16_t addr = id;
    mesh.write(&addr, 'A', sizeof(addr));
    addresses.push_back(0);
  }
  else
  {
    LOG_INFO("RF24", "Not connected to the network %d", connected);
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
