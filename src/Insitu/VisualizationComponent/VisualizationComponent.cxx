// Copyright (C) 2016-2020  CEA/DEN, EDF R&D
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

#include "VisualizationComponent.hxx"
#include <string>
#include <unistd.h>
#include <signal.h>
#include <SALOME_NamingService.hxx>
#include <Utils_SALOME_Exception.hxx>
#include "Utils_CorbaException.hxx"
#include <pthread.h>
#include <execinfo.h>

typedef struct
{
  bool exception;
  std::string msg;
} exception_st;

//DEFS

#include "visu.hxx"
#include "MEDCouplingFieldDoubleClient.hxx"
#include "ParaMEDCouplingFieldDoubleServant.hxx"
#include "omniORB4/poa.h"

//ENDDEF


using namespace std;

//! Constructor for component "VisualizationComponent" instance
/*!
 *
 */
VisualizationComponent_i::VisualizationComponent_i(CORBA::ORB_ptr orb,
                     PortableServer::POA_ptr poa,
                     PortableServer::ObjectId * contId,
                     const char *instanceName,
                     const char *interfaceName,
                     bool regist)
          : Engines_Component_i(orb, poa, contId, instanceName, interfaceName,
                                false, regist)
{
  _thisObj = this ;
  _id = _poa->activate_object(_thisObj);
}

VisualizationComponent_i::VisualizationComponent_i(CORBA::ORB_ptr orb,
                     PortableServer::POA_ptr poa,
                     Engines::Container_ptr container,
                     const char *instanceName,
                     const char *interfaceName,
                     bool regist)
          : Engines_Component_i(orb, poa, container, instanceName, interfaceName,
                                false, regist)
{
  _thisObj = this ;
  _id = _poa->activate_object(_thisObj);
}

//! Destructor for component "VisualizationComponent" instance
VisualizationComponent_i::~VisualizationComponent_i()
{
}


void * th_Visualize(void * s)
{
  std::ostringstream msg;
  exception_st *est = new exception_st;
  est->exception = false;
  
  thread_Visualize_struct *st = (thread_Visualize_struct *)s;
  
  try
  {
    
    PARAVIS_ORB::VisualizationComponent_var compo = PARAVIS_ORB::VisualizationComponent::_narrow((*(st->tior))[st->ip]);
    compo->Visualize(st->field,st->path_python_file);
  }
  catch(const SALOME::SALOME_Exception &ex)
  {
    est->exception = true;
    est->msg = ex.details.text;
  }
  catch(const CORBA::Exception &ex)
  {
    est->exception = true;
    msg << "CORBA::Exception: " << ex;
    est->msg = msg.str();
  }
  
  delete st;
  return ((void*)est);
}

