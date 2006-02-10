# Microsoft Developer Studio Project File - Name="PrtView" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PrtView - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PrtView.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PrtView.mak" CFG="PrtView - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PrtView - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PrtView - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "PrtView"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PrtView - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /include:"Release/"
# ADD F90 /include:"Release/"
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /O2 /I "..\..\..\STLport\stlport" /I "..\..\..\libxml2\include" /I "..\..\libs" /I "..\..\include" /I "..\..\..\gtk2-win32\include\glib-2.0" /I "..\..\..\gtk2-win32\lib\glib-2.0\include" /I "..\..\..\gtk2-win32\lib\gtk-2.0\include" /I "..\..\..\gtk2-win32\include\gtk-2.0" /I "..\..\..\gtk2-win32\include\gtk-2.0\gdk" /I "..\..\..\gtk2-win32\include\pango-1.0" /I "..\..\..\gtk2-win32\include\atk-1.0" /D "NDEBUG" /D "GTK_PLUGIN" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 glib-2.0.lib gobject-2.0.lib gdk-win32-2.0.lib gtk-win32-2.0.lib pango-1.0.lib /nologo /subsystem:windows /dll /machine:I386 /def:".\PrtView.def" /libpath:"..\..\..\gtk2-win32\lib"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to dir...
PostBuild_Cmds=copy Release\PrtView.dll "..\..\install\plugins"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PrtView - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /include:"Debug/"
# ADD F90 /browser /include:"Debug/"
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /ZI /Od /I "..\..\..\STLport\stlport" /I "..\..\..\libxml2\include" /I "..\..\libs" /I "..\..\include" /I "..\..\..\gtk2-win32\include\glib-2.0" /I "..\..\..\gtk2-win32\lib\glib-2.0\include" /I "..\..\..\gtk2-win32\lib\gtk-2.0\include" /I "..\..\..\gtk2-win32\include\gtk-2.0" /I "..\..\..\gtk2-win32\include\gtk-2.0\gdk" /I "..\..\..\gtk2-win32\include\pango-1.0" /I "..\..\..\gtk2-win32\include\atk-1.0" /D "_DEBUG" /D "GTK_PLUGIN" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 glib-2.0.lib gobject-2.0.lib gdk-win32-2.0.lib gtk-win32-2.0.lib pango-1.0.lib /nologo /subsystem:windows /dll /debug /machine:I386 /def:".\PrtView.def" /pdbtype:sept /libpath:"..\..\..\gtk2-win32\lib"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to dir...
PostBuild_Cmds=copy Debug\PrtView.dll "..\..\install\plugins"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PrtView - Win32 Release"
# Name "PrtView - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AboutDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadPortalFileDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Portals.cpp
# End Source File
# Begin Source File

SOURCE=.\PrtView.cpp
# End Source File
# Begin Source File

SOURCE=.\PrtView.def
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\PrtView.rc
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AboutDialog.h
# End Source File
# Begin Source File

SOURCE=.\ConfigDialog.h
# End Source File
# Begin Source File

SOURCE=.\LoadPortalFileDialog.h
# End Source File
# Begin Source File

SOURCE=.\Portals.h
# End Source File
# Begin Source File

SOURCE=.\PrtView.h
# End Source File
# Begin Source File

SOURCE=.\PrtView2.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\PrtView.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
