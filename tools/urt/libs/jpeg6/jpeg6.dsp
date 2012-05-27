# Microsoft Developer Studio Project File - Name="jpeg6" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=jpeg6 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jpeg6.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jpeg6.mak" CFG="jpeg6 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jpeg6 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "jpeg6 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "jpeg6"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jpeg6 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
MTL=midl.exe
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /O2 /I "../" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "jpeg6 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
MTL=midl.exe
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Z7 /Od /I "../" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "jpeg6 - Win32 Release"
# Name "jpeg6 - Win32 Debug"
# Begin Source File

SOURCE=.\Jchuff.h
# End Source File
# Begin Source File

SOURCE=.\JCOMAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\Jconfig.h
# End Source File
# Begin Source File

SOURCE=.\JDAPIMIN.cpp
# End Source File
# Begin Source File

SOURCE=.\JDAPISTD.cpp
# End Source File
# Begin Source File

SOURCE=.\JDATASRC.cpp
# End Source File
# Begin Source File

SOURCE=.\JDCOEFCT.cpp
# End Source File
# Begin Source File

SOURCE=.\JDCOLOR.cpp
# End Source File
# Begin Source File

SOURCE=.\Jdct.h
# End Source File
# Begin Source File

SOURCE=.\JDDCTMGR.cpp
# End Source File
# Begin Source File

SOURCE=.\JDHUFF.cpp
# End Source File
# Begin Source File

SOURCE=.\Jdhuff.h
# End Source File
# Begin Source File

SOURCE=.\JDINPUT.cpp
# End Source File
# Begin Source File

SOURCE=.\JDMAINCT.cpp
# End Source File
# Begin Source File

SOURCE=.\JDMARKER.cpp
# End Source File
# Begin Source File

SOURCE=.\JDMASTER.cpp
# End Source File
# Begin Source File

SOURCE=.\JDPOSTCT.cpp
# End Source File
# Begin Source File

SOURCE=.\JDSAMPLE.cpp
# End Source File
# Begin Source File

SOURCE=.\JDTRANS.cpp
# End Source File
# Begin Source File

SOURCE=.\JERROR.cpp
# End Source File
# Begin Source File

SOURCE=.\Jerror.h
# End Source File
# Begin Source File

SOURCE=.\JFDCTFLT.cpp
# End Source File
# Begin Source File

SOURCE=.\JIDCTFLT.cpp
# End Source File
# Begin Source File

SOURCE=.\Jinclude.h
# End Source File
# Begin Source File

SOURCE=.\JMEMMGR.cpp
# End Source File
# Begin Source File

SOURCE=.\JMEMNOBS.cpp
# End Source File
# Begin Source File

SOURCE=.\Jmemsys.h
# End Source File
# Begin Source File

SOURCE=.\Jmorecfg.h
# End Source File
# Begin Source File

SOURCE=.\Jpegint.h
# End Source File
# Begin Source File

SOURCE=.\JPGLOAD.cpp
# End Source File
# Begin Source File

SOURCE=.\JUTILS.cpp
# End Source File
# Begin Source File

SOURCE=.\Jversion.h
# End Source File
# End Target
# End Project
