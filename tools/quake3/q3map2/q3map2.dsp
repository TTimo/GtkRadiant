# Microsoft Developer Studio Project File - Name="q3map2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Q3MAP2 - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "q3map2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "q3map2.mak" CFG="Q3MAP2 - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "q3map2 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "q3map2 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "q3map2"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "q3map2 - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O2 /I "..\..\..\include" /I "..\common" /I "..\..\..\libs" /I "..\..\..\..\libxml2\include" /I "..\q3map2" /I "..\..\..\..\libpng" /I "..\..\..\..\zlib" /I "..\..\..\..\gtk2-win32\include\glib-2.0" /I "..\..\..\..\gtk2-win32\lib\glib-2.0\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /WX /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 glib-2.0.lib wsock32.lib jpeg6.lib l_net.lib mathlib.lib md5lib.lib zlib.lib libpng12.lib picomodel.lib ddslib.lib /nologo /stack:0x400000 /subsystem:console /map /machine:I386 /libpath:"..\..\..\libs\ddslib\release" /libpath:"..\..\..\libs\md5lib\release" /libpath:"..\..\..\libs\mathlib\release" /libpath:"..\..\..\libs\pak\release" /libpath:"..\..\..\libs\jpeg6\release" /libpath:"..\..\..\libs\l_net\release" /libpath:"..\..\..\..\libxml2\win32\libxml2\release_so" /libpath:"..\..\..\..\libpng\projects\msvc\libpng___Win32_Release" /libpath:"..\..\..\..\libpng\projects\msvc\zlib___Win32_Release" /libpath:"../../../libs/picomodel/release" /libpath:"..\..\..\..\gtk2-win32\lib\\"
# SUBTRACT LINK32 /pdb:none /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Python post build
PostBuild_Cmds=cd ..\..\.. && run_python.bat win32_install.py release Q3Map2
# End Special Build Tool

!ELSEIF  "$(CFG)" == "q3map2 - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "..\..\..\include" /I "..\common" /I "..\..\..\libs" /I "..\..\..\..\libxml2\include" /I "..\..\..\..\libpng" /I "..\..\..\..\zlib" /I "..\..\..\..\gtk2-win32\include\glib-2.0" /I "..\..\..\..\gtk2-win32\lib\glib-2.0\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 glib-2.0.lib wsock32.lib jpeg6.lib l_net.lib mathlib.lib md5lib.lib zlibd.lib libpng12d.lib picomodel.lib ddslib.lib /nologo /stack:0x400000 /subsystem:console /profile /map /debug /machine:I386 /nodefaultlib:"libcd" /libpath:"..\..\..\libs\ddslib\Debug" /libpath:"..\..\..\libs\md5lib\Debug" /libpath:"..\..\..\libs\mathlib\Debug" /libpath:"..\..\..\libs\pak\Debug" /libpath:"..\..\..\libs\jpeg6\Debug" /libpath:"..\..\..\libs\l_net\Debug" /libpath:"..\..\..\..\libxml2\win32\libxml2\debug_so" /libpath:"..\..\..\..\lpng\projects\msvc\win32\libpng___Win32_Debug" /libpath:"..\..\..\libs\jpeg6\release" /libpath:"..\..\..\libs\l_net\release" /libpath:"..\..\..\..\libxml2\win32\libxml2\release_so" /libpath:"..\..\..\..\libpng\projects\msvc\libpng___Win32_Debug" /libpath:"..\..\..\..\libpng\projects\msvc\zlib___Win32_Debug" /libpath:"../../../libs/picomodel/debug" /libpath:"..\..\..\..\gtk2-win32\lib\\"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Python post build
PostBuild_Cmds=cd ..\..\.. && run_python.bat win32_install.py Q3Map2
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "q3map2 - Win32 Release"
# Name "q3map2 - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter "c;cpp;cxx;cc;C"
# Begin Group "common"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\common\cmdlib.c
# End Source File
# Begin Source File

SOURCE=..\common\imagelib.c
# End Source File
# Begin Source File

SOURCE=..\common\inout.c
# End Source File
# Begin Source File

SOURCE=..\common\mutex.c
# End Source File
# Begin Source File

