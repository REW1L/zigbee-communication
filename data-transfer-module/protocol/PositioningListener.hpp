//
// Created by Ivan on 07.03.18.
//

#ifndef POSITIONINGLISTENER_HPP
#define POSITIONINGLISTENER_HPP

#include <cstdint>
#include <cstring>
#include <map>
#include "Listener.hpp"

typedef struct {
  uint8_t RSSI;
  uint8_t RSSIe;
} PositioningUnit;

class PositioningListener : public Listener {
public:
  PositioningListener () { clear_positioning(); }
  virtual ~PositioningListener () {}
  virtual void notify(Event);
private:
  std::map<int, double> calc_position(uint8_t id, uint8_t RSSI, uint8_t RSSIe);
  void clear_positioning() { pos_map.clear(); };
  std::map<int, PositioningUnit> pos_map;
};


#endif //POSITIONINGLISTENER_HPP
