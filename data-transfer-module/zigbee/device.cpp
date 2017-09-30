#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "device.hpp"
#include "stdlib.h"

#include "../logging/ProtocolLogger.hpp"
#include <chrono>
#include <thread>

static int
set_interface_attribs (int fd, int speed, int parity)
{
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0)
  {
    // printf ("error %d from tcgetattr\n", errno);
    return errno;
  }

  cfsetospeed (&tty, speed);
  cfsetispeed (&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
  // disable IGNBRK for mismatched speed tests; otherwise receive break
  // as \000 chars
  tty.c_iflag &= ~IGNBRK;         // disable break processing
  tty.c_lflag = 0;                // no signaling chars, no echo,
                                  // no canonical processing
  tty.c_oflag = 0;                // no remapping, no delays
  tty.c_cc[VMIN]  = 0;            // read doesn't block
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

  tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                  // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
  {
    // printf ("error %d from tcsetattr\n", errno);

    return errno;
  }
  return 0;
}

static void
set_blocking (int fd, int should_block)
{
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0)
  {
    // printf ("error %d from tggetattr\n", errno);
    return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 0;            // 0.5 seconds read timeout

  // if (tcsetattr (fd, TCSANOW, &tty) != 0)
    // printf ("error %d setting term attributes", errno);
}


/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */

static int
open_port(const char* device)
{
  int fd; /* File descriptor for the port */

  fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd != -1)
    fcntl(fd, F_SETFL, 0);

  return fd;
}

int Device::configure_device(const char* device)
{
  uint8_t buffer[100], offset;
  this->fd = open_port(device);
  
  if (this->fd == -1)
    return -1;

  set_interface_attribs (this->fd, B115200, 0);
  set_blocking (this->fd, 0);

  write(this->fd, "+++", 3); // go to AT mode
  usleep(2000000); // Guard Times
  read(this->fd, buffer, 10);
  if(strncmp((char*)buffer, "OK", 2) != 0) // successfully went to AT mode
  {
    LOG_INFO("DEVICE", "Configuring buffer: %s", buffer);
    return -1;
  }
  write(this->fd, "ATSH\r", 5); // getting top 8 bytes of MAC
  usleep(100000); // it needs to get all response
  read(this->fd, buffer, 9);
  for(offset = 0; buffer[offset] != '\r' && buffer[offset] != '\n'; offset++);
  write(this->fd, "ATSL\r", 5); // getting bottom 8 bytes of MAC
  usleep(100000); // it needs to get all response
  read(this->fd, buffer+offset, 9);
  write(this->fd, "ATCN\r", 5);
  sscanf((char*)(buffer), "%llX", &(this->mac)); // all info is got is in human readable style
  LOG_INFO("DEVICE", "Configured with mac: 0x%llX, buffer: %s", this->mac, buffer);
  return 0;
}

int Device::send_frame(char *array, int size)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_BEFORE_SEND)); // waiting for last transmission
  uint8_t header[8] = {0x7E, 0x00, 0x00, 0x01, 0x01, 0xFF, 0xFF, 0x0}; // broadcast
  size += 5;
  char *array_tx = (char*)calloc(1, (size_t)size + 4); // plus header size and crc byte
  header[1] = (uint8_t)((size >> 8) & 0xFF); // packet int size in little-endian
  header[2] = (uint8_t)(size & 0xFF);
  memcpy(array_tx, header, 8); // copy header in array to send
  memcpy(&array_tx[8], array, size - 5); // copy userdata in array to send
  array_tx[size + 3] = calc_checksum(&array_tx[3], size+1);
  LOG_INFO("DEVICE", "Sending frame (%d) with checksum: %02hhX", size, (char)array_tx[size + 3]);
//  printf("\nSTART PACKET\n");
//  for(int i = 0; i < size + 4; i++)
//  {
//    printf("%02hhX ", array_tx[i]);
//  }
//  printf("\nEND PACKET\n");
  int bytes_sent = write(this->fd, array_tx, size + 4); // write bytes to XBEE
  delete[] array_tx; // memory deallocating
  return bytes_sent;
}

int Device::send(char* array, int size)
{
  return write(this->fd, array, size);
}

int Device::read_from_device(char *buffer, int size)
{
  return read(this->fd, buffer, size);
}

char Device::calc_checksum(const char* data, int size)
{
  uint16_t crc = 0;
  for(int i = 0; i < size-1; i++)
    crc += (char)data[i];
  return (char)(0xFF-crc&0xFF);
}

int Device::get_available_bytes()
{
  int number;
  ioctl(this->fd, FIONREAD, &number);
  return number;
}
