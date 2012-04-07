# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=qdata - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to qdata - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "qdata - Win32 Release" && "$(CFG)" != "qdata - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "qdata.mak" CFG="qdata - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qdata - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "qdata - Win32 Debug" (based on "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "qdata - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qdata - Win32 Release"

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

ALL : "$(OUTDIR)\qdata.exe"

CLEAN : 
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\images.obj"
	-@erase "$(INTDIR)\l3dslib.obj"
	-@erase "$(INTDIR)\lbmlib.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\models.obj"
	-@erase "$(INTDIR)\qdata.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(INTDIR)\sprites.obj"
	-@erase "$(INTDIR)\tables.obj"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\trilib.obj"
	-@erase "$(OUTDIR)\qdata.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "../common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "../common" /D "WIN32" /D "NDEBUG" /D\
 "_CONSOLE" /Fp"$(INTDIR)/qdata.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qdata.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/qdata.pdb" /machine:I386 /out:"$(OUTDIR)/qdata.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\images.obj" \
	"$(INTDIR)\l3dslib.obj" \
	"$(INTDIR)\lbmlib.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\models.obj" \
	"$(INTDIR)\qdata.obj" \
	"$(INTDIR)\scriplib.obj" \
	"$(INTDIR)\sprites.obj" \
	"$(INTDIR)\tables.obj" \
	"$(INTDIR)\threads.obj" \
	"$(INTDIR)\trilib.obj"

"$(OUTDIR)\qdata.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qdata - Win32 Debug"

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

ALL : "$(OUTDIR)\qdata.exe"

CLEAN : 
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\images.obj"
	-@erase "$(INTDIR)\l3dslib.obj"
	-@erase "$(INTDIR)\lbmlib.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\models.obj"
	-@erase "$(INTDIR)\qdata.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(INTDIR)\sprites.obj"
	-@erase "$(INTDIR)\tables.obj"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\trilib.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\qdata.exe"
	-@erase "$(OUTDIR)\qdata.ilk"
	-@erase "$(OUTDIR)\qdata.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "../common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "../common" /D "WIN32" /D "_DEBUG"\
 /D "_CONSOLE" /Fp"$(INTDIR)/qdata.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qdata.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/qdata.pdb" /debug /machine:I386 /out:"$(OUTDIR)/qdata.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\images.obj" \
	"$(INTDIR)\l3dslib.obj" \
	"$(INTDIR)\lbmlib.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\models.obj" \
	"$(INTDIR)\qdata.obj" \
	"$(INTDIR)\scriplib.obj" \
	"$(INTDIR)\sprites.obj" \
	"$(INTDIR)\tables.obj" \
	"$(INTDIR)\threads.obj" \
	"$(INTDIR)\trilib.obj"

"$(OUTDIR)\qdata.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "qdata - Win32 Release"
# Name "qdata - Win32 Debug"

!IF  "$(CFG)" == "qdata - Win32 Release"

!ELSEIF  "$(CFG)" == "qdata - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\images.c
DEP_CPP_IMAGE=\
	"..\common\qfiles.h"\
	".\../common\bspfile.h"\
	".\../common\cmdlib.h"\
	".\../common\l3dslib.h"\
	".\../common\lbmlib.h"\
	".\../common\mathlib.h"\
	".\../common\scriplib.h"\
	".\../common\threads.h"\
	".\../common\trilib.h"\
	".\qdata.h"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\images.obj" : $(SOURCE) $(DEP_CPP_IMAGE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\modelgen.h

!IF  "$(CFG)" == "qdata - Win32 Release"

!ELSEIF  "$(CFG)" == "qdata - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qdata.h

!IF  "$(CFG)" == "qdata - Win32 Release"

!ELSEIF  "$(CFG)" == "qdata - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sprites.c
DEP_CPP_SPRIT=\
	"..\common\qfiles.h"\
	".\../common\bspfile.h"\
	".\../common\cmdlib.h"\
	".\../common\l3dslib.h"\
	".\../common\lbmlib.h"\
	".\../common\mathlib.h"\
	".\../common\scriplib.h"\
	".\../common\threads.h"\
	".\../common\trilib.h"\
	".\qdata.h"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\sprites.obj" : $(SOURCE) $(DEP_CPP_SPRIT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\l3dslib.c
