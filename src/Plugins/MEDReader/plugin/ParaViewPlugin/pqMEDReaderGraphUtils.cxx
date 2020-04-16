// Copyright (C) 2010-2020  CEA/DEN, EDF R&D
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
// Author : Anthony Geay

#include "pqMEDReaderGraphUtils.h"

#include "vtkGraph.h"
#include "vtkTree.h"
#include "vtkNew.h"
#include "vtkStringArray.h"
#include "vtkDataSetAttributes.h"

namespace pqMedReaderGraphUtils
{
void getCurrentTS(vtkGraph* graph, vtkIdType id, QStringList& dts, QStringList& its,
                  QStringList& tts)
{
  vtkNew<vtkTree> tree;
  tree->CheckedShallowCopy(graph);
  vtkStringArray* names = vtkStringArray::SafeDownCast(
    tree->GetVertexData()->GetAbstractArray("Names"));
  vtkIdType root = tree->GetRoot();
  vtkIdType fst = tree->GetChild(root, 0); // FieldsStatusTree
  vtkIdType tsr = tree->GetChild(tree->GetChild(fst, id*2), 0); //Time Step root node
  vtkIdType tmp;
  for (int i = 0; i < tree->GetNumberOfChildren(tsr); i++)
    {
    // Each Time Step
    // Recover step
    tmp = tree->GetChild(tsr, i);
    dts << QString(names->GetValue(tmp));

    // Recover mode
    tmp = tree->GetChild(tmp, 0);
    its << QString(names->GetValue(tmp));

    // Recover value
    tmp = tree->GetChild(tmp, 0);
    tts << QString(names->GetValue(tmp));
    }
}

int getMaxNumberOfTS(vtkGraph* graph)
{
  vtkNew<vtkTree> tree;
  tree->CheckedShallowCopy(graph);
  vtkStringArray* names = vtkStringArray::SafeDownCast(
    tree->GetVertexData()->GetAbstractArray("Names"));
  vtkIdType root = tree->GetRoot();
  vtkIdType fst = tree->GetChild(root, 0); // FieldsStatusTree
  vtkIdType tmp;
  int nbTS = 0;
  for (int i = 0; i < tree->GetNumberOfChildren(fst); i += 2)
    {
    // Look for max time steps
    tmp = tree->GetChild(tree->GetChild(fst, i), 0);
    nbTS = std::max(nbTS, names->GetVariantValue(tmp).ToInt());
    }
  return nbTS;
}
}
