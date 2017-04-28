#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include "zigbee_definitions.h"
#include <stdint.h>
#include <stdio.h>

// TODO: make logs

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
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

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

int configure_device(const char* device)
{
  int fd;
  fd = open_port(device);
  
  if (fd == -1)
    return fd;

  set_interface_attribs (fd, B115200, 0);
  set_blocking (fd, 0);

  return fd;
}

int send_frame(int fd, char *array, size_t size)
{
  char text[76];
  if(size > 74)
    size = 74;
  // memcpy(text, "AT+BCASTB:4A,00\r", 16);
  // write(fd, text, 16);
  // memset(text, 0, FRAME_SIZE+2);
  // strncpy(text, "XB", 2);
  memcpy(text, array, size);
  return write(fd, text, size);
}

int send(int fd, char* array, size_t size)
{
  return write(fd, array, size);
}

size_t read_from_device(int fd, char *buffer, size_t size)
{
  return read(fd, buffer, size);
}

int get_network_info(int fd)
{
  return write(fd, "AT+N\r", 5);
}

int join_network(int fd)
{
  return write(fd, "AT+JN\r", 6);
}

int exit_network(int fd)
{
  return write(fd, "AT+DASSL\r", 9);
}

int create_network(int fd)
{
  return write(fd, "AT+EN\r", 6);
}

enum device_type get_device_type(char* text)
{
  if(strncmp(text, "FFD", 3) == 0)
    return FFD;
  if(strncmp(text, "COO", 3) == 0)
    return COO;
  if(strncmp(text, "ZED", 3) == 0)
    return ZED;
  if(strncmp(text, "SED", 3) == 0)
    return SED;
  if(strncmp(text, "MED", 3) == 0)
    return MED;
  return NON_TYPE;
}