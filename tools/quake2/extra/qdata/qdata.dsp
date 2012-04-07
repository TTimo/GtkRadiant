# Microsoft Developer Studio Project File - Name="qdata" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=qdata - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "qdata.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "qdata.mak" CFG="qdata - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qdata - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "qdata - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qdata - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "qdata - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386

!ENDIF 

# Begin Target

# Name "qdata - Win32 Release"
# Name "qdata - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\common\bspfile.c
# End Source File
# Begin Source File

SOURCE=..\common\cmdlib.c
# End Source File
# Begin Source File

SOURCE=.\images.c
# End Source File
# Begin Source File

SOURCE=..\common\l3dslib.c
# End Source File
# Begin Source File

SOURCE=..\common\lbmlib.c
# End Source File
# Begin Source File

SOURCE=..\common\mathlib.c
# End Source File
# Begin Source File

SOURCE=..\common\mdfour.c
# End Source File
# Begin Source File

SOURCE=.\models.c
# End Source File
# Begin Source File

SOURCE=.\qdata.c
# End Source File
# Begin Source File

SOURCE=..\common\scriplib.c
# End Source File
# Begin Source File

SOURCE=.\sprites.c
# End Source File
# Begin Source File

SOURCE=.\tables.c
# End Source File
# Begin Source File

SOURCE=..\common\threads.c
# End Source File
# Begin Source File

SOURCE=..\common\trilib.c
# End Source File
# Begin Source File

SOURCE=.\video.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\anorms.h
# End Source File
# Begin Source File

SOURCE=..\common\bspfile.h
# End Source File
# Begin Source File

SOURCE=..\common\cmdlib.h
# End Source File
# Begin Source File

SOURCE=..\common\l3dslib.h
# End Source File
# Begin Source File

SOURCE=..\common\lbmlib.h
# End Source File
# Begin Source File

SOURCE=..\common\mathlib.h
# End Source File
# Begin Source File

SOURCE=.\modelgen.h
# End Source File
# Begin Source File

SOURCE=.\qdata.h
# End Source File
# Begin Source File

SOURCE=..\common\qfiles.h
# End Source File
# Begin Source File

SOURCE=..\common\scriplib.h
# End Source File
# Begin Source File

SOURCE=..\common\threads.h
# End Source File
# Begin Source File

SOURCE=..\common\trilib.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
