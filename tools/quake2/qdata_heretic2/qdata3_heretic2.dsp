# Microsoft Developer Studio Project File - Name="qdata3_heretic2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=qdata3_heretic2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "qdata3_heretic2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "qdata3_heretic2.mak" CFG="qdata3_heretic2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qdata3_heretic2 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "qdata3_heretic2 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qdata3_heretic2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX- /Zi /O2 /I "./common" /I "./qcommon" /I "." /I "..\..\..\..\libxml2\include" /I "..\..\..\libs" /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib l_net.lib mathlib.lib /nologo /subsystem:console /map /machine:I386 /out:"Release/qdata3.exe" /libpath:"..\..\..\libs\l_net\release" /libpath:"..\..\..\..\libxml2\win32\libxml2\release_so" /libpath:"..\..\..\libs\mathlib\release"

!ELSEIF  "$(CFG)" == "qdata3_heretic2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX- /ZI /Od /I "./common" /I "./qcommon" /I "." /I "..\..\..\..\libxml2\include" /I "..\..\..\libs" /I "..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib l_net.lib mathlib.lib /nologo /subsystem:console /profile /map /debug /machine:I386 /out:"Debug/qdata3.exe" /libpath:"..\..\..\libs\l_net\debug" /libpath:"..\..\..\..\libxml2\win32\libxml2\debug_so" /libpath:"..\..\..\libs\mathlib\debug"

!ENDIF 

# Begin Target

# Name "qdata3_heretic2 - Win32 Release"
# Name "qdata3_heretic2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\animcomp.c
# End Source File
# Begin Source File

SOURCE=.\book.c
# End Source File
# Begin Source File

SOURCE=.\common\bspfile.c
# End Source File
# Begin Source File

SOURCE=.\common\cmdlib.c
# End Source File
# Begin Source File

SOURCE=.\fmodels.c
# End Source File
# Begin Source File

SOURCE=.\images.c
# End Source File
# Begin Source File

SOURCE=.\common\inout.c
# End Source File
# Begin Source File

SOURCE=.\jointed.c
# End Source File
# Begin Source File

SOURCE=.\common\l3dslib.c
# End Source File
# Begin Source File

SOURCE=.\common\lbmlib.c
# End Source File
# Begin Source File

SOURCE=.\common\mathlib.c
# End Source File
# Begin Source File

SOURCE=.\common\md4.c
# End Source File
# Begin Source File

SOURCE=.\models.c
# End Source File
# Begin Source File

SOURCE=.\common\path_init.c
# End Source File
# Begin Source File

SOURCE=.\pics.c
# End Source File
# Begin Source File

SOURCE=.\qd_skeletons.c
# End Source File
# Begin Source File

SOURCE=.\qdata.c
# End Source File
# Begin Source File

SOURCE=.\common\qfiles.c
# End Source File
# Begin Source File

SOURCE=.\qcommon\reference.c
# End Source File
# Begin Source File

SOURCE=.\qcommon\resourcemanager.c
# End Source File
# Begin Source File

SOURCE=.\common\scriplib.c
# End Source File
# Begin Source File

SOURCE=.\qcommon\skeletons.c
# End Source File
# Begin Source File

SOURCE=.\sprites.c
# End Source File
# Begin Source File

SOURCE=.\svdcmp.c
# End Source File
# Begin Source File

SOURCE=.\tables.c
# End Source File
# Begin Source File

SOURCE=.\common\threads.c
# End Source File
# Begin Source File

SOURCE=.\tmix.c
# End Source File
# Begin Source File

SOURCE=.\common\token.c
# End Source File
# Begin Source File

SOURCE=.\common\trilib.c
# End Source File
# Begin Source File

SOURCE=.\video.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\adpcm.h
# End Source File
# Begin Source File

SOURCE=.\animcomp.h
# End Source File
# Begin Source File

SOURCE=.\anorms.h
# End Source File
# Begin Source File

SOURCE=.\common\bspfile.h
# End Source File
# Begin Source File

SOURCE=.\common\cmdlib.h
# End Source File
# Begin Source File

SOURCE=.\common\her2_threads.h
# End Source File
# Begin Source File

SOURCE=.\common\inout.h
# End Source File
# Begin Source File

SOURCE=.\jointed.h
# End Source File
# Begin Source File

SOURCE=.\joints.h
# End Source File
# Begin Source File

SOURCE=.\common\l3dslib.h
# End Source File
# Begin Source File

SOURCE=.\common\lbmlib.h
# End Source File
# Begin Source File

SOURCE=.\common\mathlib.h
# End Source File
# Begin Source File

SOURCE=.\qd_fmodel.h
# End Source File
# Begin Source File

SOURCE=.\qd_skeletons.h
# End Source File
# Begin Source File

SOURCE=.\qdata.h
# End Source File
# Begin Source File

SOURCE=.\common\qfiles.h
# End Source File
# Begin Source File

SOURCE=.\qcommon\reference.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\qcommon\resourcemanager.h
# End Source File
# Begin Source File

SOURCE=.\common\scriplib.h
# End Source File
# Begin Source File

SOURCE=.\qcommon\skeletons.h
# End Source File
# Begin Source File

SOURCE=.\common\token.h
# End Source File
# Begin Source File

SOURCE=.\common\trilib.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