void VisualizationComponent_i::Visualize(SALOME_MED::ParaMEDCouplingFieldDoubleCorbaInterface_ptr field,const char* path_python_file)
{
  beginService("VisualizationComponent_i::Visualize");
  void *ret_th;
  pthread_t *th;
  exception_st *est;

  try
    {
      // Run the service in every MPI process
      if(_numproc == 0)
      {
        th = new pthread_t[_nbproc];
        for(int ip=1;ip<_nbproc;ip++)
        {
            thread_Visualize_struct *st = new thread_Visualize_struct;
            st->ip = ip;
            st->tior = _tior;
            st->field = field;
st->path_python_file = path_python_file;
            pthread_create(&(th[ip]),NULL,th_Visualize,(void*)st);
        }
      }
      
//BODY

const MEDCoupling::MEDCouplingFieldDouble * local_field(NULL);
int nb_fields = field->tior()->length();
if(nb_fields == _nbproc)
{
  SALOME_MED::ParaMEDCouplingFieldDoubleCorbaInterface_ptr local_corba_field = 
               SALOME_MED::ParaMEDCouplingFieldDoubleCorbaInterface::_narrow((*(field->tior()))[_numproc]);


  PortableServer::ServantBase *ret;
  try {
    ret=PortableServer::POA::_the_root_poa()->reference_to_servant(local_corba_field);
    std::cerr << "Servant succeeded!" << std::endl;
    MEDCoupling::ParaMEDCouplingFieldDoubleServant* servant_field = 
               dynamic_cast<MEDCoupling::ParaMEDCouplingFieldDoubleServant*>(ret);
    if(servant_field != NULL)
    {
      std::cerr << "In-situ configuration!" << std::endl;
      // same container, same mpi proc, use the pointer directly.
      local_field = servant_field->getPointer();
      ret->_remove_ref();
    }
  }
  catch(...){
    // different container - need to make a copy of the field.
    ret = NULL;
    std::cerr << "Co-processing configuration!" << std::endl;
    local_field = MEDCoupling::MEDCouplingFieldDoubleClient::New(local_corba_field);
  }
}
else if(nb_fields < _nbproc)
{
  if(_numproc < nb_fields)
  {
    SALOME_MED::ParaMEDCouplingFieldDoubleCorbaInterface_ptr local_corba_field = 
               SALOME_MED::ParaMEDCouplingFieldDoubleCorbaInterface::_narrow((*(field->tior()))[_numproc]);
    local_field = MEDCoupling::MEDCouplingFieldDoubleClient::New(local_corba_field);
  }
}
else //nb_fields > _nbproc
{
  int q = nb_fields / _nbproc; // int division
  int r = nb_fields - q * _nbproc;
  int start, end;
  
  if(_numproc < r)
  {
    // get one more field to process
    start = _numproc * (q + 1);
    end = start + q + 1;
  }
  else
  {
    start = r * (q + 1) + (_numproc - r) * q;
    end = start + q;
  }
  
  std::cerr << "Proc n° " << _numproc << ". Merge fields from " << start << " to " << end << std::endl;
  
  std::vector<const MEDCoupling::MEDCouplingFieldDouble *> fieldsToProcess;
  for(int i = start; i < end; i++)
  {
    SALOME_MED::ParaMEDCouplingFieldDoubleCorbaInterface_ptr local_corba_field = 
               SALOME_MED::ParaMEDCouplingFieldDoubleCorbaInterface::_narrow((*(field->tior()))[i]);
    fieldsToProcess.push_back(MEDCoupling::MEDCouplingFieldDoubleClient::New(local_corba_field));
  }
  
  local_field = MEDCoupling::MEDCouplingFieldDouble::MergeFields(fieldsToProcess);
}

Visualization v;
v.run(const_cast<MEDCoupling::MEDCouplingFieldDouble*>(local_field), path_python_file);

//ENDBODY
      if(_numproc == 0)
      {
        for(int ip=1;ip<_nbproc;ip++)
        {
          pthread_join(th[ip],&ret_th);
          est = (exception_st*)ret_th;
          if(est->exception)
          {
              std::ostringstream msg;
              msg << "[" << ip << "] " << est->msg;
              delete est;
              delete[] th;
              THROW_SALOME_CORBA_EXCEPTION(msg.str().c_str(),SALOME::INTERNAL_ERROR);
          }
          delete est;
        }
        delete[] th;
      }
    }
  catch ( const SALOME_Exception & ex)
    {
      THROW_SALOME_CORBA_EXCEPTION(CORBA::string_dup(ex.what()), SALOME::INTERNAL_ERROR);
    }
  catch ( const SALOME::SALOME_Exception & ex)
    {
      throw;
    }
  catch ( const std::exception& ex)
    {
      THROW_SALOME_CORBA_EXCEPTION(CORBA::string_dup(ex.what()), SALOME::INTERNAL_ERROR);
    }
  catch (...)
    {
      THROW_SALOME_CORBA_EXCEPTION("unknown exception", SALOME::INTERNAL_ERROR);
    }
  endService("VisualizationComponent_i::Visualize");
}



extern "C"
{
  PortableServer::ObjectId * VisualizationComponentEngine_factory( CORBA::ORB_ptr orb,
                                                    PortableServer::POA_ptr poa,
                                                    PortableServer::ObjectId * contId,
                                                    const char *instanceName,
                                                    const char *interfaceName)
  {
    MESSAGE("PortableServer::ObjectId * VisualizationComponentEngine_factory()");
    int is_mpi_container;
    bool regist;
    int numproc;
    
    MPI_Initialized(&is_mpi_container);
    if (!is_mpi_container)
    {
      int argc = 0;
      char ** argv = NULL;
      MPI_Init(&argc, &argv);
    }
    
    MPI_Comm_rank( MPI_COMM_WORLD, &numproc );
    regist = ( numproc == 0 );
    VisualizationComponent_i * myEngine = new VisualizationComponent_i(orb, poa, contId, instanceName, interfaceName, regist);
    return myEngine->getId() ;
  }
}
