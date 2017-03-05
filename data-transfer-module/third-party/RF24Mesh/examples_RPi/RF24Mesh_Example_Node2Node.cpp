 
 
 /** RF24Mesh_Example.cpp by TMRh20
  * 
  * Note: This sketch only functions on -RPi-
  *
  * This example sketch shows how to manually configure a node via RF24Mesh, and send data to the
  * master node.
  * In this sketch, the nodes will refresh their network address as soon as a single write fails. This allows the
  * nodes to change position in relation to each other and the master node.
  *
  */
  
#include "RF24Mesh/RF24Mesh.h" 
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>


RF24 radio(RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);
RF24Mesh mesh(radio,network);

uint32_t displayTimer=0;

int main(int argc, char** argv) {
  int i;
  // Set the nodeID to 0 for the master node
  mesh.setNodeID(2);
  // Connect to the mesh
  printf("start nodeID %d\n",mesh.getNodeID());

  // mesh.begin();
  printf("All is ok, %d\n\n", mesh.begin( MESH_DEFAULT_CHANNEL, RF24_1MBPS, 2000 ));
  radio.printDetails();
  char buff[500];
  while(1)
  {
    for(i = 0; i < 500; i++) 
      buff[i] = 0;
    // Call mesh.update to keep the network updated
    mesh.update();
    mesh.DHCP();

    if(network.available())
    {
      RF24NetworkHeader header;
      uint32_t mills;
      network.read(header, &buff, 500);
      printf("Rcv %s from nodeID %d\n", buff, mesh.getNodeID(header.from_node));
    }
      
    // Send the current millis() to the master node every second
    if(millis() - displayTimer >= 1000)
    {
      displayTimer = millis();
      
      if(!mesh.write(&displayTimer,'M',sizeof(displayTimer), 1))
      {
         
        // If a write fails, check connectivity to the mesh network
        if( ! mesh.checkConnection() )
        {
          // The address could be refreshed per a specified timeframe or only when sequential writes fail, etc.
          printf("Renewing Address\n");
          // mesh.renewAddress(); 
        }
        else
        {
          printf("Send fail, Test OK\n"); 
        }
      }
      else
      {
        printf("Send OK: %u\n",displayTimer);
      }
    }
    delay(1);
  }
  return 0;
}

      
      
      
