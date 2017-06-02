#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include "protocol_encode.c"
#include "parser.c"

int check_coords_encoding(uint32_t* coords, uint8_t field_num)
{
  char buffer[20];
  raw_field rf = pack_coords(buffer, coords, true, field_num);
  if(buffer[0] != (char)field_num)
  {
    printf("Error with field number e: %02hhX a: %02hhX\n", 
      field_num, buffer[0]);
    return 1;
  }
  if(buffer[1] != (char)((COORD_SIZE*2)&0xff) 
     || buffer[2] != (char)(((COORD_SIZE*2)/0x100)&0xff))
  {
    printf("Error with size e: %X a: %02hhX %02hhX\n", 
      COORD_SIZE, buffer[1], buffer[2]);
    return 1;
  }
  if(buffer[3] != (char)((coords[0])&0xff) 
     || buffer[4] != (char)((coords[0]/0x100)&0xff)
     || buffer[5] != (char)((coords[0]/0x10000)&0xff) 
     || buffer[6] != (char)((coords[0]/0x1000000)&0xff))
  {
    printf("Error with first coord e: %X a: %02hhX %02hhX %02hhX %02hhX\n", 
      coords[0], buffer[3], buffer[4], buffer[5], buffer[6]);
    return 1;
  }
  if(buffer[7] != (char)((coords[1])&0xff) 
     || buffer[8] != (char)((coords[1]/0x100)&0xff)
     || buffer[9] != (char)((coords[1]/0x10000)&0xff) 
     || buffer[10] != (char)((coords[1]/0x1000000)&0xff))
  {
    printf("Error with second coord e: %X a: %02hhX %02hhX %02hhX %02hhX\n", 
      coords[1], buffer[7], buffer[8], buffer[9], buffer[10]);
    return 1;
  }
  if(rf.size != (SIZE_SIZE + FIELD_ID_SIZE + COORD_SIZE + COORD_SIZE))
  {
    printf("Error with size e: %d a: %d", 
      SIZE_SIZE + FIELD_ID_SIZE + COORD_SIZE + COORD_SIZE ,rf.size);
  }
  return 0;
}

int check_coords()
{
  printf("Checking coords encoding\n");
  int from = 1, to = 0x10000000;
  uint32_t coords[2] = {0,0};
  if(check_coords_encoding(coords, COORDS_START))
  {
      return 1;
  }
  for (int i = from; i < to; i *= 11)
  {
    coords[0] = i;
    coords[1] = i;
    if(check_coords_encoding(coords, i%2))
    {
      printf("FAILED\n");
      return 1;
    }
  }
  printf("PASSED\n");
  return 0;
}

int check_header_encoding(uint16_t flags, uint32_t id, char mes_num, char op)
{
  char buff[100];
  memset(buff, 0, 100);
  make_header(buff, flags, id, mes_num, op, 0, 0);
  if(buff[0] != (char)(flags&0xff) || buff[1] != (char)((flags/0x100)&0xff))
  {
    printf("Error in flags encoding e: %02hX a: %02hhX %02hhX\n", 
      flags, buff[0], buff[1]);
    return 1;
  } 
  else if(buff[2] != mes_num)
  {
    printf("Error in message number encoding e: %02hhX a: %02hhX\n", 
      mes_num, buff[2]);
    return 1;
  }
  else if(buff[3] != (char)(id&0xff) || buff[4] != (char)((id / 0x100)&0xff)
    || buff[5] != (char)((id / 0x10000)&0xff) 
    || buff[6] != (char)((id / 0x1000000)&0xff))
  {
    printf("Error in id encoding e: %X a: %02hhX %02hhX %02hhX %02hhX\n", 
      ((id / 0x100)), buff[3], buff[4], buff[5], buff[6]);
    return 1;
  }
  else if(buff[7] != op)
  {
    printf("Error in OP encoding e: %02hhX a: %02hhX\n", op, buff[7]);
    return 1;
  }
  return 0;
}

int check_header()
{
  uint32_t from = 1, to = 0x10000000;
  printf("Checking header encoding\n");
  for (uint32_t i = from; i < to; i *= 11)
  {
    if(check_header_encoding(i%0xffff, i%0xffffffff, i%0xff, i%0xff))
    {
      printf("FAILED\n");
      return 1;
    }
  }
  printf("PASSED\n");
  return 0;
}

