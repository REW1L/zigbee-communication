//
// Created by Ivan on 23.09.17.
//

#ifndef RAWDATAPACKET_HPP
#define RAWDATAPACKET_HPP
#include <cstdint>
#include <cstdlib>


class RawDataPacket {
public:
  RawDataPacket(){}
  ~RawDataPacket() { free(data); }
  RawDataPacket(size_t size, char* data)
          : size(size), data(data) {}
  size_t size;
  char* data;
};


#endif //RAWDATAPACKET_HPP
