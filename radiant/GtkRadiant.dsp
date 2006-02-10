# Microsoft Developer Studio Project File - Name="GtkRadiant" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=GtkRadiant - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GtkRadiant.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GtkRadiant.mak" CFG="GtkRadiant - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GtkRadiant - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "GtkRadiant - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GtkRadiant - Win32 Release"

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
# ADD F90 /browser /include:"Release/"
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MD /W3 /Zd /O2 /Op /Ob2 /I "..\..\STLport\stlport" /I "..\..\gtk2-win32\include\glib-2.0" /I "..\..\gtk2-win32\lib\glib-2.0\include" /I "..\..\gtk2-win32\lib\gtk-2.0\include" /I "..\..\gtk2-win32\include\gtk-2.0" /I "..\..\gtk2-win32\include\gtk-2.0\gdk" /I "..\..\gtk2-win32\include\pango-1.0" /I "..\..\gtk2-win32\include\atk-1.0" /I "..\..\gtk2-win32\include\gtkglext-1.0" /I "..\..\gtk2-win32\lib\gtkglext-1.0\include" /I "..\libs" /I "..\..\libxml2\include" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WIN32" /D "QUAKE3" /Fr /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 opengl32.lib kernel32.lib winspool.lib ole32.lib oleaut32.lib uuid.lib advapi32.lib comdlg32.lib user32.lib gdi32.lib shell32.lib Wsock32.lib glib-2.0.lib gobject-2.0.lib gdk-win32-2.0.lib gdk_pixbuf-2.0.lib gtk-win32-2.0.lib pango-1.0.lib gtkglext-win32-1.0.lib gdkglext-win32-1.0.lib jpeg6.lib l_net.lib cmdlib.lib mathlib.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /map /debug /machine:I386 /libpath:"../libs/mathlib/release" /libpath:"../libs/pak/release" /libpath:"../libs/cmdlib/release" /libpath:"../libs/jpeg6/release" /libpath:"../libs/l_net/release" /libpath:"..\..\gtk2-win32\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to dir...
PostBuild_Cmds=copy Release\GtkRadiant.exe "../install"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "GtkRadiant - Win32 Debug"

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
# ADD F90 /browser /include:"Debug/"
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "..\..\STLport\stlport" /I "..\..\gtk2-win32\include\glib-2.0" /I "..\..\gtk2-win32\lib\glib-2.0\include" /I "..\..\gtk2-win32\lib\gtk-2.0\include" /I "..\..\gtk2-win32\include\gtk-2.0" /I "..\..\gtk2-win32\include\gtk-2.0\gdk" /I "..\..\gtk2-win32\include\pango-1.0" /I "..\..\gtk2-win32\include\atk-1.0" /I "..\..\gtk2-win32\include\gtkglext-1.0" /I "..\..\gtk2-win32\lib\gtkglext-1.0\include" /I "..\libs" /I "..\..\libxml2\include" /I "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WIN32" /D "QUAKE3" /D "GNOME_DISABLE_DEPRECATED" /FR /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib advapi32.lib comdlg32.lib user32.lib gdi32.lib shell32.lib Wsock32.lib glib-2.0.lib gobject-2.0.lib gdk-win32-2.0.lib gtk-win32-2.0.lib gdk_pixbuf-2.0.lib gtkglext-win32-1.0.lib gdkglext-win32-1.0.lib pango-1.0.lib jpeg6.lib l_net.lib cmdlib.lib mathlib.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../libs/mathlib/debug" /libpath:"../libs/pak/debug" /libpath:"../libs/cmdlib/debug" /libpath:"../libs/jpeg6/debug" /libpath:"..\libs\l_net\debug" /libpath:"..\..\gtk2-win32\lib"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to dir...
PostBuild_Cmds=copy Debug\GtkRadiant.exe "../install"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "GtkRadiant - Win32 Release"
# Name "GtkRadiant - Win32 Debug"
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Group "Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\cullable.h
# End Source File
# Begin Source File

SOURCE=..\include\editable.h
# End Source File
# Begin Source File

SOURCE=..\include\iarchive.h
# End Source File
# Begin Source File

SOURCE=..\include\ibrush.h
# End Source File
# Begin Source File