DEP_CPP_L3DSL=\
	".\../common\cmdlib.h"\
	".\../common\l3dslib.h"\
	".\../common\mathlib.h"\
	".\../common\trilib.h"\
	

"$(INTDIR)\l3dslib.obj" : $(SOURCE) $(DEP_CPP_L3DSL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\lbmlib.c
DEP_CPP_LBMLI=\
	".\../common\cmdlib.h"\
	".\../common\lbmlib.h"\
	

"$(INTDIR)\lbmlib.obj" : $(SOURCE) $(DEP_CPP_LBMLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\mathlib.c
DEP_CPP_MATHL=\
	".\../common\cmdlib.h"\
	".\../common\mathlib.h"\
	

"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\scriplib.c
DEP_CPP_SCRIP=\
	".\../common\cmdlib.h"\
	".\../common\scriplib.h"\
	

"$(INTDIR)\scriplib.obj" : $(SOURCE) $(DEP_CPP_SCRIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\threads.c
DEP_CPP_THREA=\
	".\../common\cmdlib.h"\
	".\../common\threads.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\threads.obj" : $(SOURCE) $(DEP_CPP_THREA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\trilib.c
DEP_CPP_TRILI=\
	".\../common\cmdlib.h"\
	".\../common\mathlib.h"\
	".\../common\trilib.h"\
	

"$(INTDIR)\trilib.obj" : $(SOURCE) $(DEP_CPP_TRILI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\cmdlib.c
DEP_CPP_CMDLI=\
	".\../common\cmdlib.h"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\cmdlib.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\models.c
DEP_CPP_MODEL=\
	"..\common\qfiles.h"\
	".\../common\bspfile.h"\
	".\../common\cmdlib.h"\
	".\../common\l3dslib.h"\
	".\../common\lbmlib.h"\
	".\../common\mathlib.h"\
	".\../common\scriplib.h"\
	".\../common\threads.h"\
	".\../common\trilib.h"\
	".\anorms.h"\
	".\qdata.h"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\models.obj" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qdata.c
DEP_CPP_QDATA=\
	"..\common\qfiles.h"\
	".\../common\bspfile.h"\
	".\../common\cmdlib.h"\
	".\../common\l3dslib.h"\
	".\../common\lbmlib.h"\
	".\../common\mathlib.h"\
	".\../common\scriplib.h"\
	".\../common\threads.h"\
	".\../common\trilib.h"\
	".\qdata.h"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\qdata.obj" : $(SOURCE) $(DEP_CPP_QDATA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\mathlib.h

!IF  "$(CFG)" == "qdata - Win32 Release"

!ELSEIF  "$(CFG)" == "qdata - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\lbmlib.h

!IF  "$(CFG)" == "qdata - Win32 Release"

!ELSEIF  "$(CFG)" == "qdata - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\cmdlib.h

!IF  "$(CFG)" == "qdata - Win32 Release"

!ELSEIF  "$(CFG)" == "qdata - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tables.c
DEP_CPP_TABLE=\
	"..\common\qfiles.h"\
	".\../common\bspfile.h"\
	".\../common\cmdlib.h"\
	".\../common\l3dslib.h"\
	".\../common\lbmlib.h"\
	".\../common\mathlib.h"\
	".\../common\scriplib.h"\
	".\../common\threads.h"\
	".\../common\trilib.h"\
	".\qdata.h"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\tables.obj" : $(SOURCE) $(DEP_CPP_TABLE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\bspfile.h

!IF  "$(CFG)" == "qdata - Win32 Release"

!ELSEIF  "$(CFG)" == "qdata - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\qfiles.h

!IF  "$(CFG)" == "qdata - Win32 Release"

!ELSEIF  "$(CFG)" == "qdata - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\bspfile.c
DEP_CPP_BSPFI=\
	"..\common\qfiles.h"\
	".\../common\bspfile.h"\
	".\../common\cmdlib.h"\
	".\../common\mathlib.h"\
	".\../common\scriplib.h"\
	

"$(INTDIR)\bspfile.obj" : $(SOURCE) $(DEP_CPP_BSPFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
