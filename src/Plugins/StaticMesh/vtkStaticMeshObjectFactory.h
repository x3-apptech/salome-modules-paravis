/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStaticDataSetSurfaceFilter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkStaticMeshObjectFactory
 * @brief   Generate static version of dataset and filter for statix mesh plugin
 *
 * vtkStaticMeshObjectFactory is a vtk object factory, instantiating static version
 * of some dataset and filters.
*/

#ifndef vtkStaticMeshObjectFactory_h
#define vtkStaticMeshObjectFactory_h

#include "vtkObjectFactory.h" // Must be included before singletons

class vtkStaticMeshObjectFactory : public vtkObjectFactory
{
public:
  vtkTypeMacro(vtkStaticMeshObjectFactory, vtkObjectFactory);
  static vtkStaticMeshObjectFactory* New();
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /**
   * All sub-classes of vtkObjectFactory must return the version of
   * VTK they were built with.  This should be implemented with the macro
   * VTK_SOURCE_VERSION and NOT a call to vtkVersion::GetVTKSourceVersion.
   * As the version needs to be compiled into the file as a string constant.
   * This is critical to determine possible incompatible dynamic factory loads.
   */
  const char* GetVTKSourceVersion() VTK_OVERRIDE;

  /**
   * Return a descriptive string describing the factory.
   */
  const char* GetDescription() VTK_OVERRIDE;

protected:
  vtkStaticMeshObjectFactory();
  ~vtkStaticMeshObjectFactory() VTK_OVERRIDE;

private:
  vtkStaticMeshObjectFactory(const vtkStaticMeshObjectFactory&) VTK_DELETE_FUNCTION;
  void operator=(const vtkStaticMeshObjectFactory&) VTK_DELETE_FUNCTION;
};

#endif
