/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStaticDataSetSurfaceFilter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkStaticMeshObjectFactory.h"

#include <vtkCollectionIterator.h>
#include <vtkObjectFactory.h>
#include <vtkObjectFactoryCollection.h>
#include <vtkVersion.h>

#include "vtkStaticDataSetSurfaceFilter.h"
#include "vtkStaticEnSight6BinaryReader.h"
#include "vtkStaticEnSight6Reader.h"
#include "vtkStaticEnSightGoldBinaryReader.h"
#include "vtkStaticEnSightGoldReader.h"
#include "vtkStaticPlaneCutter.h"

#ifdef PARAVIEW_USE_MPI
#include "vtkStaticPUnstructuredGridGhostCellsGenerator.h"
#endif

vtkStandardNewMacro(vtkStaticMeshObjectFactory);

VTK_CREATE_CREATE_FUNCTION(vtkStaticDataSetSurfaceFilter);
VTK_CREATE_CREATE_FUNCTION(vtkStaticPlaneCutter);
VTK_CREATE_CREATE_FUNCTION(vtkStaticEnSight6BinaryReader);
VTK_CREATE_CREATE_FUNCTION(vtkStaticEnSight6Reader);
VTK_CREATE_CREATE_FUNCTION(vtkStaticEnSightGoldReader);
VTK_CREATE_CREATE_FUNCTION(vtkStaticEnSightGoldBinaryReader);

#ifdef PARAVIEW_USE_MPI
VTK_CREATE_CREATE_FUNCTION(vtkStaticPUnstructuredGridGhostCellsGenerator);
#endif

vtkStaticMeshObjectFactory::vtkStaticMeshObjectFactory()
{
  vtkDebugMacro("Create vtkStaticMeshObjectFactory");

  this->RegisterOverride("vtkDataSetSurfaceFilter", "vtkStaticDataSetSurfaceFilter",
    "StaticDataSetSurfaceFilter", 1, vtkObjectFactoryCreatevtkStaticDataSetSurfaceFilter);
  this->RegisterOverride("vtkPlaneCutter", "vtkStaticPlaneCutter", "StaticPlaneCutter", 1,
    vtkObjectFactoryCreatevtkStaticPlaneCutter);
  this->RegisterOverride("vtkEnSight6BinaryReader", "vtkStaticEnSight6BinaryReader", "StaticEnSight6BinaryReader", 1,
    vtkObjectFactoryCreatevtkStaticEnSight6BinaryReader);
  this->RegisterOverride("vtkEnSight6Reader", "vtkStaticEnSight6Reader", "StaticEnSight6Reader", 1,
    vtkObjectFactoryCreatevtkStaticEnSight6Reader);
  this->RegisterOverride("vtkEnSightGoldReader", "vtkStaticEnSight6BinaryReader", "StaticEnSight6BinaryReader", 1,
    vtkObjectFactoryCreatevtkStaticEnSightGoldReader);
  this->RegisterOverride("vtkEnSightGoldBinaryReader", "vtkStaticEnSightGoldBinaryReader", "StaticEnSightGoldBinaryReader", 1,
    vtkObjectFactoryCreatevtkStaticEnSightGoldBinaryReader);

#ifdef PARAVIEW_USE_MPI
  this->RegisterOverride("vtkPUnstructuredGridGhostCellsGenerator",
    "vtkStaticPUnstructuredGridGhostCellsGenerator", "StaticPUnstructuredGridGhostCellsGenerator",
    1, vtkObjectFactoryCreatevtkStaticPUnstructuredGridGhostCellsGenerator);
#endif
}

vtkStaticMeshObjectFactory::~vtkStaticMeshObjectFactory()
{
  vtkDebugMacro("Delete vtkStaticMeshObjectFactory");
}

void vtkStaticMeshObjectFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "VTK Static Mesh Extension Factory" << endl;
}

const char* vtkStaticMeshObjectFactory::GetVTKSourceVersion()
{
  return VTK_SOURCE_VERSION;
}

const char* vtkStaticMeshObjectFactory::GetDescription()
{
  return "VTK Static Mesh Extension Factory";
}

class StaticFactoryInitialize
{
public:
  StaticFactoryInitialize()
  {
    bool hasStaticPluginFactory = false;
    vtkObjectFactoryCollection* collection = vtkObjectFactory::GetRegisteredFactories();
    collection->InitTraversal();
    vtkObjectFactory* f = collection->GetNextItem();
    while (f)
    {
      if (f->IsA("vtkStaticMeshObjectFactory"))
      {
        hasStaticPluginFactory = true;
        break;
      }
      f = collection->GetNextItem();
    }
    if (!hasStaticPluginFactory)
    {
      vtkStaticMeshObjectFactory* instance = vtkStaticMeshObjectFactory::New();
      vtkObjectFactory::RegisterFactory(instance);
      instance->Delete();
    }
  }

  virtual ~StaticFactoryInitialize()
  {
    vtkObjectFactoryCollection* collection = vtkObjectFactory::GetRegisteredFactories();
    collection->InitTraversal();
    vtkObjectFactory* f;
    while ((f = collection->GetNextItem()))
    {
      if (f->IsA("vtkStaticMeshObjectFactory"))
      {
        vtkObjectFactory::UnRegisterFactory(f);
        break;
      }
    }
  }
};

static StaticFactoryInitialize StaticFactory;
