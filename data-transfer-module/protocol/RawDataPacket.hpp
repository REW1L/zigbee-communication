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
  RawDataPacket(uint32_t src, size_t size, char* data)
          : src(src), size(size), data(data) {}
  uint32_t src;
  size_t size;
  char* data;
};


#endif //RAWDATAPACKET_HPP