SOURCE=..\include\ibspfrontend.h
# End Source File
# Begin Source File

SOURCE=..\include\icamera.h
# End Source File
# Begin Source File

SOURCE=..\include\icharstream.h
# End Source File
# Begin Source File

SOURCE=..\include\idatastream.h
# End Source File
# Begin Source File

SOURCE=..\include\ieclass.h
# End Source File
# Begin Source File

SOURCE=..\include\ientity.h
# End Source File
# Begin Source File

SOURCE=..\include\ifilesystem.h
# End Source File
# Begin Source File

SOURCE=..\include\ifiletypes.h
# End Source File
# Begin Source File

SOURCE=..\include\ifilter.h
# End Source File
# Begin Source File

SOURCE=..\include\igl.h
# End Source File
# Begin Source File

SOURCE=..\include\iimage.h
# End Source File
# Begin Source File

SOURCE=..\include\imap.h
# End Source File
# Begin Source File

SOURCE=..\include\imodel.h
# End Source File
# Begin Source File

SOURCE=..\include\ipatch.h
# End Source File
# Begin Source File

SOURCE=..\include\iplugin.h
# End Source File
# Begin Source File

SOURCE=..\include\irefcount.h
# End Source File
# Begin Source File

SOURCE=..\include\ireference.h
# End Source File
# Begin Source File

SOURCE=..\include\irender.h
# End Source File
# Begin Source File

SOURCE=..\include\iscenegraph.h
# End Source File
# Begin Source File

SOURCE=..\include\iscriplib.h
# End Source File
# Begin Source File

SOURCE=..\include\iselection.h
# End Source File
# Begin Source File

SOURCE=..\include\ishaders.h
# End Source File
# Begin Source File

SOURCE=..\include\itexdef.h
# End Source File
# Begin Source File

SOURCE=..\include\itextures.h
# End Source File
# Begin Source File

SOURCE=..\include\itoolbar.h
# End Source File
# Begin Source File

SOURCE=..\include\iUI.h
# End Source File
# Begin Source File

SOURCE=..\include\iUI_gtk.h
# End Source File
# Begin Source File

SOURCE=..\include\iundo.h
# End Source File
# Begin Source File

SOURCE=..\include\modules.h
# End Source File
# Begin Source File

SOURCE=..\include\nameable.h
# End Source File
# Begin Source File

SOURCE=..\include\qerplugin.h
# End Source File
# Begin Source File

SOURCE=..\include\qertypes.h
# End Source File
# Begin Source File

SOURCE=..\include\qsysprintf.h
# End Source File
# Begin Source File

SOURCE=..\include\renderable.h
# End Source File
# Begin Source File

SOURCE=..\include\selectable.h
# End Source File
# Begin Source File

SOURCE=..\include\window.h
# End Source File
# End Group
# Begin Group "libs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\libs\aabb.h
# End Source File
# Begin Source File

SOURCE=..\libs\aabblib.h
# End Source File
# Begin Source File

SOURCE=..\libs\cache.h
# End Source File
# Begin Source File

SOURCE=..\libs\chardatastream.h
# End Source File
# Begin Source File

SOURCE=..\libs\charstream.h
# End Source File
# Begin Source File

SOURCE=..\libs\cmdlib.h
# End Source File
# Begin Source File

SOURCE=..\libs\const_reference.h
# End Source File
# Begin Source File

SOURCE=..\libs\eclasslib.h
# End Source File
# Begin Source File

SOURCE=..\libs\entitylib.h
# End Source File
# Begin Source File

SOURCE=..\libs\entityxml.h
# End Source File
# Begin Source File

SOURCE=..\libs\expression.h
# End Source File
# Begin Source File

SOURCE=..\libs\filestream.h
# End Source File
# Begin Source File

SOURCE=..\libs\fs_filesystem.h
# End Source File
# Begin Source File

SOURCE=..\libs\fs_path.h
# End Source File
# Begin Source File

SOURCE=..\libs\function.h
# End Source File
# Begin Source File

SOURCE=..\libs\hashfunc.h
# End Source File
# Begin Source File

SOURCE=..\libs\hashtable.h
# End Source File
# Begin Source File

