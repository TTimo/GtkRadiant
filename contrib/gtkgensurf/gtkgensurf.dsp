# Microsoft Developer Studio Project File - Name="gtkgensurf" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=gtkgensurf - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gtkgensurf.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gtkgensurf.mak" CFG="gtkgensurf - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gtkgensurf - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gtkgensurf - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "gtkgensurf"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gtkgensurf - Win32 Release"

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
# ADD BASE F90 /include:"Release/"
# ADD F90 /include:"Release/"
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GTKGENSURF_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /O2 /I "..\..\libs" /I "..\..\include" /I "..\..\..\gtk2-win32\include\glib-2.0" /I "..\..\..\gtk2-win32\lib\glib-2.0\include" /I "..\..\..\gtk2-win32\lib\gtk-2.0\include" /I "..\..\..\gtk2-win32\include\gtk-2.0" /I "..\..\..\gtk2-win32\include\gtk-2.0\gdk" /I "..\..\..\gtk2-win32\include\pango-1.0" /I "..\..\..\gtk2-win32\include\atk-1.0" /I "..\..\..\stlport\stlport" /I "..\..\..\libxml2\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GTKGENSURF_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 glib-2.0.lib gobject-2.0.lib gdk-win32-2.0.lib gtk-win32-2.0.lib pango-1.0.lib /nologo /dll /machine:I386 /def:".\gensurf.def" /out:"Release/gensurf.dll" /libpath:"..\..\..\src\glib" /libpath:"..\..\..\src\gtk+\gtk" /libpath:"..\..\..\src\gtk+\gdk" /libpath:"../../libs/synapse/debug" /libpath:"..\..\..\gtk2-win32\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "gtkgensurf - Win32 Debug"

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
# ADD BASE F90 /include:"Debug/"
# ADD F90 /include:"Debug/"
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GTKGENSURF_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "..\..\libs" /I "..\..\include" /I "..\..\..\gtk2-win32\include\glib-2.0" /I "..\..\..\gtk2-win32\lib\glib-2.0\include" /I "..\..\..\gtk2-win32\lib\gtk-2.0\include" /I "..\..\..\gtk2-win32\include\gtk-2.0" /I "..\..\..\gtk2-win32\include\gtk-2.0\gdk" /I "..\..\..\gtk2-win32\include\pango-1.0" /I "..\..\..\gtk2-win32\include\atk-1.0" /I "..\..\..\stlport\stlport" /I "..\..\..\libxml2\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GTKGENSURF_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 glib-2.0.lib gobject-2.0.lib gdk-win32-2.0.lib gtk-win32-2.0.lib pango-1.0.lib /nologo /dll /debug /machine:I386 /def:".\gensurf.def" /out:"Debug/gensurf.dll" /pdbtype:sept /libpath:"..\..\..\src\glib" /libpath:"..\..\..\src\gtk+\gtk" /libpath:"..\..\..\src\gtk+\gdk" /libpath:"../../libs/synapse/debug" /libpath:"..\..\..\gtk2-win32\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "gtkgensurf - Win32 Release"
# Name "gtkgensurf - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\dec.cpp
# End Source File
# Begin Source File

SOURCE=.\face.cpp
# End Source File
# Begin Source File

SOURCE=.\font.cpp
# End Source File
# Begin Source File

SOURCE=.\gendlgs.cpp
# End Source File
# Begin Source File

SOURCE=.\genmap.cpp
# End Source File
# Begin Source File

SOURCE=.\gensurf.cpp
# End Source File
# Begin Source File

SOURCE=.\gensurf.def
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\heretic.cpp
# End Source File
# Begin Source File

SOURCE=.\plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\triangle.c
# End Source File
# Begin Source File

SOURCE=.\view.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\gendlgs.h
# End Source File
# Begin Source File

SOURCE=.\gensurf.h
# End Source File
# Begin Source File

SOURCE=.\triangle.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
