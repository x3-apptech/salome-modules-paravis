// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

//  SALOME Container : implementation of container and engine for Kernel
//  File   : Container_init_python.cxx
//  Author : Paul RASCLE, EDF
//  Module : KERNEL
//  $Header$
//
#include <time.h>
#ifndef WIN32
  #include <sys/time.h>
#endif
#include <iostream>

//#include "utilities.h"
#define MESSAGE(a)

#include "Container_init_python.hxx"

void KERNEL_PYTHON::init_python(int argc, char **argv)
{
  if (Py_IsInitialized())
    {
      MESSAGE("Python already initialized");
      return;
    }
  MESSAGE("=================================================================");
  MESSAGE("Python Initialization...");
  MESSAGE("=================================================================");
  // set stdout to line buffering (aka C++ std::cout)
  setvbuf(stdout, (char *)NULL, _IOLBF, BUFSIZ);
  char* salome_python=getenv("SALOME_PYTHON");
  //size_t size_salome_python = sizeof(salome_python) / sizeof(salome_python[0]); // unused
  if(salome_python != 0)
	  Py_SetProgramName(Py_DecodeLocale(salome_python, NULL));

  Py_Initialize(); // Initialize the interpreter
  wchar_t **w_argv = new wchar_t*[argc];
  for (int i = 0; i < argc; i++)
	  w_argv[i] = Py_DecodeLocale(argv[i], NULL);
  PySys_SetArgv(argc, w_argv);
  PyRun_SimpleString("import threading\n");
  PyEval_InitThreads(); // Create (and acquire) the interpreter lock
  PyThreadState *pts = PyGILState_GetThisThreadState(); 
  PyEval_ReleaseThread(pts);  
}