SOURCE=..\libs\instancelib.h
# End Source File
# Begin Source File

SOURCE=..\libs\jpeg6\Jconfig.h
# End Source File
# Begin Source File

SOURCE=..\libs\jpeg6\Jmorecfg.h
# End Source File
# Begin Source File

SOURCE=..\libs\jpeglib.h
# End Source File
# Begin Source File

SOURCE=..\libs\l_net\l_net.h
# End Source File
# Begin Source File

SOURCE=..\libs\maplib.h
# End Source File
# Begin Source File

SOURCE=..\LIBS\mathlib.h
# End Source File
# Begin Source File

SOURCE=..\libs\matrixlib.h
# End Source File
# Begin Source File

SOURCE=..\libs\memstream.h
# End Source File
# Begin Source File

SOURCE=..\libs\missing.h
# End Source File
# Begin Source File

SOURCE=..\libs\modulesmap.h
# End Source File
# Begin Source File

SOURCE=..\libs\picomodel.h
# End Source File
# Begin Source File

SOURCE=..\libs\radiant_assert.h
# End Source File
# Begin Source File

SOURCE=..\libs\refcounted_ptr.h
# End Source File
# Begin Source File

SOURCE=..\libs\render.h
# End Source File
# Begin Source File

SOURCE=..\libs\scenelib.h
# End Source File
# Begin Source File

SOURCE=..\libs\selectionlib.h
# End Source File
# Begin Source File

SOURCE=..\libs\shaderlib.h
# End Source File
# Begin Source File

SOURCE=..\libs\singletonmodule.h
# End Source File
# Begin Source File

SOURCE=..\libs\str.h
# End Source File
# Begin Source File

SOURCE=..\libs\stringio.h
# End Source File
# Begin Source File

SOURCE=..\libs\stringlib.h
# End Source File
# Begin Source File

SOURCE=..\libs\stringstream.h
# End Source File
# Begin Source File

SOURCE=..\libs\textfilestream.h
# End Source File
# Begin Source File

SOURCE=..\libs\texturelib.h
# End Source File
# Begin Source File

SOURCE=..\libs\transformlib.h
# End Source File
# Begin Source File

SOURCE=..\libs\traverselib.h
# End Source File
# Begin Source File

SOURCE=..\libs\undolib.h
# End Source File
# Begin Source File

SOURCE=..\libs\xml.h
# End Source File
# Begin Source File

SOURCE=..\libs\xmlio.h
# End Source File
# Begin Source File

SOURCE=..\libs\xmlparser.h
# End Source File
# Begin Source File

SOURCE=..\libs\xmlwriter.h
# End Source File
# End Group
# Begin Group "stl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\gtkr_list.h
# End Source File
# Begin Source File

SOURCE=..\include\gtkr_vector.h
# End Source File
# Begin Source File

SOURCE=..\include\stl_check.h
# End Source File
# Begin Source File

SOURCE=..\include\stl_warnings.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\bp_dlg.h
# End Source File
# Begin Source File

SOURCE=.\brush.h
# End Source File
# Begin Source File

SOURCE=.\brush_primit.h
# End Source File
# Begin Source File

SOURCE=.\brushwrapper.h
# End Source File
# Begin Source File

SOURCE=.\camwindow.h
# End Source File
# Begin Source File

SOURCE=.\csg.h
# End Source File
# Begin Source File

SOURCE=.\dialog.h
# End Source File
# Begin Source File

SOURCE=.\eclass.h
# End Source File
# Begin Source File

SOURCE=.\eclass_def.h
# End Source File
# Begin Source File

SOURCE=.\entityinspector.h
# End Source File
# Begin Source File

SOURCE=.\error.h
# End Source File
# Begin Source File

SOURCE=.\feedback.h
# End Source File
# Begin Source File

SOURCE=.\file.h
# End Source File
# Begin Source File

SOURCE=.\filetypes.h
# End Source File
# Begin Source File

SOURCE=.\filters.h
# End Source File
# Begin Source File

SOURCE=.\findtexturedialog.h
# End Source File
# Begin Source File

SOURCE=.\glinterface.h
# End Source File
# Begin Source File

SOURCE=.\glwidget.h
# End Source File
# Begin Source File