SOURCE=..\common\polylib.c
# End Source File
# Begin Source File

SOURCE=..\common\scriplib.c
# End Source File
# Begin Source File

SOURCE=..\common\threads.c
# End Source File
# Begin Source File

SOURCE=..\common\unzip.c
# End Source File
# Begin Source File

SOURCE=..\common\vfs.c

!IF  "$(CFG)" == "q3map2 - Win32 Release"

!ELSEIF  "$(CFG)" == "q3map2 - Win32 Debug"

# ADD CPP /I "..\..\..\..\src\glib"

!ENDIF 

# End Source File
# End Group
# Begin Group "bsp"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=.\brush.c
# End Source File
# Begin Source File

SOURCE=.\brush_primit.c
# End Source File
# Begin Source File

SOURCE=.\bsp.c
# End Source File
# Begin Source File

SOURCE=.\decals.c
# End Source File
# Begin Source File

SOURCE=.\facebsp.c
# End Source File
# Begin Source File

SOURCE=.\fog.c
# End Source File
# Begin Source File

SOURCE=.\leakfile.c
# End Source File
# Begin Source File

SOURCE=.\map.c
# End Source File
# Begin Source File

SOURCE=.\patch.c
# End Source File
# Begin Source File

SOURCE=.\portals.c
# End Source File
# Begin Source File

SOURCE=.\prtfile.c
# End Source File
# Begin Source File

SOURCE=.\surface.c
# End Source File
# Begin Source File

SOURCE=.\surface_foliage.c
# End Source File
# Begin Source File

SOURCE=.\surface_fur.c
# End Source File
# Begin Source File

SOURCE=.\surface_meta.c
# End Source File
# Begin Source File

SOURCE=.\tjunction.c
# End Source File
# Begin Source File

SOURCE=.\tree.c
# End Source File
# Begin Source File

SOURCE=.\writebsp.c
# End Source File
# End Group
# Begin Group "light"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=.\light.c
# End Source File
# Begin Source File

SOURCE=.\light_bounce.c
# End Source File
# Begin Source File

SOURCE=.\light_trace.c
# End Source File
# Begin Source File

SOURCE=.\light_ydnar.c
# End Source File
# Begin Source File

SOURCE=.\lightmaps_ydnar.c
# End Source File
# End Group
# Begin Group "vis"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=.\vis.c
# End Source File
# Begin Source File

SOURCE=.\visflow.c
# End Source File
# End Group
# Begin Group "convert"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=.\convert_ase.c
# End Source File
# Begin Source File

SOURCE=.\convert_map.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\bspfile_abstract.c
# End Source File
# Begin Source File

SOURCE=.\bspfile_ibsp.c
# End Source File
# Begin Source File

SOURCE=.\bspfile_rbsp.c
# End Source File
# Begin Source File

SOURCE=.\image.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\mesh.c
# End Source File
# Begin Source File

SOURCE=.\model.c
# End Source File
# Begin Source File

SOURCE=.\path_init.c
# End Source File
# Begin Source File

SOURCE=.\shaders.c
# End Source File
# Begin Source File

SOURCE=.\surface_extra.c
# End Source File
# End Group
# Begin Group "include"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\game_ef.h
# End Source File
# Begin Source File

SOURCE=.\game_ja.h
# End Source File
# Begin Source File

SOURCE=.\game_jk2.h
# End Source File
# Begin Source File

SOURCE=.\game_quake3.h
# End Source File
# Begin Source File

SOURCE=.\game_sof2.h
# End Source File
# Begin Source File

SOURCE=.\game_wolf.h
# End Source File
# Begin Source File

SOURCE=.\game_wolfet.h
# End Source File
# Begin Source File

SOURCE=.\q3map2.h
# End Source File
# End Group
# Begin Group "doc"

# PROP Default_Filter "*.txt"
# Begin Source File

SOURCE=.\changelog.q3map2.txt
# End Source File
# End Group
# Begin Group "rc"

# PROP Default_Filter ".rc;.ico"
# Begin Source File

SOURCE=.\q3map2.ico
# End Source File
# Begin Source File

SOURCE=.\q3map2.rc
# End Source File
# End Group
# End Target
# End Project
