# Microsoft Developer Studio Project File - Name="bobToolz_gtk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=bobToolz_gtk - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bobToolz_gtk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bobToolz_gtk.mak" CFG="bobToolz_gtk - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bobToolz_gtk - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "bobToolz_gtk - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "bobToolz_gtk"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bobToolz_gtk - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /include:"Release/"
# ADD F90 /include:"Release-GTK/"
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BOBTOOLZ_GTK_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /O2 /I "../../libs" /I "../../../STLport/stlport" /I "../../include" /I "..\..\..\gtk2-win32\include\glib-2.0" /I "..\..\..\gtk2-win32\lib\glib-2.0\include" /I "..\..\..\gtk2-win32\lib\gtk-2.0\include" /I "..\..\..\gtk2-win32\include\gtk-2.0" /I "..\..\..\gtk2-win32\include\gtk-2.0\gdk" /I "..\..\..\gtk2-win32\include\pango-1.0" /I "..\..\..\gtk2-win32\include\atk-1.0" /I "dialogs" /I "..\..\..\libxml2\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BOBTOOLZ_GTK_EXPORTS" /D "_BOBTOOLZGTK_" /D "_GTK_" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG" /d "_BOBTOOLZGTK_"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 glib-2.0.lib gobject-2.0.lib gdk-win32-2.0.lib gtk-win32-2.0.lib pango-1.0.lib mathlib.lib synapse.lib /nologo /dll /machine:I386 /def:".\bobToolz.def" /out:"Release/bobToolz.dll" /libpath:"..\src\glib" /libpath:"..\src\gtk+\gtk" /libpath:"..\src\gtk+\gdk" /libpath:"../../../src/glib" /libpath:"../../../src/gtk+/gtk" /libpath:"../../../src/gtk+/gdk" /libpath:"..\..\libs\mathlib\Release" /libpath:"../../libs/synapse/release" /libpath:"..\..\..\gtk2-win32\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "bobToolz_gtk - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /include:"Debug/"
# ADD F90 /include:"Debug-GTK/"
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BOBTOOLZ_GTK_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "../../libs" /I "../../../STLport/stlport" /I "../../include" /I "..\..\..\gtk2-win32\include\glib-2.0" /I "..\..\..\gtk2-win32\lib\glib-2.0\include" /I "..\..\..\gtk2-win32\lib\gtk-2.0\include" /I "..\..\..\gtk2-win32\include\gtk-2.0" /I "..\..\..\gtk2-win32\include\gtk-2.0\gdk" /I "..\..\..\gtk2-win32\include\pango-1.0" /I "..\..\..\gtk2-win32\include\atk-1.0" /I "dialogs" /I "..\..\..\libxml2\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BOBTOOLZ_GTK_EXPORTS" /D "_BOBTOOLZGTK_" /D "_GTK_" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG" /d "_BOBTOOLZGTK_"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 glib-2.0.lib gobject-2.0.lib gdk-win32-2.0.lib gtk-win32-2.0.lib pango-1.0.lib mathlib.lib synapse.lib /nologo /dll /debug /machine:I386 /def:".\bobToolz.def" /out:"Debug/bobToolz.dll" /pdbtype:sept /libpath:"../../../src/glib" /libpath:"../../../src/gtk+/gtk" /libpath:"../../../src/gtk+/gdk" /libpath:"..\..\libs\mathlib\Debug" /libpath:"../../libs/synapse/debug" /libpath:"..\..\..\gtk2-win32\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "bobToolz_gtk - Win32 Release"
# Name "bobToolz_gtk - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\dialogs\dialogs-gtk.cpp"
# End Source File
# End Group
# Begin Source File

SOURCE=".\bobToolz-GTK.cpp"
# End Source File
# Begin Source File

SOURCE=".\bobtoolz-gtk.rc"
# End Source File
# Begin Source File

SOURCE=.\bobToolz.def
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\bsploader.cpp
# End Source File
# Begin Source File

SOURCE=.\cportals.cpp
# End Source File
# Begin Source File

SOURCE=.\DBobView.cpp
# End Source File
# Begin Source File

SOURCE=.\DBrush.cpp
# End Source File
# Begin Source File

SOURCE=.\DEntity.cpp
# End Source File
# Begin Source File

SOURCE=.\DEPair.cpp
# End Source File
# Begin Source File

SOURCE=.\DListener.cpp
# End Source File
# Begin Source File

SOURCE=.\DMap.cpp
# End Source File
# Begin Source File

SOURCE=.\DPatch.cpp
# End Source File
# Begin Source File

SOURCE=.\DPlane.cpp
# End Source File
# Begin Source File

SOURCE=.\DPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\DShape.cpp
# End Source File
# Begin Source File

SOURCE=.\DTrainDrawer.cpp
# End Source File
# Begin Source File

SOURCE=.\DTreePlanter.cpp
# End Source File
# Begin Source File

SOURCE=.\DVisDrawer.cpp
# End Source File
# Begin Source File

SOURCE=.\DWinding.cpp
# End Source File
# Begin Source File

SOURCE=".\funchandlers-GTK.cpp"
# End Source File
# Begin Source File

SOURCE=.\lists.cpp
# End Source File
# Begin Source File

SOURCE=.\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptParser.cpp
# End Source File
# Begin Source File

SOURCE=.\shapes.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\visfind.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "dialog - headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\dialogs\dialogs-gtk.h"
# End Source File
# End Group
# Begin Group "interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\iUI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\qerplugin.h
# End Source File
# Begin Source File

SOURCE=..\..\include\qertypes.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\bsploader.h
# End Source File
# Begin Source File

SOURCE=.\CPortals.h
# End Source File
# Begin Source File

SOURCE=.\DBobView.h
# End Source File
# Begin Source File

SOURCE=.\DBrush.h
# End Source File
# Begin Source File

SOURCE=.\DEntity.h
# End Source File
# Begin Source File

SOURCE=.\DEPair.h
# End Source File
# Begin Source File

SOURCE=.\DListener.h
# End Source File
# Begin Source File

SOURCE=.\DMap.h
# End Source File
# Begin Source File

SOURCE=.\DPatch.h
# End Source File
# Begin Source File

SOURCE=.\DPlane.h
# End Source File
# Begin Source File

SOURCE=.\DPoint.h
# End Source File
# Begin Source File

SOURCE=.\DShape.h
# End Source File
# Begin Source File

SOURCE=.\DTrainDrawer.h
# End Source File
# Begin Source File

SOURCE=.\DTreePlanter.h
# End Source File
# Begin Source File

SOURCE=.\DVisDrawer.h
# End Source File
# Begin Source File

SOURCE=.\DWinding.h
# End Source File
# Begin Source File

SOURCE=.\funchandlers.h
# End Source File
# Begin Source File

SOURCE=.\lists.h
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=.\res\plugin.rc2
# End Source File
# Begin Source File

SOURCE=".\resource-GTK.h"
# End Source File
# Begin Source File

SOURCE=.\shapes.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\visfind.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
