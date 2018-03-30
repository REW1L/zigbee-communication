//
// Created by Ivan on 07.03.18.
//

#include "PositioningListener.hpp"

#include "ProtocolLogger.hpp"
#include "WorkerThread.hpp"
#include "RawDataPacket.hpp"

#include <math.h>

void PositioningListener::notify(Event ev)
{
  switch(ev.ev)
  {
    case RECEIVED_POSITIONING_DATA:
    {
      uint8_t* rssi_data = (uint8_t*)ev.data;
      if(!rssi_data[1] || rssi_data[1] > 3)
      {
        LOG_WARNING("POS_LIST", "Received positioning from strange id: %02hhX", rssi_data[1]);
        return;
      }
      LOG_INFO("POS_LIST", "Received positioning from id: %02hhX (RSSI: %d, RSSIe: %d, RSSIe2: %d)",
               rssi_data[1], rssi_data[0], rssi_data[2], rssi_data[3]);

      std::map<int, double> distance = calc_position(rssi_data[1], rssi_data[0], rssi_data[2], rssi_data[3]);

      if(!distance.empty())
      {
        size_t str_size = distance.size()*10+10*2+11;
        char* data = new char[str_size];
        snprintf(data, str_size, "{d:[%f,%f,%f],p:[0,0]}", distance[0], distance[1], distance[2]);
        RawDataPacket* rdp = new RawDataPacket(str_size, data);
        Event pos_ev = { .ev = RECEIVED_RAW_DATA_PACKET, .data = rdp};
        WorkerThread::add_event( pos_ev );
        return;
      }
      return;
    }
    default: return;
  }
}

static double calc_distance(double RSSI, double RSSIe, double path_loss_exp)
{
  return pow(10.0, (-RSSIe+RSSI)/(10.0 * path_loss_exp));
}

static double calc_path_loss_exp(double RSSIe, double RSSI)
{
  return (-RSSIe+RSSI)/(10*log10(2));
}

static std::list<std::pair<double, double>> calc_intersec_points( double x0, double y0, double r0,
                                                                  double x1, double y1, double r1)
{
  std::list<std::pair<double, double>> intersec_points;
  double d = sqrt( pow(x1-x0, 2) +  pow(y1-y0, 2) );
  if (d > r1+r0)
    return intersec_points;

  double p = (r0 + r1 + d)/2;
  double h = 2 * sqrt(p * (p - r0) * (p - r1) * (p - d)) / d;
  double a = sqrt( pow(r0, 2) - pow(h, 2) );
  double xr1 = x0 + (a * (x1-x0) + h * (y1-y0)) / d;
  double yr1 = y0 + (a * (y1-y0) + h * (x1-x0)) / d;
  double xr2 = x0 + (a * (x1-x0) - h * (y1-y0)) / d;
  double yr2 = y0 + (a * (y1-y0) - h * (x1-x0)) / d;
  if( pow(xr1,2) + pow(yr1,2) - pow(r0, 2) < 0.01 )
  {
    intersec_points.push_back(std::pair<double, double>(xr1, yr1));
    intersec_points.push_back(std::pair<double, double>(xr2, yr2));
  }
  else
  {
    intersec_points.push_back(std::pair<double, double>(xr1, yr2));
    intersec_points.push_back(std::pair<double, double>(xr2, yr1));
  }
  return intersec_points;
}


std::map<int, double> PositioningListener::calc_position(uint8_t id, uint8_t RSSI, uint8_t RSSIe, uint8_t RSSIe2)
{
  std::map<int, double> rezult;
  if (id == 1) // new data
    clear_positioning();

  this->pos_map[id-1] = PositioningUnit { .RSSI = RSSI, .RSSIe = RSSIe, .RSSIe2 = RSSIe2 };

  if(this->pos_map.size() < 3)
    return rezult;

  if(id == 3)
    id = 1;
  else
    id--;

  double path_loss_exp = calc_path_loss_exp(pos_map[1].RSSIe, pos_map[1].RSSIe2);

  LOG_INFO("POS_LIST", "Ethalon RSSI: [%d, %d] Path Loss Exponent: %f", pos_map[id].RSSIe,
           pos_map[id].RSSIe2, path_loss_exp);

  for(int i = 0; i < 3; i ++)
    rezult[i] = calc_distance(pos_map[i].RSSI, pos_map[id].RSSIe, path_loss_exp);

  LOG_INFO("POS_LIST", "Calculated distance: 1: %f 2: %f 3: %f", rezult.at(0), rezult.at(1), rezult.at(2));

  std::list<std::pair<double, double>> temp_list;
  std::map<int, std::list<std::pair<double, double>>> intersec_points;
  double x = 0, y = 0, k = 0, max_dist = 0, temp_dist;

  temp_list = calc_intersec_points(0, 0, rezult[1], 1, 0, rezult[0]);
  for(auto i: temp_list)
  {
    if(i.first >= 0 && i.second >= 0)
    {
      intersec_points[0].push_back(i);
      x += i.first;
      y += i.second;
      k++;
    }
  }

  temp_list = calc_intersec_points(0, 0, rezult[1], 0, 2, rezult[2]);
  for(auto i: temp_list)
  {
    if(i.first >= 0 && i.second >= 0)
    {
      intersec_points[1].push_back(i);
      x += i.first;
      y += i.second;
      k++;
    }
  }

  temp_list = calc_intersec_points(1, 0, rezult[0], 0, 2, rezult[2]);
  for(auto i: temp_list)
  {
    if(i.first >= 0 && i.second >= 0)
    {
      intersec_points[2].push_back(i);
      x += i.first;
      y += i.second;
      k++;
    }
  }
  x = x/k;
  y = y/k;

  int temp_c = 0;
  char* temp = new char[intersec_points.size()*40];
  for(int i = 0; i < intersec_points.size(); i++)
  {
    for (std::pair<double, double> j: intersec_points[i])
    {
      temp_c += sprintf(&temp[temp_c], "{%f, %f} ", j.first, j.second);
      temp_dist = sqrt(pow(x-j.first, 2) + pow(y-j.second, 2));
      if(max_dist < temp_dist)
        max_dist = temp_dist;
    }
  }

  LOG_INFO("POS_LIST", "Intersec pointers: %s", temp);
  delete[] temp;

  LOG_INFO("POS_LIST", "Guessing position: [%f;%f] radius: %f", x, y, max_dist);

  return rezult;
}
