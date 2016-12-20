#ifndef _VISU_HXX_
#define _VISU_HXX_

#include <string>
#include "MEDCouplingFieldDouble.hxx"

namespace MEDCoupling
{
  class MEDCouplingFieldDouble;
}

class vtkCPProcessor;
class vtkDataSet;

class Visualization
{
  vtkCPProcessor* Processor;

  //private :
  public :
    void CatalystInitialize(const std::string& pipeline);
    void CatalystFinalize();
    void CatalystCoProcess(vtkDataSet *VTKGrid, double time, unsigned int timeStep);
    void ConvertToVTK(MEDCoupling::MEDCouplingFieldDouble* field, vtkDataSet *&VTKGrid);
  public :
    Visualization();
    void run(MEDCoupling::MEDCouplingFieldDouble*, const std::string& pathPipeline);
};

#endif //_VISU_HXX_
