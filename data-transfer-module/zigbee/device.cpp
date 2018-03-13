#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdint.h>
#include <sys/ioctl.h>
#include "device.hpp"

#include "../logging/ProtocolLogger.hpp"

static int
set_interface_attribs (int fd, int speed, int parity)
{
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0)
  {
    LOG_ERROR("DEVICE", "error %d from tcgetattr", errno);
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
    LOG_ERROR("DEVICE", "error %d from tcsetattr", errno);
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
    LOG_ERROR("DEVICE", "error %d from tggetattr", errno);
    return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 0;            // 0.5 seconds read timeout
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
  uint8_t buffer[255], *raw_mac = (uint8_t*)&(this->mac); //, offset;
  int response, fail_count;
  memset(buffer, 0, 255);
  this->fd = open_port(device);
  
  if (this->fd == -1)
    return -1;
  set_interface_attribs (this->fd, B19200, 0);
  set_blocking (this->fd, 0);

  fail_count = 0;
  this->_send_command("SH", 2);
  do
  {
    if (fail_count > 100)
    {
      fail_count = 0;
      this->_send_command("SH", 2);
    }
    response = this->read_frame((char*)buffer, 255);
    fail_count++;
  } while(response || buffer[3] != 0x88 || buffer[5] != 'S' || buffer[6] != 'H');
  for(int i = 0; i < 4; i++) raw_mac[7-i] = buffer[8+i];

  fail_count = 0;
  this->_send_command("SL", 2);
  do
  {
    if (fail_count > 100)
    {
      fail_count = 0;
      this->_send_command("SL", 2);
    }
    response = this->read_frame((char*)buffer, 255);
    fail_count++;
  } while(response || buffer[3] != 0x88 || buffer[5] != 'S' || buffer[6] != 'L');
  for(int i = 0; i < 4; i++) raw_mac[3-i] = buffer[8+i];

  LOG_INFO("DEVICE", "Configured with mac: 0x%llX", this->mac);
  return 0;
}

int Device::_send_command(const char* cmd, uint8_t cmd_size)
{
  uint8_t header[5] = { 0x7E, 0x00, (uint8_t)(cmd_size + 2), 0x08, 0x37 }; // command size can't be more than 255
  char* command_frame = (char*)malloc(cmd_size + 6); // delimiter + size + frame_type + frame_id + checksum
  memcpy(command_frame, header, 5);
  memcpy(&command_frame[5], cmd, cmd_size);
  command_frame[5+cmd_size] = calc_checksum(&command_frame[3], cmd_size + 5);
  return write(this->fd, command_frame, cmd_size + 6);
}

int Device::read_frame(char* buffer, const int max_size)
{
  int bytes;
  uint16_t size;

  usleep(1000000 / 1200); // 2400 Bytes/sec
  ioctl(this->fd, FIONREAD, &bytes);
  if( bytes < 1 )
    return 1;
  read(this->fd, buffer, 1);
  if(buffer[0] != 0x7E)
    return 2;

  usleep(2000000 / 1200); // 2400 Bytes/sec
  read(this->fd, &buffer[1], 2);
  size = (uint16_t)(((uint16_t)(buffer[1] & 0xFF) << 8) + (buffer[2] & 0xFF));
  if (size > max_size)
    return 3;

  usleep(size * 1000000 / 1200); // 2400 Bytes/sec
  bytes = (int)read(this->fd, &buffer[3], (size_t)size+1);
  return 0;
}

int Device::send_frame(char *array, int size)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_BEFORE_SEND)); // waiting for last transmission
  uint8_t header[8] = {0x7E, 0x00, 0x00, 0x01, 0x01, 0xFF, 0xFF, 0x0}; // broadcast
  size += 5; // size of data + XBEE header without first 3 bytes
  char *array_tx = (char*)calloc(1, (size_t)size + 4); // plus header size and crc byte
  header[1] = (uint8_t)((size >> 8) & 0xFF); // packet int size in little-endian
  header[2] = (uint8_t)(size & 0xFF);
  memcpy(array_tx, header, 8); // copy header in array to send
  memcpy(&array_tx[8], array, size - 5); // copy userdata in array to send
  array_tx[size + 3] = calc_checksum(&array_tx[3], size+1);
  LOG_INFO("DEVICE", "Sending frame (%d) with checksum: %02hhX", size, (char)array_tx[size + 3]);
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
