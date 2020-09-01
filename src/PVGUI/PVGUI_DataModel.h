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
// Author : Adrien Bruneton (CEA)
//

#ifndef PVGUIDATAMODEL_H_
#define PVGUIDATAMODEL_H_

#include <LightApp_DataModel.h>

#include <QObject>
#include <QString>
#include <QStringList>

class PVGUI_Module;
class CAM_Study;

class PVGUI_DataModel: public LightApp_DataModel
{
  Q_OBJECT

public:
  PVGUI_DataModel( PVGUI_Module* theModule );
  virtual ~PVGUI_DataModel();

  virtual bool dumpPython( const QString&,  CAM_Study*, bool, QStringList& );
  virtual bool open( const QString&, CAM_Study*, QStringList );
  virtual bool save( QStringList& );
  virtual bool saveAs( const QString& url, CAM_Study* study, QStringList& theListOfFiles );
  virtual bool create( CAM_Study* );

private:
  void publishComponent(CAM_Study*);

  bool createAndCheckRestoreFlag(const QString& tmpdir, QStringList& listOfFiles, bool & alreadyThere);

  QString              myStudyURL;

  static const QString RESTORE_FLAG_FILE;
};

#endif /* PVGUIDATAMODEL_H_ */
