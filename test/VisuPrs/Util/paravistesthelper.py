# Copyright (C) 2013-2015  CEA/DEN, EDF R&D
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

import subprocess
import sys, os
import signal
import killSalomeWithPort

## TEMP >>> ###
if not os.getenv("OMNIORB_USER_PATH"):
    os.environ["OMNIORB_USER_PATH"] = os.path.realpath(os.path.expanduser('~'))
    pass
## <<< TEMP ###


class SalomeSession(object):
    def __init__(self, args=[]):
        sys.argv = ['runSalome'] + args

        if "INGUI" in args:
            sys.argv += ["--gui"]
            sys.argv += ["--show-desktop=1"]
            sys.argv += ["--splash=0"]
            #sys.argv += ["--standalone=study"]
            #sys.argv += ["--embedded=SalomeAppEngine,cppContainer,registry,moduleCatalog"]
        else:
            sys.argv += ["--terminal"]
        sys.argv += ["--modules=MED,PARAVIS,GUI"]

        import setenv
        setenv.main(True)

        import runSalome
        runSalome.runSalome()
    pass
#

port = 0

def run_test(command):
  # Run SALOME
  import tempfile
  log = tempfile.NamedTemporaryFile(suffix='_nsport.log', delete=False)
  log.close()
  import salome
  salome_session = SalomeSession(args=["--ns-port-log=%s"%log.name])
  salome.salome_init()
  session_server = salome.naming_service.Resolve('/Kernel/Session')
  if session_server:
      session_server.emitMessage("connect_to_study")
      session_server.emitMessage("activate_viewer/ParaView")
      pass

  global port
  with open(log.name) as f:
      port = int(f.readline())

  # Run test
  #res = subprocess.check_call(command)
  p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  _out, _err = p.communicate()
  res = p.returncode

  # Exit SALOME
  killSalomeWithPort.killMyPort(port)
  os.remove(log.name)

  # :TRICKY: Special case of returncode=127
  # When using paraview in SALOME environment, the following error
  # systematicallty appears when exiting paraview (it's also true when using
  # PARAVIS and exiting SALOME):
  # Inconsistency detected by ld.so: dl-close.c: 738: _dl_close: Assertion `map->l_init_called' failed!
  # For PARAVIS tests purpose, paraview functionalities are accessed in each
  # test; these tests are run in the above subprocess call.
  # The assertion error implies a subprocess return code of 127, and the test
  # status is considered as "failed".
  # The tricky part here is to discard such return codes, waiting for a fix
  # maybe in paraview...
  if res == 127 and _err.startswith("Inconsistency detected by ld.so: dl-close.c"):
      print "    ** THE FOLLOWING MESSAGE IS NOT CONSIDERED WHEN ANALYZING TEST SUCCESSFULNESS **"
      print _err
      print "    ** end of message **"
      res = 0;
  elif _err:
      print "    ** Detected error **"
      print "Error code: ", res
      print _err
      print "    ** end of message **"
      pass

  if _out:
      print _out
  return res
#

def timeout_handler(signum, frame):
    print "FAILED : timeout(" + sys.argv[1] + ") is reached"
    killSalomeWithPort.killMyPort(port)
    exit(1)
#
signal.alarm(abs(int(sys.argv[1])-10))
signal.signal(signal.SIGALRM, timeout_handler)

res = 1
try:
    res = run_test([sys.executable]+sys.argv[2:])
except:
    #import traceback
    #traceback.print_exc()
    pass

exit(res)
