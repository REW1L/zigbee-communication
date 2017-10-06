//
// Created by Ivan on 06.10.17.
//

#ifndef CARS_SENDERWRAPPER_HPP
#define CARS_SENDERWRAPPER_HPP

#include <boost/python.hpp>
#include "Sender.hpp"

using namespace boost::python;
void send(Sender const&, std::string, int);

BOOST_PYTHON_MODULE( protocol )
{
  class_<Sender>("Sender")
          .def( "Sender", init<const char*>( args("path") ))
          .def( "Sender", init<Device*>( args("device") ))
          .def( "send", send, args("data", "from") )
          .add_property("device", make_getter(&Sender::device));
}
void send(Sender const& obj, std::string str, int from)
{
  obj.send(str.c_str(), str.length(), from);
}

#endif //CARS_SENDERWRAPPER_HPP
