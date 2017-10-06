//
// Created by Ivan on 06.10.17.
//

#ifndef CARS_WORKERTHREADWRAPPER_CPP
#define CARS_WORKERTHREADWRAPPER_CPP

#include "WorkerThread.hpp"
#include <boost/python.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE( protocol )
{
  class_<WorkerThread>("WorkerThread")
          .def( init<>() )
          .def("run", &WorkerThread::run);
}
#endif //CARS_WORKERTHREADWRAPPER_CPP
