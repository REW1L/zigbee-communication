//
// Created by Ivan on 06.10.17.
//

#ifndef CARS_READERWRAPPER_HPP
#define CARS_READERWRAPPER_HPP

#include <boost/python.hpp>
#include "Reader.hpp"

using namespace boost::python;

BOOST_PYTHON_MODULE( protocol )
{
  class_<Reader>("Reader")
          .def( "Reader", init<const char*>( args("path") ))
          .def( "Reader", init<Device*>( args("device") ))
          .def("run", &Reader::run)
          .add_property("device", make_getter(&Reader::device));
}

#endif //CARS_READERWRAPPER_HPP