SOURCE=.\glwindow.h
# End Source File
# Begin Source File

SOURCE=.\groupdialog.h
# End Source File
# Begin Source File

SOURCE=.\gtkdlgs.h
# End Source File
# Begin Source File

SOURCE=.\gtkmisc.h
# End Source File
# Begin Source File

SOURCE=.\image.h
# End Source File
# Begin Source File

SOURCE=.\main.h
# End Source File
# Begin Source File

SOURCE=.\mainframe.h
# End Source File
# Begin Source File

SOURCE=.\map.h
# End Source File
# Begin Source File

SOURCE=.\multimon.h
# End Source File
# Begin Source File

SOURCE=.\nullmodel.h
# End Source File
# Begin Source File

SOURCE=.\parse.h
# End Source File
# Begin Source File

SOURCE=.\patch.h
# End Source File
# Begin Source File

SOURCE=.\patchdialog.h
# End Source File
# Begin Source File

SOURCE=.\patchmanip.h
# End Source File
# Begin Source File

SOURCE=.\plugin.h
# End Source File
# Begin Source File

SOURCE=.\pluginapi.h
# End Source File
# Begin Source File

SOURCE=.\pluginmanager.h
# End Source File
# Begin Source File

SOURCE=.\points.h
# End Source File
# Begin Source File

SOURCE=.\preferencedictionary.h
# End Source File
# Begin Source File

SOURCE=.\preferences.h
# End Source File
# Begin Source File

SOURCE=.\preferencesystem.h
# End Source File
# Begin Source File

SOURCE=.\profile.h
# End Source File
# Begin Source File

SOURCE=.\qe3.h
# End Source File
# Begin Source File

SOURCE=.\qfiles.h
# End Source File
# Begin Source File

SOURCE=.\qgl.h
# End Source File
# Begin Source File

SOURCE=.\referencecache.h
# End Source File
# Begin Source File

SOURCE=.\renderer.h
# End Source File
# Begin Source File

SOURCE=.\renderstate.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph.h
# End Source File
# Begin Source File

SOURCE=.\select.h
# End Source File
# Begin Source File

SOURCE=.\selection.h
# End Source File
# Begin Source File

SOURCE=.\selectionmanager.h
# End Source File
# Begin Source File

SOURCE=.\server.h
# End Source File
# Begin Source File

SOURCE=.\shaderinfo.h
# End Source File
# Begin Source File

SOURCE=.\surfacedialog.h
# End Source File
# Begin Source File

SOURCE=.\targetname.h
# End Source File
# Begin Source File

SOURCE=.\texmanip.h
# End Source File
# Begin Source File

SOURCE=.\textures.h
# End Source File
# Begin Source File

SOURCE=.\texwindow.h
# End Source File
# Begin Source File

SOURCE=.\timer.h
# End Source File
# Begin Source File

SOURCE=.\treemodel.h
# End Source File
# Begin Source File

SOURCE=.\ui.h
# End Source File
# Begin Source File

SOURCE=.\undo.h
# End Source File
# Begin Source File

SOURCE=..\include\version.h
# End Source File
# Begin Source File

SOURCE=.\view.h
# End Source File
# Begin Source File

SOURCE=.\watchbsp.h
# End Source File
# Begin Source File

SOURCE=.\winding.h
# End Source File
# Begin Source File

SOURCE=.\xmlstuff.h
# End Source File
# Begin Source File

SOURCE=.\xorrectangle.h
# End Source File
# Begin Source File

SOURCE=.\xywindow.h
# End Source File
# Begin Source File

SOURCE=.\z.h
# End Source File
# Begin Source File

SOURCE=.\zwindow.h
# End Source File
# End Group
# Begin Group "Code"

# PROP Default_Filter ""
# Begin Group "BSP monitoring"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\feedback.cpp
# End Source File
# Begin Source File

SOURCE=.\watchbsp.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\bp_dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\brush.cpp
# End Source File
# Begin Source File

SOURCE=.\brush_primit.cpp
# End Source File
# Begin Source File

SOURCE=.\brushwrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\camwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\csg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\eclass.cpp
# End Source File
# Begin Source File

