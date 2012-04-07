# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=texpaint - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to texpaint - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "texpaint - Win32 Release" && "$(CFG)" !=\
 "texpaint - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "texpaint.mak" CFG="texpaint - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "texpaint - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "texpaint - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "texpaint - Win32 Debug"
MTL=mktyplib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "texpaint - Win32 Release"

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

ALL : "$(OUTDIR)\texpaint.exe"

CLEAN : 
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\l3dslib.obj"
	-@erase "$(INTDIR)\lbmlib.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(INTDIR)\texmake.res"
	-@erase "$(INTDIR)\texpaint.obj"
	-@erase "$(INTDIR)\trilib.obj"
	-@erase "$(INTDIR)\win_cam.obj"
	-@erase "$(INTDIR)\win_main.obj"
	-@erase "$(INTDIR)\win_pal.obj"
	-@erase "$(INTDIR)\win_skin.obj"
	-@erase "$(OUTDIR)\texpaint.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /GX /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /GX /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/texpaint.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/texmake.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/texpaint.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 opengl32.lib glu32.lib glaux.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=opengl32.lib glu32.lib glaux.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/texpaint.pdb" /machine:I386 /out:"$(OUTDIR)/texpaint.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\l3dslib.obj" \
	"$(INTDIR)\lbmlib.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\scriplib.obj" \
	"$(INTDIR)\texmake.res" \
	"$(INTDIR)\texpaint.obj" \
	"$(INTDIR)\trilib.obj" \
	"$(INTDIR)\win_cam.obj" \
	"$(INTDIR)\win_main.obj" \
	"$(INTDIR)\win_pal.obj" \
	"$(INTDIR)\win_skin.obj"

"$(OUTDIR)\texpaint.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "texpaint - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "texpaint"
# PROP BASE Intermediate_Dir "texpaint"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\texpaint.exe"

CLEAN : 
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\l3dslib.obj"
	-@erase "$(INTDIR)\lbmlib.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(INTDIR)\texmake.res"
	-@erase "$(INTDIR)\texpaint.obj"
	-@erase "$(INTDIR)\trilib.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\win_cam.obj"
	-@erase "$(INTDIR)\win_main.obj"
	-@erase "$(INTDIR)\win_pal.obj"
	-@erase "$(INTDIR)\win_skin.obj"
	-@erase "$(OUTDIR)\texpaint.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN_ERROR" /YX /c
CPP_PROJ=/nologo /MLd /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "WIN_ERROR" /Fp"$(INTDIR)/texpaint.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/texmake.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/texpaint.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 opengl32.lib glu32.lib glaux.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386
LINK32_FLAGS=opengl32.lib glu32.lib glaux.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:none /debug\
 /machine:I386 /out:"$(OUTDIR)/texpaint.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\l3dslib.obj" \
	"$(INTDIR)\lbmlib.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\scriplib.obj" \
	"$(INTDIR)\texmake.res" \
	"$(INTDIR)\texpaint.obj" \
	"$(INTDIR)\trilib.obj" \
	"$(INTDIR)\win_cam.obj" \
	"$(INTDIR)\win_main.obj" \
	"$(INTDIR)\win_pal.obj" \
	"$(INTDIR)\win_skin.obj"

"$(OUTDIR)\texpaint.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

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

################################################################################
# Begin Target

# Name "texpaint - Win32 Release"
# Name "texpaint - Win32 Debug"

!IF  "$(CFG)" == "texpaint - Win32 Release"

