# Copyright (C) 2007-2014  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
# Author : Adrien Bruneton (CEA)
#

import PARAVIS_ORB__POA
import SALOME_ComponentPy
import SALOME_DriverPy
import SALOMEDS
import PARAVIS_utils
import subprocess as subp
from time import sleep
import os
from SALOME_utilities import MESSAGE

class PARAVIS_Impl:
    """ The core implementation (non CORBA, or Study related).
        See the IDL for the documentation.
    """
    MAX_PVSERVER_PORT_TRIES = 10
    
    def __init__(self):
        self.pvserverPort = -1
        self.pvserverPop = None  # Popen object from subprocess module
        self.liveConnection = None
        self.lastTrace = ""
    
    """
    Private. Keeps a connection to the PVServer to prevent 
    it from stopping because all connections from GUI or scripts have been closed.
    """
    def __startKeepAlive(self, host, port):
        print "[PARAVIS] 0 __startKeepAlive", host, port
        from paraview import simple as pvs
        self.liveConnection = pvs.Connect(host, port)
        print "[PARAVIS] 1 __startKeepAlive", self.liveConnection
                                           
    def FindOrStartPVServer( self, port ):
        print "[PARAVIS] FindOrStartPVServer"
        MESSAGE("[PARAVIS] FindOrStartPVServer")
        host = "localhost"
        alive = True
        if self.pvserverPop is None:
            alive = False
        else:
            # Poll active server to check if still alive
            self.pvserverPop.poll()
            if not self.pvserverPop.returncode is None:  # server terminated
                alive = False
        
        if alive:
            return "cs://%s:%d" % (host, self.pvserverPort)  
          
        # (else) Server not alive, start it:
        pvRootDir = os.getenv("PARAVIEW_ROOT_DIR")
        pvServerPath = os.path.join(pvRootDir, 'bin', 'pvserver')
        opt = []
        if port <= 0:
            port = 11111
        currPort = port
        success = False
        while not success and (currPort - port) < self.MAX_PVSERVER_PORT_TRIES:
            self.pvserverPop = subp.Popen([pvServerPath, "--multi-clients", "--server-port=%d" % currPort])
            sleep(2)
            # Is PID still alive? If yes, consider that the launch was successful
            self.pvserverPop.poll()
            if self.pvserverPop.returncode is None:
                success = True
                self.pvserverPort = currPort
                MESSAGE("[PARAVIS] pvserver successfully launched on port %d" % currPort)
            currPort += 1
        if (currPort - port) == self.MAX_PVSERVER_PORT_TRIES:
            self.pvserverPop = None
            raise SalomeException("Unable to start PVServer after %d tries!" % self.MAX_PVSERVER_PORT_TRIES)
#         self.__startKeepAlive(host, self.pvserverPort)
        return "cs://%s:%d" % (host, self.pvserverPort)

    def StopPVServer( self ):
        MESSAGE("[PARAVIS] Trying to stop PVServer (sending KILL) ...")
        if not self.pvserverPop is None:
            self.pvserverPop.poll()
            if not self.pvserverPop.returncode is None:
                # Send KILL if still running:
                self.pvserverPop.kill()
                self.liveConnection = None
    
    def PutPythonTraceStringToEngine( self, t ):
        self.lastTrace = t
        
    def GetPythonTraceString (self):
        return self.lastTrace
    
