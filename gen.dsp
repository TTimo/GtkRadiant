# Microsoft Developer Studio Project File - Name="gen" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=gen - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gen.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gen.mak" CFG="gen - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gen - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "gen - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "gen"
# PROP Scc_LocalPath "."
MTL=midl.exe

!IF  "$(CFG)" == "gen - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "gen___Win32_Release"
# PROP BASE Intermediate_Dir "gen___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "gen___Win32_Release"
# PROP Intermediate_Dir "gen___Win32_Release"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "gen - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "gen___Win32_Debug"
# PROP BASE Intermediate_Dir "gen___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "gen___Win32_Debug"
# PROP Intermediate_Dir "gen___Win32_Debug"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "gen - Win32 Release"
# Name "gen - Win32 Debug"
# Begin Source File

SOURCE=.\include\aboutmsg.default
# End Source File
# Begin Source File

SOURCE=.\gen.readme

!IF  "$(CFG)" == "gen - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=.\gen.readme

BuildCmds= \
	run_python.bat makeversion.py

"include/version.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"include/aboutmsg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"include/RADIANT_MAJOR" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"include/RADIANT_MINOR" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "gen - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=.\gen.readme

BuildCmds= \
	run_python.bat makeversion.py

"include/version.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"include/aboutmsg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\makeversion.py
# End Source File
# Begin Source File

SOURCE=.\run_python.bat
# End Source File
# Begin Source File

SOURCE=.\include\version.default
# End Source File
# End Target
# End Project
