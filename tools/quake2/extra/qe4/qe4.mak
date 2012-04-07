# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Win32 (ALPHA) Application" 0x0601

!IF "$(CFG)" == ""
CFG=qe3 - Win32 (ALPHA) Debug
!MESSAGE No configuration specified.  Defaulting to qe3 - Win32 (ALPHA) Debug.
!ENDIF 

!IF "$(CFG)" != "qe3 - Win32 Release" && "$(CFG)" != "qe3 - Win32 Debug" &&\
 "$(CFG)" != "qe3 - Win32 (ALPHA) Debug" && "$(CFG)" !=\
 "qe3 - Win32 (ALPHA) Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "qe4.mak" CFG="qe3 - Win32 (ALPHA) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qe3 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "qe3 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "qe3 - Win32 (ALPHA) Debug" (based on "Win32 (ALPHA) Application")
!MESSAGE "qe3 - Win32 (ALPHA) Release" (based on "Win32 (ALPHA) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "qe3 - Win32 Debug"

!IF  "$(CFG)" == "qe3 - Win32 Release"

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
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\qe4.exe" "$(OUTDIR)\qe4.bsc"

CLEAN : 
	-@erase "$(INTDIR)\brush.obj"
	-@erase "$(INTDIR)\brush.sbr"
	-@erase "$(INTDIR)\camera.obj"
	-@erase "$(INTDIR)\camera.sbr"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\cmdlib.sbr"
	-@erase "$(INTDIR)\csg.obj"
	-@erase "$(INTDIR)\csg.sbr"
	-@erase "$(INTDIR)\drag.obj"
	-@erase "$(INTDIR)\drag.sbr"
	-@erase "$(INTDIR)\eclass.obj"
	-@erase "$(INTDIR)\eclass.sbr"
	-@erase "$(INTDIR)\entity.obj"
	-@erase "$(INTDIR)\entity.sbr"
	-@erase "$(INTDIR)\lbmlib.obj"
	-@erase "$(INTDIR)\lbmlib.sbr"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\map.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\mru.obj"
	-@erase "$(INTDIR)\mru.sbr"
	-@erase "$(INTDIR)\parse.obj"
	-@erase "$(INTDIR)\parse.sbr"
	-@erase "$(INTDIR)\points.obj"
	-@erase "$(INTDIR)\points.sbr"
	-@erase "$(INTDIR)\qe3.obj"
	-@erase "$(INTDIR)\qe3.sbr"
	-@erase "$(INTDIR)\select.obj"
	-@erase "$(INTDIR)\select.sbr"
	-@erase "$(INTDIR)\textures.obj"
	-@erase "$(INTDIR)\textures.sbr"
	-@erase "$(INTDIR)\vertsel.obj"
	-@erase "$(INTDIR)\vertsel.sbr"
	-@erase "$(INTDIR)\win_cam.obj"
	-@erase "$(INTDIR)\win_cam.sbr"
	-@erase "$(INTDIR)\win_dlg.obj"
	-@erase "$(INTDIR)\win_dlg.sbr"
	-@erase "$(INTDIR)\win_ent.obj"
	-@erase "$(INTDIR)\win_ent.sbr"
	-@erase "$(INTDIR)\win_main.obj"
	-@erase "$(INTDIR)\win_main.sbr"
	-@erase "$(INTDIR)\win_qe3.obj"
	-@erase "$(INTDIR)\win_qe3.res"
	-@erase "$(INTDIR)\win_qe3.sbr"
	-@erase "$(INTDIR)\win_xy.obj"
	-@erase "$(INTDIR)\win_xy.sbr"
	-@erase "$(INTDIR)\win_z.obj"
	-@erase "$(INTDIR)\win_z.sbr"
	-@erase "$(INTDIR)\xy.obj"
	-@erase "$(INTDIR)\xy.sbr"
	-@erase "$(INTDIR)\z.obj"
	-@erase "$(INTDIR)\z.sbr"
	-@erase "$(OUTDIR)\qe4.bsc"
	-@erase "$(OUTDIR)\qe4.exe"
	-@erase "$(OUTDIR)\qe4.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Zd /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fr /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /Zd /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fr"$(INTDIR)/" /Fp"$(INTDIR)/qe4.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/win_qe3.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qe4.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\brush.sbr" \
	"$(INTDIR)\camera.sbr" \
	"$(INTDIR)\cmdlib.sbr" \
	"$(INTDIR)\csg.sbr" \
	"$(INTDIR)\drag.sbr" \
	"$(INTDIR)\eclass.sbr" \
	"$(INTDIR)\entity.sbr" \
	"$(INTDIR)\lbmlib.sbr" \
	"$(INTDIR)\map.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\mru.sbr" \
	"$(INTDIR)\parse.sbr" \
	"$(INTDIR)\points.sbr" \
	"$(INTDIR)\qe3.sbr" \
	"$(INTDIR)\select.sbr" \
	"$(INTDIR)\textures.sbr" \
	"$(INTDIR)\vertsel.sbr" \
	"$(INTDIR)\win_cam.sbr" \
	"$(INTDIR)\win_dlg.sbr" \
	"$(INTDIR)\win_ent.sbr" \
	"$(INTDIR)\win_main.sbr" \
	"$(INTDIR)\win_qe3.sbr" \
	"$(INTDIR)\win_xy.sbr" \
	"$(INTDIR)\win_z.sbr" \
	"$(INTDIR)\xy.sbr" \
	"$(INTDIR)\z.sbr"

"$(OUTDIR)\qe4.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 comctl32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=comctl32.lib opengl32.lib glu32.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/qe4.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/qe4.exe" 
LINK32_OBJS= \
	"$(INTDIR)\brush.obj" \
	"$(INTDIR)\camera.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\csg.obj" \
	"$(INTDIR)\drag.obj" \
	"$(INTDIR)\eclass.obj" \
	"$(INTDIR)\entity.obj" \
	"$(INTDIR)\lbmlib.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\mru.obj" \
	"$(INTDIR)\parse.obj" \
	"$(INTDIR)\points.obj" \
	"$(INTDIR)\qe3.obj" \
	"$(INTDIR)\select.obj" \
	"$(INTDIR)\textures.obj" \
	"$(INTDIR)\vertsel.obj" \
	"$(INTDIR)\win_cam.obj" \
	"$(INTDIR)\win_dlg.obj" \
	"$(INTDIR)\win_ent.obj" \
	"$(INTDIR)\win_main.obj" \
	"$(INTDIR)\win_qe3.obj" \
	"$(INTDIR)\win_qe3.res" \
	"$(INTDIR)\win_xy.obj" \
	"$(INTDIR)\win_z.obj" \
	"$(INTDIR)\xy.obj" \
	"$(INTDIR)\z.obj"

"$(OUTDIR)\qe4.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

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
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\qe4.exe" "$(OUTDIR)\qe4.bsc"

CLEAN : 
	-@erase "$(INTDIR)\brush.obj"
	-@erase "$(INTDIR)\brush.sbr"
	-@erase "$(INTDIR)\camera.obj"
	-@erase "$(INTDIR)\camera.sbr"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\cmdlib.sbr"
	-@erase "$(INTDIR)\csg.obj"
	-@erase "$(INTDIR)\csg.sbr"
	-@erase "$(INTDIR)\drag.obj"
	-@erase "$(INTDIR)\drag.sbr"
	-@erase "$(INTDIR)\eclass.obj"
	-@erase "$(INTDIR)\eclass.sbr"
	-@erase "$(INTDIR)\entity.obj"
	-@erase "$(INTDIR)\entity.sbr"
	-@erase "$(INTDIR)\lbmlib.obj"
	-@erase "$(INTDIR)\lbmlib.sbr"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\map.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\mru.obj"
	-@erase "$(INTDIR)\mru.sbr"
	-@erase "$(INTDIR)\parse.obj"
	-@erase "$(INTDIR)\parse.sbr"
	-@erase "$(INTDIR)\points.obj"
	-@erase "$(INTDIR)\points.sbr"
	-@erase "$(INTDIR)\qe3.obj"
	-@erase "$(INTDIR)\qe3.sbr"
	-@erase "$(INTDIR)\select.obj"
	-@erase "$(INTDIR)\select.sbr"
	-@erase "$(INTDIR)\textures.obj"
	-@erase "$(INTDIR)\textures.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\vertsel.obj"
	-@erase "$(INTDIR)\vertsel.sbr"
	-@erase "$(INTDIR)\win_cam.obj"
	-@erase "$(INTDIR)\win_cam.sbr"
	-@erase "$(INTDIR)\win_dlg.obj"
	-@erase "$(INTDIR)\win_dlg.sbr"
	-@erase "$(INTDIR)\win_ent.obj"
	-@erase "$(INTDIR)\win_ent.sbr"
	-@erase "$(INTDIR)\win_main.obj"
	-@erase "$(INTDIR)\win_main.sbr"
	-@erase "$(INTDIR)\win_qe3.obj"
	-@erase "$(INTDIR)\win_qe3.res"
	-@erase "$(INTDIR)\win_qe3.sbr"
	-@erase "$(INTDIR)\win_xy.obj"
	-@erase "$(INTDIR)\win_xy.sbr"
	-@erase "$(INTDIR)\win_z.obj"
	-@erase "$(INTDIR)\win_z.sbr"
	-@erase "$(INTDIR)\xy.obj"
	-@erase "$(INTDIR)\xy.sbr"
	-@erase "$(INTDIR)\z.obj"
	-@erase "$(INTDIR)\z.sbr"
	-@erase "$(OUTDIR)\qe4.bsc"
	-@erase "$(OUTDIR)\qe4.exe"
	-@erase "$(OUTDIR)\qe4.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qe4.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/win_qe3.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qe4.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\brush.sbr" \
	"$(INTDIR)\camera.sbr" \
	"$(INTDIR)\cmdlib.sbr" \
	"$(INTDIR)\csg.sbr" \
	"$(INTDIR)\drag.sbr" \
	"$(INTDIR)\eclass.sbr" \
	"$(INTDIR)\entity.sbr" \
	"$(INTDIR)\lbmlib.sbr" \
	"$(INTDIR)\map.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\mru.sbr" \
	"$(INTDIR)\parse.sbr" \
	"$(INTDIR)\points.sbr" \
	"$(INTDIR)\qe3.sbr" \
	"$(INTDIR)\select.sbr" \
	"$(INTDIR)\textures.sbr" \
	"$(INTDIR)\vertsel.sbr" \
	"$(INTDIR)\win_cam.sbr" \
	"$(INTDIR)\win_dlg.sbr" \
	"$(INTDIR)\win_ent.sbr" \
	"$(INTDIR)\win_main.sbr" \
	"$(INTDIR)\win_qe3.sbr" \
	"$(INTDIR)\win_xy.sbr" \
	"$(INTDIR)\win_z.sbr" \
	"$(INTDIR)\xy.sbr" \
	"$(INTDIR)\z.sbr"

"$(OUTDIR)\qe4.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 comctl32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386
LINK32_FLAGS=comctl32.lib opengl32.lib glu32.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows\
 /profile /map:"$(INTDIR)/qe4.map" /debug /machine:I386 /out:"$(OUTDIR)/qe4.exe"\
 
LINK32_OBJS= \
	"$(INTDIR)\brush.obj" \
	"$(INTDIR)\camera.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\csg.obj" \
	"$(INTDIR)\drag.obj" \
	"$(INTDIR)\eclass.obj" \
	"$(INTDIR)\entity.obj" \
	"$(INTDIR)\lbmlib.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\mru.obj" \
	"$(INTDIR)\parse.obj" \
	"$(INTDIR)\points.obj" \
	"$(INTDIR)\qe3.obj" \
	"$(INTDIR)\select.obj" \
	"$(INTDIR)\textures.obj" \
	"$(INTDIR)\vertsel.obj" \
	"$(INTDIR)\win_cam.obj" \
	"$(INTDIR)\win_dlg.obj" \
	"$(INTDIR)\win_ent.obj" \
	"$(INTDIR)\win_main.obj" \
	"$(INTDIR)\win_qe3.obj" \
	"$(INTDIR)\win_qe3.res" \
	"$(INTDIR)\win_xy.obj" \
	"$(INTDIR)\win_z.obj" \
	"$(INTDIR)\xy.obj" \
	"$(INTDIR)\z.obj"

"$(OUTDIR)\qe4.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "qe3___Wi"
# PROP BASE Intermediate_Dir "qe3___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "debug_alpha"
# PROP Intermediate_Dir "debug_alpha"
# PROP Target_Dir ""
OUTDIR=.\debug_alpha
INTDIR=.\debug_alpha

ALL :                      "$(OUTDIR)\qe3.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qe3.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\debug_alpha/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /alpha
# ADD MTL /nologo /D "_DEBUG" /alpha
MTL_PROJ=/nologo /D "_DEBUG" /alpha 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/win_qe3.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qe3.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:ALPHA
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 comctl32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:ALPHA
# SUBTRACT LINK32 /incremental:no
LINK32_FLAGS=comctl32.lib opengl32.lib glu32.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/qe3.pdb" /debug\
 /machine:ALPHA /out:"$(OUTDIR)/qe3.exe" 
LINK32_OBJS= \
	"$(INTDIR)\brush.obj" \
	"$(INTDIR)\camera.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\csg.obj" \
	"$(INTDIR)\drag.obj" \
	"$(INTDIR)\eclass.obj" \
	"$(INTDIR)\entity.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\mru.obj" \
	"$(INTDIR)\parse.obj" \
	"$(INTDIR)\points.obj" \
	"$(INTDIR)\qe3.obj" \
	"$(INTDIR)\select.obj" \
	"$(INTDIR)\textures.obj" \
	"$(INTDIR)\vertsel.obj" \
	"$(INTDIR)\win_cam.obj" \
	"$(INTDIR)\win_dlg.obj" \
	"$(INTDIR)\win_ent.obj" \
	"$(INTDIR)\win_main.obj" \
	"$(INTDIR)\win_qe3.obj" \
	"$(INTDIR)\win_qe3.res" \
	"$(INTDIR)\win_xy.obj" \
	"$(INTDIR)\win_z.obj" \
	"$(INTDIR)\xy.obj" \
	"$(INTDIR)\z.obj"

"$(OUTDIR)\qe3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "qe3___W0"
# PROP BASE Intermediate_Dir "qe3___W0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "release_alpha"
# PROP Intermediate_Dir "release_alpha"
# PROP Target_Dir ""
OUTDIR=.\release_alpha
INTDIR=.\release_alpha

ALL :                      "$(OUTDIR)\qe3.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /Gt0 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Gt0 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /Gt0 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qe3.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\release_alpha/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /alpha
# ADD MTL /nologo /D "NDEBUG" /alpha
MTL_PROJ=/nologo /D "NDEBUG" /alpha 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/win_qe3.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qe3.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:ALPHA
# ADD LINK32 comctl32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:ALPHA
LINK32_FLAGS=comctl32.lib opengl32.lib glu32.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/qe3.pdb" /machine:ALPHA\
 /out:"$(OUTDIR)/qe3.exe" 
LINK32_OBJS= \
	"$(INTDIR)\brush.obj" \
	"$(INTDIR)\camera.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\csg.obj" \
	"$(INTDIR)\drag.obj" \
	"$(INTDIR)\eclass.obj" \
	"$(INTDIR)\entity.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\mru.obj" \
	"$(INTDIR)\parse.obj" \
	"$(INTDIR)\points.obj" \
	"$(INTDIR)\qe3.obj" \
	"$(INTDIR)\select.obj" \
	"$(INTDIR)\textures.obj" \
	"$(INTDIR)\vertsel.obj" \
	"$(INTDIR)\win_cam.obj" \
	"$(INTDIR)\win_dlg.obj" \
	"$(INTDIR)\win_ent.obj" \
	"$(INTDIR)\win_main.obj" \
	"$(INTDIR)\win_qe3.obj" \
	"$(INTDIR)\win_qe3.res" \
	"$(INTDIR)\win_xy.obj" \
	"$(INTDIR)\win_z.obj" \
	"$(INTDIR)\xy.obj" \
	"$(INTDIR)\z.obj"

"$(OUTDIR)\qe3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Target

# Name "qe3 - Win32 Release"
# Name "qe3 - Win32 Debug"
# Name "qe3 - Win32 (ALPHA) Debug"
# Name "qe3 - Win32 (ALPHA) Release"

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\textures.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\textures.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_TEXTU=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\textures.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"

"$(INTDIR)\textures.sbr" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_TEXTU=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\textures.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"

"$(INTDIR)\textures.sbr" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_TEXTU=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\textures.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_TEXTU=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\textures.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\resource.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mathlib.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mathlib.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_MATHL=\
	".\cmdlib.h"\
	".\mathlib.h"\
	

"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"

"$(INTDIR)\mathlib.sbr" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_MATHL=\
	".\cmdlib.h"\
	".\mathlib.h"\
	

"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"

"$(INTDIR)\mathlib.sbr" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_MATHL=\
	".\cmdlib.h"\
	".\mathlib.h"\
	

"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_MATHL=\
	".\cmdlib.h"\
	".\mathlib.h"\
	

"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\map.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\map.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_MAP_C=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"

"$(INTDIR)\map.sbr" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_MAP_C=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"

"$(INTDIR)\map.sbr" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_MAP_C=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_MAP_C=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cmdlib.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cmdlib.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_CMDLI=\
	".\cmdlib.h"\
	

"$(INTDIR)\cmdlib.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"

"$(INTDIR)\cmdlib.sbr" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_CMDLI=\
	".\cmdlib.h"\
	

"$(INTDIR)\cmdlib.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"

"$(INTDIR)\cmdlib.sbr" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_CMDLI=\
	".\cmdlib.h"\
	

"$(INTDIR)\cmdlib.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_CMDLI=\
	".\cmdlib.h"\
	

"$(INTDIR)\cmdlib.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\brush.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\brush.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_BRUSH=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\brush.obj" : $(SOURCE) $(DEP_CPP_BRUSH) "$(INTDIR)"

"$(INTDIR)\brush.sbr" : $(SOURCE) $(DEP_CPP_BRUSH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_BRUSH=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\brush.obj" : $(SOURCE) $(DEP_CPP_BRUSH) "$(INTDIR)"

"$(INTDIR)\brush.sbr" : $(SOURCE) $(DEP_CPP_BRUSH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_BRUSH=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\brush.obj" : $(SOURCE) $(DEP_CPP_BRUSH) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_BRUSH=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\brush.obj" : $(SOURCE) $(DEP_CPP_BRUSH) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\notes.txt

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_cam.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_WIN_C=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_cam.obj" : $(SOURCE) $(DEP_CPP_WIN_C) "$(INTDIR)"

"$(INTDIR)\win_cam.sbr" : $(SOURCE) $(DEP_CPP_WIN_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_WIN_C=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_cam.obj" : $(SOURCE) $(DEP_CPP_WIN_C) "$(INTDIR)"

"$(INTDIR)\win_cam.sbr" : $(SOURCE) $(DEP_CPP_WIN_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_WIN_C=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_cam.obj" : $(SOURCE) $(DEP_CPP_WIN_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_WIN_C=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_cam.obj" : $(SOURCE) $(DEP_CPP_WIN_C) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qe3.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\parse.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\parse.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_PARSE=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\parse.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"

"$(INTDIR)\parse.sbr" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_PARSE=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\parse.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"

"$(INTDIR)\parse.sbr" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_PARSE=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\parse.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_PARSE=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\parse.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\camera.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\camera.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_CAMER=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\camera.obj" : $(SOURCE) $(DEP_CPP_CAMER) "$(INTDIR)"

"$(INTDIR)\camera.sbr" : $(SOURCE) $(DEP_CPP_CAMER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_CAMER=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\camera.obj" : $(SOURCE) $(DEP_CPP_CAMER) "$(INTDIR)"

"$(INTDIR)\camera.sbr" : $(SOURCE) $(DEP_CPP_CAMER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_CAMER=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\camera.obj" : $(SOURCE) $(DEP_CPP_CAMER) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_CAMER=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\camera.obj" : $(SOURCE) $(DEP_CPP_CAMER) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_qe3.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_xy.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_WIN_X=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_xy.obj" : $(SOURCE) $(DEP_CPP_WIN_X) "$(INTDIR)"

"$(INTDIR)\win_xy.sbr" : $(SOURCE) $(DEP_CPP_WIN_X) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_WIN_X=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_xy.obj" : $(SOURCE) $(DEP_CPP_WIN_X) "$(INTDIR)"

"$(INTDIR)\win_xy.sbr" : $(SOURCE) $(DEP_CPP_WIN_X) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_WIN_X=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_xy.obj" : $(SOURCE) $(DEP_CPP_WIN_X) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_WIN_X=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_xy.obj" : $(SOURCE) $(DEP_CPP_WIN_X) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_qe3.rc

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_RSC_WIN_Q=\
	".\icon1.ico"\
	".\q.bmp"\
	".\toolbar1.bmp"\
	

"$(INTDIR)\win_qe3.res" : $(SOURCE) $(DEP_RSC_WIN_Q) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_RSC_WIN_Q=\
	".\icon1.ico"\
	".\q.bmp"\
	".\toolbar1.bmp"\
	

"$(INTDIR)\win_qe3.res" : $(SOURCE) $(DEP_RSC_WIN_Q) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_RSC_WIN_Q=\
	".\icon1.ico"\
	".\toolbar1.bmp"\
	

"$(INTDIR)\win_qe3.res" : $(SOURCE) $(DEP_RSC_WIN_Q) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/win_qe3.res" /d "_DEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_RSC_WIN_Q=\
	".\icon1.ico"\
	".\toolbar1.bmp"\
	

"$(INTDIR)\win_qe3.res" : $(SOURCE) $(DEP_RSC_WIN_Q) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/win_qe3.res" /d "NDEBUG" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\xy.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_XY_C14=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\xy.obj" : $(SOURCE) $(DEP_CPP_XY_C14) "$(INTDIR)"

"$(INTDIR)\xy.sbr" : $(SOURCE) $(DEP_CPP_XY_C14) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_XY_C14=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\xy.obj" : $(SOURCE) $(DEP_CPP_XY_C14) "$(INTDIR)"

"$(INTDIR)\xy.sbr" : $(SOURCE) $(DEP_CPP_XY_C14) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_XY_C14=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\xy.obj" : $(SOURCE) $(DEP_CPP_XY_C14) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_XY_C14=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\xy.obj" : $(SOURCE) $(DEP_CPP_XY_C14) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\xy.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\select.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_SELEC=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_SELEC=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_SELEC=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_SELEC=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_qe3.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_WIN_QE=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_qe3.obj" : $(SOURCE) $(DEP_CPP_WIN_QE) "$(INTDIR)"

"$(INTDIR)\win_qe3.sbr" : $(SOURCE) $(DEP_CPP_WIN_QE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_WIN_QE=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_qe3.obj" : $(SOURCE) $(DEP_CPP_WIN_QE) "$(INTDIR)"

"$(INTDIR)\win_qe3.sbr" : $(SOURCE) $(DEP_CPP_WIN_QE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_WIN_QE=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_qe3.obj" : $(SOURCE) $(DEP_CPP_WIN_QE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_WIN_QE=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_qe3.obj" : $(SOURCE) $(DEP_CPP_WIN_QE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\select.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qe3.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_QE3_C=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\qe3.obj" : $(SOURCE) $(DEP_CPP_QE3_C) "$(INTDIR)"

"$(INTDIR)\qe3.sbr" : $(SOURCE) $(DEP_CPP_QE3_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_QE3_C=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\qe3.obj" : $(SOURCE) $(DEP_CPP_QE3_C) "$(INTDIR)"

"$(INTDIR)\qe3.sbr" : $(SOURCE) $(DEP_CPP_QE3_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_QE3_C=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\qe3.obj" : $(SOURCE) $(DEP_CPP_QE3_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_QE3_C=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\qe3.obj" : $(SOURCE) $(DEP_CPP_QE3_C) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\eclass.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_ECLAS=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\eclass.obj" : $(SOURCE) $(DEP_CPP_ECLAS) "$(INTDIR)"

"$(INTDIR)\eclass.sbr" : $(SOURCE) $(DEP_CPP_ECLAS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_ECLAS=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\eclass.obj" : $(SOURCE) $(DEP_CPP_ECLAS) "$(INTDIR)"

"$(INTDIR)\eclass.sbr" : $(SOURCE) $(DEP_CPP_ECLAS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_ECLAS=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\eclass.obj" : $(SOURCE) $(DEP_CPP_ECLAS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_ECLAS=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\eclass.obj" : $(SOURCE) $(DEP_CPP_ECLAS) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\eclass.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\entity.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_ENTIT=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\entity.obj" : $(SOURCE) $(DEP_CPP_ENTIT) "$(INTDIR)"

"$(INTDIR)\entity.sbr" : $(SOURCE) $(DEP_CPP_ENTIT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_ENTIT=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\entity.obj" : $(SOURCE) $(DEP_CPP_ENTIT) "$(INTDIR)"

"$(INTDIR)\entity.sbr" : $(SOURCE) $(DEP_CPP_ENTIT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_ENTIT=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\entity.obj" : $(SOURCE) $(DEP_CPP_ENTIT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_ENTIT=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\entity.obj" : $(SOURCE) $(DEP_CPP_ENTIT) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\entity.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_dlg.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_WIN_D=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_dlg.obj" : $(SOURCE) $(DEP_CPP_WIN_D) "$(INTDIR)"

"$(INTDIR)\win_dlg.sbr" : $(SOURCE) $(DEP_CPP_WIN_D) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_WIN_D=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_dlg.obj" : $(SOURCE) $(DEP_CPP_WIN_D) "$(INTDIR)"

"$(INTDIR)\win_dlg.sbr" : $(SOURCE) $(DEP_CPP_WIN_D) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_WIN_D=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_dlg.obj" : $(SOURCE) $(DEP_CPP_WIN_D) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_WIN_D=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_dlg.obj" : $(SOURCE) $(DEP_CPP_WIN_D) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\points.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_POINT=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\points.obj" : $(SOURCE) $(DEP_CPP_POINT) "$(INTDIR)"

"$(INTDIR)\points.sbr" : $(SOURCE) $(DEP_CPP_POINT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_POINT=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\points.obj" : $(SOURCE) $(DEP_CPP_POINT) "$(INTDIR)"

"$(INTDIR)\points.sbr" : $(SOURCE) $(DEP_CPP_POINT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_POINT=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\points.obj" : $(SOURCE) $(DEP_CPP_POINT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_POINT=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\points.obj" : $(SOURCE) $(DEP_CPP_POINT) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_z.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_WIN_Z=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_z.obj" : $(SOURCE) $(DEP_CPP_WIN_Z) "$(INTDIR)"

"$(INTDIR)\win_z.sbr" : $(SOURCE) $(DEP_CPP_WIN_Z) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_WIN_Z=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_z.obj" : $(SOURCE) $(DEP_CPP_WIN_Z) "$(INTDIR)"

"$(INTDIR)\win_z.sbr" : $(SOURCE) $(DEP_CPP_WIN_Z) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_WIN_Z=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_z.obj" : $(SOURCE) $(DEP_CPP_WIN_Z) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_WIN_Z=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_z.obj" : $(SOURCE) $(DEP_CPP_WIN_Z) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\z.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_Z_C26=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\z.obj" : $(SOURCE) $(DEP_CPP_Z_C26) "$(INTDIR)"

"$(INTDIR)\z.sbr" : $(SOURCE) $(DEP_CPP_Z_C26) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_Z_C26=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\z.obj" : $(SOURCE) $(DEP_CPP_Z_C26) "$(INTDIR)"

"$(INTDIR)\z.sbr" : $(SOURCE) $(DEP_CPP_Z_C26) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_Z_C26=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\z.obj" : $(SOURCE) $(DEP_CPP_Z_C26) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_Z_C26=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\z.obj" : $(SOURCE) $(DEP_CPP_Z_C26) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\z.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\drag.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_DRAG_=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\drag.obj" : $(SOURCE) $(DEP_CPP_DRAG_) "$(INTDIR)"

"$(INTDIR)\drag.sbr" : $(SOURCE) $(DEP_CPP_DRAG_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_DRAG_=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\drag.obj" : $(SOURCE) $(DEP_CPP_DRAG_) "$(INTDIR)"

"$(INTDIR)\drag.sbr" : $(SOURCE) $(DEP_CPP_DRAG_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_DRAG_=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\drag.obj" : $(SOURCE) $(DEP_CPP_DRAG_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_DRAG_=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\drag.obj" : $(SOURCE) $(DEP_CPP_DRAG_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_main.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_WIN_M=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\entityw.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_main.obj" : $(SOURCE) $(DEP_CPP_WIN_M) "$(INTDIR)"

"$(INTDIR)\win_main.sbr" : $(SOURCE) $(DEP_CPP_WIN_M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_WIN_M=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\entityw.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_main.obj" : $(SOURCE) $(DEP_CPP_WIN_M) "$(INTDIR)"

"$(INTDIR)\win_main.sbr" : $(SOURCE) $(DEP_CPP_WIN_M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_WIN_M=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_main.obj" : $(SOURCE) $(DEP_CPP_WIN_M) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_WIN_M=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_main.obj" : $(SOURCE) $(DEP_CPP_WIN_M) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\csg.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_CSG_C=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\csg.obj" : $(SOURCE) $(DEP_CPP_CSG_C) "$(INTDIR)"

"$(INTDIR)\csg.sbr" : $(SOURCE) $(DEP_CPP_CSG_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_CSG_C=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\csg.obj" : $(SOURCE) $(DEP_CPP_CSG_C) "$(INTDIR)"

"$(INTDIR)\csg.sbr" : $(SOURCE) $(DEP_CPP_CSG_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_CSG_C=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\csg.obj" : $(SOURCE) $(DEP_CPP_CSG_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_CSG_C=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\csg.obj" : $(SOURCE) $(DEP_CPP_CSG_C) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\vertsel.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_VERTS=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\vertsel.obj" : $(SOURCE) $(DEP_CPP_VERTS) "$(INTDIR)"

"$(INTDIR)\vertsel.sbr" : $(SOURCE) $(DEP_CPP_VERTS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_VERTS=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\vertsel.obj" : $(SOURCE) $(DEP_CPP_VERTS) "$(INTDIR)"

"$(INTDIR)\vertsel.sbr" : $(SOURCE) $(DEP_CPP_VERTS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_VERTS=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\vertsel.obj" : $(SOURCE) $(DEP_CPP_VERTS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_VERTS=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\vertsel.obj" : $(SOURCE) $(DEP_CPP_VERTS) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mru.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_MRU_C=\
	".\mru.h"\
	

"$(INTDIR)\mru.obj" : $(SOURCE) $(DEP_CPP_MRU_C) "$(INTDIR)"

"$(INTDIR)\mru.sbr" : $(SOURCE) $(DEP_CPP_MRU_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_MRU_C=\
	".\mru.h"\
	

"$(INTDIR)\mru.obj" : $(SOURCE) $(DEP_CPP_MRU_C) "$(INTDIR)"

"$(INTDIR)\mru.sbr" : $(SOURCE) $(DEP_CPP_MRU_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_MRU_C=\
	".\mru.h"\
	

"$(INTDIR)\mru.obj" : $(SOURCE) $(DEP_CPP_MRU_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_MRU_C=\
	".\mru.h"\
	

"$(INTDIR)\mru.obj" : $(SOURCE) $(DEP_CPP_MRU_C) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_ent.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_WIN_E=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\entityw.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_ent.obj" : $(SOURCE) $(DEP_CPP_WIN_E) "$(INTDIR)"

"$(INTDIR)\win_ent.sbr" : $(SOURCE) $(DEP_CPP_WIN_E) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_WIN_E=\
	".\brush.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\entityw.h"\
	".\glingr.h"\
	".\lbmlib.h"\
	".\map.h"\
	".\mathlib.h"\
	".\mru.h"\
	".\parse.h"\
	".\qe3.h"\
	".\qedefs.h"\
	".\qfiles.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_ent.obj" : $(SOURCE) $(DEP_CPP_WIN_E) "$(INTDIR)"

"$(INTDIR)\win_ent.sbr" : $(SOURCE) $(DEP_CPP_WIN_E) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

DEP_CPP_WIN_E=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\entityw.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_ent.obj" : $(SOURCE) $(DEP_CPP_WIN_E) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

DEP_CPP_WIN_E=\
	".\brush.h"\
	".\bspfile.h"\
	".\camera.h"\
	".\cmdlib.h"\
	".\entity.h"\
	".\entityw.h"\
	".\gl\GL.H"\
	".\gl\GLAUX.H"\
	".\gl\GLU.H"\
	".\glingr.h"\
	".\map.h"\
	".\mathlib.h"\
	".\parse.h"\
	".\qe3.h"\
	".\select.h"\
	".\textures.h"\
	".\xy.h"\
	".\z.h"\
	

"$(INTDIR)\win_ent.obj" : $(SOURCE) $(DEP_CPP_WIN_E) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lbmlib.c

!IF  "$(CFG)" == "qe3 - Win32 Release"

DEP_CPP_LBMLI=\
	".\cmdlib.h"\
	".\lbmlib.h"\
	

"$(INTDIR)\lbmlib.obj" : $(SOURCE) $(DEP_CPP_LBMLI) "$(INTDIR)"

"$(INTDIR)\lbmlib.sbr" : $(SOURCE) $(DEP_CPP_LBMLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

DEP_CPP_LBMLI=\
	".\cmdlib.h"\
	".\lbmlib.h"\
	

"$(INTDIR)\lbmlib.obj" : $(SOURCE) $(DEP_CPP_LBMLI) "$(INTDIR)"

"$(INTDIR)\lbmlib.sbr" : $(SOURCE) $(DEP_CPP_LBMLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qfiles.h

!IF  "$(CFG)" == "qe3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qe3 - Win32 Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "qe3 - Win32 (ALPHA) Release"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