!ELSEIF  "$(CFG)" == "texpaint - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\texpaint.c
DEP_CPP_TEXPA=\
	"..\common\cmdlib.h"\
	"..\common\l3dslib.h"\
	"..\common\lbmlib.h"\
	"..\common\mathlib.h"\
	"..\common\trilib.h"\
	".\texpaint.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\texpaint.obj" : $(SOURCE) $(DEP_CPP_TEXPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=\quake\utils2\common\mathlib.c
DEP_CPP_MATHL=\
	"..\..\..\quake\utils2\common\cmdlib.h"\
	"..\..\..\quake\utils2\common\mathlib.h"\
	

"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\quake\utils2\common\l3dslib.c
DEP_CPP_L3DSL=\
	"..\..\..\quake\utils2\common\cmdlib.h"\
	"..\..\..\quake\utils2\common\l3dslib.h"\
	"..\..\..\quake\utils2\common\mathlib.h"\
	"..\..\..\quake\utils2\common\trilib.h"\
	

"$(INTDIR)\l3dslib.obj" : $(SOURCE) $(DEP_CPP_L3DSL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\quake\utils2\common\lbmlib.c
DEP_CPP_LBMLI=\
	"..\..\..\quake\utils2\common\cmdlib.h"\
	"..\..\..\quake\utils2\common\lbmlib.h"\
	

"$(INTDIR)\lbmlib.obj" : $(SOURCE) $(DEP_CPP_LBMLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\quake\utils2\common\scriplib.c
DEP_CPP_SCRIP=\
	"..\..\..\quake\utils2\common\cmdlib.h"\
	"..\..\..\quake\utils2\common\scriplib.h"\
	

"$(INTDIR)\scriplib.obj" : $(SOURCE) $(DEP_CPP_SCRIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\quake\utils2\common\trilib.c
DEP_CPP_TRILI=\
	"..\..\..\quake\utils2\common\cmdlib.h"\
	"..\..\..\quake\utils2\common\mathlib.h"\
	"..\..\..\quake\utils2\common\trilib.h"\
	

"$(INTDIR)\trilib.obj" : $(SOURCE) $(DEP_CPP_TRILI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_skin.c
DEP_CPP_WIN_S=\
	"..\common\cmdlib.h"\
	"..\common\l3dslib.h"\
	"..\common\lbmlib.h"\
	"..\common\mathlib.h"\
	"..\common\trilib.h"\
	".\texpaint.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_skin.obj" : $(SOURCE) $(DEP_CPP_WIN_S) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_main.c
DEP_CPP_WIN_M=\
	"..\common\cmdlib.h"\
	"..\common\l3dslib.h"\
	"..\common\lbmlib.h"\
	"..\common\mathlib.h"\
	"..\common\trilib.h"\
	".\texpaint.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_main.obj" : $(SOURCE) $(DEP_CPP_WIN_M) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_pal.c
DEP_CPP_WIN_P=\
	"..\common\cmdlib.h"\
	"..\common\l3dslib.h"\
	"..\common\lbmlib.h"\
	"..\common\mathlib.h"\
	"..\common\trilib.h"\
	".\texpaint.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_pal.obj" : $(SOURCE) $(DEP_CPP_WIN_P) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win_cam.c
DEP_CPP_WIN_C=\
	"..\common\cmdlib.h"\
	"..\common\l3dslib.h"\
	"..\common\lbmlib.h"\
	"..\common\mathlib.h"\
	"..\common\trilib.h"\
	".\texpaint.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\win_cam.obj" : $(SOURCE) $(DEP_CPP_WIN_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\texmake.rc

"$(INTDIR)\texmake.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\texpaint.h

!IF  "$(CFG)" == "texpaint - Win32 Release"

!ELSEIF  "$(CFG)" == "texpaint - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\quake\utils2\common\lbmlib.h

!IF  "$(CFG)" == "texpaint - Win32 Release"

!ELSEIF  "$(CFG)" == "texpaint - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\quake\utils2\common\cmdlib.h

!IF  "$(CFG)" == "texpaint - Win32 Release"

!ELSEIF  "$(CFG)" == "texpaint - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\quake\utils2\common\cmdlib.c
DEP_CPP_CMDLI=\
	"..\..\..\quake\utils2\common\cmdlib.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\cmdlib.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
