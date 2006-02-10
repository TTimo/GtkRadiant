# Microsoft Developer Studio Project File - Name="picomodel" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=picomodel - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "picomodel.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "picomodel.mak" CFG="picomodel - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "picomodel - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "picomodel - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "picomodel"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "picomodel - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O2 /I ".." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "picomodel - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I ".." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "picomodel - Win32 Release"
# Name "picomodel - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "lwo"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\lwo\clip.c
# End Source File
# Begin Source File

SOURCE=.\lwo\envelope.c
# End Source File
# Begin Source File

SOURCE=.\lwo\list.c
# End Source File
# Begin Source File

SOURCE=.\lwo\lwio.c
# End Source File
# Begin Source File

SOURCE=.\lwo\lwo2.c
# End Source File
# Begin Source File

SOURCE=.\lwo\lwob.c
# End Source File
# Begin Source File

SOURCE=.\lwo\pntspols.c
# End Source File
# Begin Source File

SOURCE=.\lwo\surface.c
# End Source File
# Begin Source File

SOURCE=.\lwo\vecmath.c
# End Source File
# Begin Source File

SOURCE=.\lwo\vmap.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\picointernal.c
# End Source File
# Begin Source File

SOURCE=.\picomodel.c
# End Source File
# Begin Source File

SOURCE=.\picomodules.c
# End Source File
# Begin Source File

SOURCE=.\pm_3ds.c
# End Source File
# Begin Source File

SOURCE=.\pm_ase.c
# End Source File
# Begin Source File

SOURCE=.\pm_fm.c
# End Source File
# Begin Source File

SOURCE=.\pm_lwo.c
# End Source File
# Begin Source File

SOURCE=.\pm_md2.c
# End Source File
# Begin Source File

SOURCE=.\pm_md3.c
# End Source File
# Begin Source File

SOURCE=.\pm_mdc.c
# End Source File
# Begin Source File

SOURCE=.\pm_ms3d.c
# End Source File
# Begin Source File

SOURCE=.\pm_obj.c
# End Source File
# End Group
# Begin Group "include"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\lwo\lwo2.h
# End Source File
# Begin Source File

SOURCE=.\picointernal.h
# End Source File
# Begin Source File

SOURCE=..\picomodel.h
# End Source File
# Begin Source File

SOURCE=.\pm_fm.h
# End Source File
# End Group
# End Target
# End Project