SOURCE=.\eclass_def.cpp
# End Source File
# Begin Source File

SOURCE=.\entityinspector.cpp
# End Source File
# Begin Source File

SOURCE=.\error.cpp
# End Source File
# Begin Source File

SOURCE=.\expression.cpp
# End Source File
# Begin Source File

SOURCE=.\file.cpp
# End Source File
# Begin Source File

SOURCE=.\filetypes.cpp
# End Source File
# Begin Source File

SOURCE=.\filters.cpp
# End Source File
# Begin Source File

SOURCE=.\findtexturedialog.cpp
# End Source File
# Begin Source File

SOURCE=.\function.cpp
# End Source File
# Begin Source File

SOURCE=.\glinterface.cpp
# End Source File
# Begin Source File

SOURCE=.\glwidget.cpp
# End Source File
# Begin Source File

SOURCE=.\glwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\groupdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\gtkdlgs.cpp
# End Source File
# Begin Source File

SOURCE=.\gtkmisc.cpp
# End Source File
# Begin Source File

SOURCE=.\image.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\mainframe.cpp
# End Source File
# Begin Source File

SOURCE=.\map.cpp
# End Source File
# Begin Source File

SOURCE=.\multimon.cpp
# End Source File
# Begin Source File

SOURCE=.\nullmodel.cpp
# End Source File
# Begin Source File

SOURCE=.\parse.cpp
# End Source File
# Begin Source File

SOURCE=.\patch.cpp
# End Source File
# Begin Source File

SOURCE=.\patchdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\patchmanip.cpp
# End Source File
# Begin Source File

SOURCE=.\plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\pluginapi.cpp
# End Source File
# Begin Source File

SOURCE=.\pluginentities.cpp
# End Source File
# Begin Source File

SOURCE=.\pluginmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\POINTS.cpp
# End Source File
# Begin Source File

SOURCE=.\preferences.cpp
# End Source File
# Begin Source File

SOURCE=.\PROFILE.cpp
# End Source File
# Begin Source File

SOURCE=.\qe3.cpp
# End Source File
# Begin Source File

SOURCE=.\qgl.cpp
# End Source File
# Begin Source File

SOURCE=.\qgl_ext.cpp
# End Source File
# Begin Source File

SOURCE=.\referencecache.cpp
# End Source File
# Begin Source File

SOURCE=.\renderstate.cpp
# End Source File
# Begin Source File

SOURCE=.\scenegraph.cpp
# End Source File
# Begin Source File

SOURCE=.\select.cpp
# End Source File
# Begin Source File

SOURCE=.\selection.cpp
# End Source File
# Begin Source File

SOURCE=.\server.cpp

!IF  "$(CFG)" == "GtkRadiant - Win32 Release"

# ADD CPP /Ob2

!ELSEIF  "$(CFG)" == "GtkRadiant - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\surfacedialog.cpp
# End Source File
# Begin Source File

SOURCE=.\targetname.cpp
# End Source File
# Begin Source File

SOURCE=.\texmanip.cpp
# End Source File
# Begin Source File

SOURCE=.\textures.cpp
# End Source File
# Begin Source File

SOURCE=.\texwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\timer.cpp
# End Source File
# Begin Source File

SOURCE=.\treemodel.cpp
# End Source File
# Begin Source File

SOURCE=.\ui.cpp
# End Source File
# Begin Source File

SOURCE=.\undo.cpp
# End Source File
# Begin Source File

SOURCE=.\view.cpp
# End Source File
# Begin Source File

SOURCE=.\winding.cpp
# End Source File
# Begin Source File

SOURCE=.\write.cpp
# End Source File
# Begin Source File

SOURCE=.\xywindow.cpp
# End Source File
# Begin Source File

SOURCE=.\z.cpp
# End Source File
# Begin Source File

SOURCE=.\zwindow.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\setup\changelog.txt
# End Source File
# Begin Source File

SOURCE=..\docs\developer\CHANGES
# End Source File
# Begin Source File

SOURCE=.\radiant.ico
# End Source File
# Begin Source File

SOURCE=.\radiant.rc
# End Source File
# Begin Source File

SOURCE=..\SConscript
# End Source File
# End Target
# End Project