class PARAVIS(PARAVIS_ORB__POA.PARAVIS_Gen,
              SALOME_ComponentPy.SALOME_ComponentPy_i,
              SALOME_DriverPy.SALOME_DriverPy_i,
              PARAVIS_Impl):
  
    """
    Construct an instance of PARAVIS module engine.
    The class PARAVIS implements CORBA interface PARAVIS_Gen (see PARAVIS_Gen.idl).
    It is inherited from the classes SALOME_ComponentPy_i (implementation of
    Engines::EngineComponent CORBA interface - SALOME component) and SALOME_DriverPy_i
    (implementation of SALOMEDS::Driver CORBA interface - SALOME module's engine).
    """
    def __init__ ( self, orb, poa, contID, containerName, instanceName, 
                   interfaceName ):
        SALOME_ComponentPy.SALOME_ComponentPy_i.__init__(self, orb, poa,
                    contID, containerName, instanceName, interfaceName, 0)
        SALOME_DriverPy.SALOME_DriverPy_i.__init__(self, interfaceName)
        PARAVIS_Impl.__init__(self)
        #
        self._naming_service = SALOME_ComponentPy.SALOME_NamingServicePy_i( self._orb )
        #

    """ Override base class destroy to make sure we try to kill the pvserver
        before leaving.
    """
    def destroy(self):    
        self.StopPVServer()
        # Invokes super():
        SALOME_ComponentPy.destroy(self)
      
    """
    Get version information.
    """
    def getVersion( self ):
        import salome_version
        return salome_version.getVersion("PARAVIS", True)

    def GetIOR(self):
        return PARAVIS_utils.getEngineIOR()

    """
    Create object.
    """
    def createObject( self, study, name ):
        self._createdNew = True # used for getModifiedData method
        builder = study.NewBuilder()
        father  = findOrCreateComponent( study )
        object  = builder.NewObject( father )
        attr    = builder.FindOrCreateAttribute( object, "AttributeName" )
        attr.SetValue( name )
        attr    = builder.FindOrCreateAttribute( object, "AttributeLocalID" )
        attr.SetValue( objectID() )
        pass

    """
    Dump module data to the Python script.
    """
    def DumpPython( self, study, isPublished ):
        print "@@@@ DumpPython"
        abuffer = []
        abuffer.append( "def RebuildData( theStudy ):" )
        names = []
        father = study.FindComponent( moduleName() )
        if father:
            iter = study.NewChildIterator( father )
            while iter.More():
                name = iter.Value().GetName()
                if name: names.append( name )
                iter.Next()
                pass
            pass
        if names:
            abuffer += [ "  from batchmode_salome import lcc" ]
            abuffer += [ "  import PARAVIS_ORB" ]
            abuffer += [ "  " ]
            abuffer += [ "  pyhello = lcc.FindOrLoadComponent( 'FactoryServerPy', '%s' )" % moduleName() ]
            abuffer += [ "  " ]
            abuffer += [ "  pyhello.createObject( theStudy, '%s' )" % name for name in names ]
            pass
        abuffer += [ "  " ]
        abuffer.append( "  pass" )
        abuffer.append( "\0" )
        return ("\n".join( abuffer ), 1)
  
    """
    Import file to restore module data
    """
    def importData(self, studyId, dataContainer, options):
      print "@@@@ ImportData"
      # get study by Id
      obj = self._naming_service.Resolve("myStudyManager")
      myStudyManager = obj._narrow(SALOMEDS.StudyManager)
      study = myStudyManager.GetStudyByID(studyId)
      # create all objects from the imported stream
      stream = dataContainer.get()
      for objname in stream.split("\n"):
        if len(objname) != 0:
          self.createObject(study, objname)
      self._createdNew = False # to store the modification of the study information later
      return ["objects"] # identifier what is in this file
 
    def getModifiedData(self, studyId):
      print "@@@@ GetModifiedData"
      if self._createdNew:
        # get study by Id
        obj = self._naming_service.Resolve("myStudyManager")
        myStudyManager = obj._narrow(SALOMEDS.StudyManager)
        study = myStudyManager.GetStudyByID(studyId)
        # iterate all objects to get their names and store this information in stream
        stream=""
        father = study.FindComponent( moduleName() )
        if father:
            iter = study.NewChildIterator( father )
            while iter.More():
                name = iter.Value().GetName()
                stream += name + "\n"
                iter.Next()
        # store stream to the temporary file to send it in DataContainer
        dataContainer = SALOME_DataContainerPy_i(stream, "", "objects", False, True)
        aVar = dataContainer._this()
        return [aVar]
      return []
