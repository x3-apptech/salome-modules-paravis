// Copyright (C) 2010-2021  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// Author : Adrien Bruneton (CEA)
//
#include <Python.h>
#include <pqPVApplicationCore.h>
#include <QApplication>
#include "PyInterp_Utils.h"
#include "Container_init_python.hxx"

#include <iostream>

int main(int argc, char ** argv)
{
  // Initialize Python in the way SALOME does:
  KERNEL_PYTHON::init_python(argc,argv);

  // The below should always work (illustration of SALOME lock protection)
  {
    PyLockWrapper lock;  // if commented, the below will crash:

    // Nothing important, just a bunch of calls to some Py* functions!
    PyRun_SimpleString("import base64");
    /*PyObject * sysmod = */PyImport_AddModule("sys"); // todo: unused
    //PyObject* sysdict = PyModule_GetDict(sysmod); // todo: unused
    //PyObject* tmp = PyDict_GetItemString(sysdict, "modules"); // todo: unused
  }
  std::cout << "Done with Py call" << std::endl;

  // Now the Qt part:
  QApplication qtapp(argc, argv);
  std::cout << "Done with Qt init" << std::endl;

  // And finally the ParaView part:
  pqPVApplicationCore* myCoreApp = new pqPVApplicationCore (argc, argv);
  std::cout << "Done with PV init" << std::endl;
  // Make sure compilation of ParaView was made with Python support:
  if (!myCoreApp->pythonManager())
    return -1;
  delete myCoreApp;

  std::cout << "Done with PV deletion" << std::endl;

  return 0;
}
