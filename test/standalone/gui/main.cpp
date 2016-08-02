// Copyright (C) 2010-2016  CEA/DEN, EDF R&D
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
// Author: Adrien Bruneton (CEA)

#include <iostream>

#include "PVViewer_Core.h"
#include "PVViewer_Behaviors.h"
#include "PLMainWindow.hxx"

#include <pqApplicationCore.h>
#include <pqSettings.h>
#include <pqParaViewBehaviors.h>
#include <pqServerResource.h>
#include <pqServerConnectReaction.h>
#include <pqServerDisconnectReaction.h>

int main(int argc, char ** argv)
{
  std::cout << "Starting LightParaView ..." << std::endl;

  std::cout << "Init Qt app ..." << std::endl;
  QApplication qtapp(argc, argv);
  QApplication::setApplicationName("LightPara");

  std::cout << "Create Qt main window ..." << std::endl;
  PLMainWindow * para_widget = new PLMainWindow();

  std::cout << "Init appli core ..." << std::endl;
  PVViewer_Core::ParaviewInitApp(para_widget, NULL);

  std::cout << "Binding ParaView widget in QMainWindow ..." << std::endl;
  para_widget->finishUISetup();

  /* Install event filter */
//  std::cout << "Install event filter ..." << std::endl;
//  pqPVApplicationCore * plApp = PVViewer_Core::GetPVApplication();
//  QApplication::instance()->installEventFilter(plApp);

  std::cout << "Init behaviors ..." << std::endl;
  PVViewer_Core::ParaviewInitBehaviors(true, para_widget);

  //para_widget->updateActiveServer();

  std::cout << "Load config ..." << std::endl;
  PVViewer_Core::ParaviewLoadConfigurations(QString(":/LightPara/Configuration"));

  /* Inspired from ParaView source code:
   * leave time for the GUI to update itself before displaying the main window: */
  QApplication::instance()->processEvents();

  // Try to connect
//  std::cout << "about to try to connect ...\n";
//  const char * server_url = "cs://localhost";
//  if (!pqServerConnectReaction::connectToServer(pqServerResource(server_url)))
//    {
//      std::cerr << "Could not connect to requested server \""
//          << server_url << "\". Creating default builtin connection.\n";
//    }

  /* ... and GO: */
  std::cout << "Show !" << std::endl;
  para_widget->show();
  int ret_code = qtapp.exec();

  /* then disconnect and leave nicely */
  //pqServerDisconnectReaction::disconnectFromServer();

  std::cout << "Clean up ..." << std::endl;
  PVViewer_Core::ParaviewCleanup();

  delete para_widget;

  std::cout << "Done." << std::endl;
  return ret_code;
}



