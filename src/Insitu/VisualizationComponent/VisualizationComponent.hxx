// Copyright (C) 2016-2019  CEA/DEN, EDF R&D
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

#ifndef _VisualizationComponent_HXX_
#define _VisualizationComponent_HXX_

#include <SALOME_Component.hh>
#include "Superv_Component_i.hxx"
#include "PARAVIS.hh"
#include "MPIObject_i.hxx"

//COMPODEFS


void * th_Visualize(void * s);
typedef struct {
  int ip; // mpi process id
  Engines::IORTab* tior;
  SALOME_MED::ParaMEDCouplingFieldDoubleCorbaInterface_ptr field;
const char* path_python_file;
} thread_Visualize_struct;


//ENDDEF

class VisualizationComponent_i: public virtual POA_PARAVIS_ORB::VisualizationComponent,
                      
                      public virtual MPIObject_i,
                      public virtual Engines_Component_i
{
  public:
    VisualizationComponent_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa,
              PortableServer::ObjectId * contId,
              const char *instanceName, const char *interfaceName,
              bool regist = true);
    VisualizationComponent_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa,
              Engines::Container_ptr container,
              const char *instanceName, const char *interfaceName,
              bool regist = true);
    virtual ~VisualizationComponent_i();
    void Visualize(SALOME_MED::ParaMEDCouplingFieldDoubleCorbaInterface_ptr field,const char* path_python_file);
};

extern "C"
{
    PortableServer::ObjectId * VisualizationComponentEngine_factory( CORBA::ORB_ptr orb,
                                                      PortableServer::POA_ptr poa,
                                                      PortableServer::ObjectId * contId,
                                                      const char *instanceName,
                                                      const char *interfaceName);
}
#endif

