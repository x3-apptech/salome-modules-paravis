This light application was built to mimick the key elements at hand
when integrating ParaView into SALOME (i.e. when designing PARAVIS).

Notably the following classes are (almost) a copy/paste of what is 
found in the PVViewer subfolder of GUI:
	PVViewer_Core
	PVViewer_GUIElements
	PVViewer_Behaviors

The application should have a boot sequence similar to the start-up
of the PVViewer in SALOME, or to the activation of the PARAVIS module.

The main executable is called
	paraLight 
and is *not* installed (it can be executed from the build directory).