int check_time_encoding(uint64_t time)
{
  char buffer[20];

  raw_field rf = pack_time(buffer, &time, true, TIME);
  if(buffer[0] != (char)TIME)
  {
    printf("Error with field number e: %02hhX a: %02hhX\n", 
      (char)TIME, buffer[0]);
    return 1;
  }

  if(buffer[1] != (char)((TIME_SIZE)&0xff) 
     || buffer[2] != (char)(((TIME_SIZE)/0x100)&0xff))
  {
    printf("Error with size e: %X a: %02hhX %02hhX\n", 
      COORD_SIZE, buffer[1], buffer[2]);
    return 1;
  }

  if(buffer[3] != (char)(time&0xff) 
     || buffer[4] != (char)((time/0x100)&0xff)
     || buffer[5] != (char)((time/0x10000)&0xff) 
     || buffer[6] != (char)((time/0x1000000)&0xff)
     || buffer[7] != (char)((time/0x100000000)&0xff) 
     || buffer[8] != (char)((time/0x10000000000)&0xff)
     || buffer[9] != (char)((time/0x1000000000000)&0xff) 
     || buffer[10] != (char)((time/0x100000000000000)&0xff))
  {
    printf("Error with time e: %llX a: %02hhX %02hhX %02hhX "
      "%02hhX %02hhX %02hhX %02hhX %02hhX\n", 
      time, buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], 
      buffer[8], buffer[9], buffer[10]);
    return 1;
  }

  if(rf.size != (SIZE_SIZE + FIELD_ID_SIZE + TIME_SIZE))
  {
    printf("Error with ret size e: %d a: %d", 
      SIZE_SIZE + FIELD_ID_SIZE + TIME_SIZE ,rf.size);
  }
  return 0;
}

int check_time()
{
  printf("Checking time encoding\n");
  int64_t from = 1, to = 0x1000000000000000;
  bool passed = true;
  if(check_time_encoding(0))
  {
    passed = false;
    printf("FAILED\n");
    return 1;
  }
  for (uint64_t i = from; i < to; i *= 0x11)
  {
    if(check_time_encoding(i))
    {
      passed = false;
      printf("FAILED\n");
      return 1;
    }
  }
  printf("PASSED\n");
  return 0;
}

int check_speed_encoding(uint16_t speed)
{
  char buffer[20];

  raw_field rf = pack_speed(buffer, &speed, true, SPEED);
  if(buffer[0] != (char)SPEED)
  {
    printf("Error with field number e: %02hhX a: %02hhX\n", 
      (char)SPEED, buffer[0]);
    return 1;
  }

  if(buffer[1] != (char)((SPEED_SIZE)&0xff) 
     || buffer[2] != (char)(((SPEED_SIZE)/0x100)&0xff))
  {
    printf("Error with size e: %X a: %02hhX %02hhX\n", 
      COORD_SIZE, buffer[1], buffer[2]);
    return 1;
  }

  if(buffer[3] != (char)(speed&0xff) 
     || buffer[4] != (char)((speed/0x100)&0xff))
  {
    printf("Error with speed e: %X a: %02hhX %02hhX\n", 
      speed, buffer[3], buffer[4]);
    return 1;
  }

  if(rf.size != (SIZE_SIZE + FIELD_ID_SIZE + SPEED_SIZE))
  {
    printf("Error with ret size e: %d a: %d", 
      SIZE_SIZE + FIELD_ID_SIZE + SPEED_SIZE ,rf.size);
  }
  return 0;
}

int check_speed()
{
  printf("Checking speed encoding\n");
  for(uint16_t i = 0; i < 0xffff; i++)
  {
    if(check_speed_encoding(i))
    {
      printf("FAILED\n");
      return 1;
    }
  }
  printf("PASSED\n");
  return 0;
}

int check_parse_info()
{
  printf("Checking parsing to info\n");
  info inf, inf_check;

  size_t i, j;
  inf.id = 1;
  inf.speed = 365;
  inf.coords_src[0] = 123;
  inf.coords_src[1] = 123;
  inf.coords_dst[0] = 256;
  inf.coords_dst[1] = 256;
  inf.time = 1484316021;
  inf_check = parse_info(a, 130, 1);
  if(inf.speed != inf_check.speed)
  {
    printf("Parse info failed:\nSpeed check failed:\nExpected: %d Actual: %d\n", inf.speed, inf_check.speed);
    return 1;
  }

  if(inf.time != inf_check.time)
  {
    printf("Parse info failed:\nTime check failed:\nExpected: %d Actual: %d\n", inf.time, inf_check.time);
    return 1;
  }

  if(inf.coords_src[0] != inf_check.coords_src[0] || inf.coords_src[1] != inf_check.coords_src[1])
  {
    printf("Parse info failed:\nSource coords check failed:\nExpected: [%d, %d] Actual: [%d, %d]\n", inf.coords_src[0], inf.coords_src[1], inf_check.coords_src[0], inf_check.coords_src[1]);
    return 1;
  }

  if(inf.coords_dst[0] != inf_check.coords_dst[0] || inf.coords_dst[1] != inf_check.coords_dst[1])
  {
    printf("Parse info failed:\nDist coords check failed:\nExpected: [%d, %d] Actual: [%d, %d]\n", inf.coords_dst[0], inf.coords_dst[1], inf_check.coords_dst[0], inf_check.coords_dst[1]);
    return 1;
  }

  printf("PASSED\n");
  return 0;
}


int main(int argc, const char* argv[])
{
  if(check_header() != 0)
    return 1;

  if(check_coords() != 0)
    return 1;

  if(check_time() != 0)
    return 1;

  if(check_speed() != 0)
    return 1;

  // check_pack_data();

  if(check_parse_info() != 0)
    return 1;
  
}