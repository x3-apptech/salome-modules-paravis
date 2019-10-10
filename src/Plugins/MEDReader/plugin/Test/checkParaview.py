import os
import os.path
paraviewBinDir = os.getenv('PARAVIEW_BIN_DIR', "")
paraviewFile = paraviewBinDir + "/paraview"
pvserverFile = paraviewBinDir + "/pvserver"
assert(os.access(paraviewFile, os.X_OK))
assert(os.access(pvserverFile, os.X_OK))
