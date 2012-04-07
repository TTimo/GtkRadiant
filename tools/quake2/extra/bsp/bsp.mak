# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=bspinfo3 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to bspinfo3 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "bsp - Win32 Release" && "$(CFG)" != "bsp - Win32 Debug" &&\
 "$(CFG)" != "qbsp3 - Win32 Release" && "$(CFG)" != "qbsp3 - Win32 Debug" &&\
 "$(CFG)" != "qvis3 - Win32 Release" && "$(CFG)" != "qvis3 - Win32 Debug" &&\
 "$(CFG)" != "qrad3 - Win32 Release" && "$(CFG)" != "qrad3 - Win32 Debug" &&\
 "$(CFG)" != "bspinfo3 - Win32 Release" && "$(CFG)" != "bspinfo3 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "bsp.mak" CFG="bspinfo3 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bsp - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "bsp - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "qbsp3 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "qbsp3 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "qvis3 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "qvis3 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "qrad3 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "qrad3 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "bspinfo3 - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "bspinfo3 - Win32 Debug" (based on "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "bspinfo3 - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bsp - Win32 Release"

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

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D\
 "_CONSOLE" /Fp"$(INTDIR)/bsp.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/bsp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(OUTDIR)/bsp.pdb" /machine:I386\
 /out:"$(OUTDIR)/bsp.exe" 
LINK32_OBJS= \
	

!ELSEIF  "$(CFG)" == "bsp - Win32 Debug"

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

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG"\
 /D "_CONSOLE" /Fp"$(INTDIR)/bsp.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/bsp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/bsp.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/bsp.exe" 
LINK32_OBJS= \
	

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "qbsp3\Release"
# PROP BASE Intermediate_Dir "qbsp3\Release"
# PROP BASE Target_Dir "qbsp3"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "qbsp3\Release"
# PROP Intermediate_Dir "qbsp3\Release"
# PROP Target_Dir "qbsp3"
OUTDIR=.\qbsp3\Release
INTDIR=.\qbsp3\Release

ALL : "$(OUTDIR)\qbsp3.exe"

CLEAN : 
	-@erase "$(INTDIR)\brushbsp.obj"
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\csg.obj"
	-@erase "$(INTDIR)\faces.obj"
	-@erase "$(INTDIR)\gldraw.obj"
	-@erase "$(INTDIR)\glfile.obj"
	-@erase "$(INTDIR)\lbmlib.obj"
	-@erase "$(INTDIR)\leakfile.obj"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\polylib.obj"
	-@erase "$(INTDIR)\portals.obj"
	-@erase "$(INTDIR)\prtfile.obj"
	-@erase "$(INTDIR)\qbsp3.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(INTDIR)\textures.obj"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\tree.obj"
	-@erase "$(INTDIR)\writebsp.obj"
	-@erase "$(OUTDIR)\qbsp3.exe"
	-@erase "$(OUTDIR)\qbsp3.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /GX /Zd /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /Zd /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D\
 "_CONSOLE" /Fp"$(INTDIR)/qbsp3.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\qbsp3\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qbsp3.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(OUTDIR)/qbsp3.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/qbsp3.exe" 
LINK32_OBJS= \
	"$(INTDIR)\brushbsp.obj" \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\csg.obj" \
	"$(INTDIR)\faces.obj" \
	"$(INTDIR)\gldraw.obj" \
	"$(INTDIR)\glfile.obj" \
	"$(INTDIR)\lbmlib.obj" \
	"$(INTDIR)\leakfile.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\polylib.obj" \
	"$(INTDIR)\portals.obj" \
	"$(INTDIR)\prtfile.obj" \
	"$(INTDIR)\qbsp3.obj" \
	"$(INTDIR)\scriplib.obj" \
	"$(INTDIR)\textures.obj" \
	"$(INTDIR)\threads.obj" \
	"$(INTDIR)\tree.obj" \
	"$(INTDIR)\writebsp.obj"

"$(OUTDIR)\qbsp3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "qbsp3\Debug"
# PROP BASE Intermediate_Dir "qbsp3\Debug"
# PROP BASE Target_Dir "qbsp3"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "qbsp3\Debug"
# PROP Intermediate_Dir "qbsp3\Debug"
# PROP Target_Dir "qbsp3"
OUTDIR=.\qbsp3\Debug
INTDIR=.\qbsp3\Debug

ALL : "$(OUTDIR)\qbsp3.exe"

CLEAN : 
	-@erase "$(INTDIR)\brushbsp.obj"
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\csg.obj"
	-@erase "$(INTDIR)\faces.obj"
	-@erase "$(INTDIR)\gldraw.obj"
	-@erase "$(INTDIR)\glfile.obj"
	-@erase "$(INTDIR)\lbmlib.obj"
	-@erase "$(INTDIR)\leakfile.obj"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\polylib.obj"
	-@erase "$(INTDIR)\portals.obj"
	-@erase "$(INTDIR)\prtfile.obj"
	-@erase "$(INTDIR)\qbsp3.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(INTDIR)\textures.obj"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\tree.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\writebsp.obj"
	-@erase "$(OUTDIR)\qbsp3.exe"
	-@erase "$(OUTDIR)\qbsp3.ilk"
	-@erase "$(OUTDIR)\qbsp3.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG"\
 /D "_CONSOLE" /Fp"$(INTDIR)/qbsp3.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\qbsp3\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qbsp3.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/qbsp3.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/qbsp3.exe" 
LINK32_OBJS= \
	"$(INTDIR)\brushbsp.obj" \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\csg.obj" \
	"$(INTDIR)\faces.obj" \
	"$(INTDIR)\gldraw.obj" \
	"$(INTDIR)\glfile.obj" \
	"$(INTDIR)\lbmlib.obj" \
	"$(INTDIR)\leakfile.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\polylib.obj" \
	"$(INTDIR)\portals.obj" \
	"$(INTDIR)\prtfile.obj" \
	"$(INTDIR)\qbsp3.obj" \
	"$(INTDIR)\scriplib.obj" \
	"$(INTDIR)\textures.obj" \
	"$(INTDIR)\threads.obj" \
	"$(INTDIR)\tree.obj" \
	"$(INTDIR)\writebsp.obj"

"$(OUTDIR)\qbsp3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qvis3 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "qvis3\Release"
# PROP BASE Intermediate_Dir "qvis3\Release"
# PROP BASE Target_Dir "qvis3"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "qvis3\Release"
# PROP Intermediate_Dir "qvis3\Release"
# PROP Target_Dir "qvis3"
OUTDIR=.\qvis3\Release
INTDIR=.\qvis3\Release

ALL : "$(OUTDIR)\qvis3.exe"

CLEAN : 
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\flow.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\qvis3.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(OUTDIR)\qvis3.exe"
	-@erase "$(OUTDIR)\qvis3.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /GX /Zd /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /Zd /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D\
 "_CONSOLE" /Fp"$(INTDIR)/qvis3.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\qvis3\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qvis3.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(OUTDIR)/qvis3.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/qvis3.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\flow.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\qvis3.obj" \
	"$(INTDIR)\scriplib.obj" \
	"$(INTDIR)\threads.obj"

"$(OUTDIR)\qvis3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qvis3 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "qvis3\Debug"
# PROP BASE Intermediate_Dir "qvis3\Debug"
# PROP BASE Target_Dir "qvis3"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "qvis3\Debug"
# PROP Intermediate_Dir "qvis3\Debug"
# PROP Target_Dir "qvis3"
OUTDIR=.\qvis3\Debug
INTDIR=.\qvis3\Debug

ALL : "$(OUTDIR)\qvis3.exe"

CLEAN : 
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\flow.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\qvis3.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\qvis3.exe"
	-@erase "$(OUTDIR)\qvis3.ilk"
	-@erase "$(OUTDIR)\qvis3.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG"\
 /D "_CONSOLE" /Fp"$(INTDIR)/qvis3.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\qvis3\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qvis3.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/qvis3.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/qvis3.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\flow.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\qvis3.obj" \
	"$(INTDIR)\scriplib.obj" \
	"$(INTDIR)\threads.obj"

"$(OUTDIR)\qvis3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qrad3 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "qrad3\Release"
# PROP BASE Intermediate_Dir "qrad3\Release"
# PROP BASE Target_Dir "qrad3"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "qrad3\Release"
# PROP Intermediate_Dir "qrad3\Release"
# PROP Target_Dir "qrad3"
OUTDIR=.\qrad3\Release
INTDIR=.\qrad3\Release

ALL : "$(OUTDIR)\qrad3.exe"

CLEAN : 
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\lbmlib.obj"
	-@erase "$(INTDIR)\lightmap.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\patches.obj"
	-@erase "$(INTDIR)\polylib.obj"
	-@erase "$(INTDIR)\qrad3.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\trace.obj"
	-@erase "$(OUTDIR)\qrad3.exe"
	-@erase "$(OUTDIR)\qrad3.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /GX /Zd /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /Zd /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D\
 "_CONSOLE" /Fp"$(INTDIR)/qrad3.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\qrad3\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qrad3.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(OUTDIR)/qrad3.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/qrad3.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\lbmlib.obj" \
	"$(INTDIR)\lightmap.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\patches.obj" \
	"$(INTDIR)\polylib.obj" \
	"$(INTDIR)\qrad3.obj" \
	"$(INTDIR)\scriplib.obj" \
	"$(INTDIR)\threads.obj" \
	"$(INTDIR)\trace.obj"

"$(OUTDIR)\qrad3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qrad3 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "qrad3\Debug"
# PROP BASE Intermediate_Dir "qrad3\Debug"
# PROP BASE Target_Dir "qrad3"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "qrad3\Debug"
# PROP Intermediate_Dir "qrad3\Debug"
# PROP Target_Dir "qrad3"
OUTDIR=.\qrad3\Debug
INTDIR=.\qrad3\Debug

ALL : "$(OUTDIR)\qrad3.exe"

CLEAN : 
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\lbmlib.obj"
	-@erase "$(INTDIR)\lightmap.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\patches.obj"
	-@erase "$(INTDIR)\polylib.obj"
	-@erase "$(INTDIR)\qrad3.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\trace.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\qrad3.exe"
	-@erase "$(OUTDIR)\qrad3.ilk"
	-@erase "$(OUTDIR)\qrad3.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG"\
 /D "_CONSOLE" /Fp"$(INTDIR)/qrad3.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\qrad3\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qrad3.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/qrad3.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/qrad3.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\lbmlib.obj" \
	"$(INTDIR)\lightmap.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\patches.obj" \
	"$(INTDIR)\polylib.obj" \
	"$(INTDIR)\qrad3.obj" \
	"$(INTDIR)\scriplib.obj" \
	"$(INTDIR)\threads.obj" \
	"$(INTDIR)\trace.obj"

"$(OUTDIR)\qrad3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "bspinfo3 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "bspinfo3\Release"
# PROP BASE Intermediate_Dir "bspinfo3\Release"
# PROP BASE Target_Dir "bspinfo3"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bspinfo3\Release"
# PROP Intermediate_Dir "bspinfo3\Release"
# PROP Target_Dir "bspinfo3"
OUTDIR=.\bspinfo3\Release
INTDIR=.\bspinfo3\Release

ALL : "$(OUTDIR)\bspinfo3.exe"

CLEAN : 
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\bspinfo3.obj"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(OUTDIR)\bspinfo3.exe"
	-@erase "$(OUTDIR)\bspinfo3.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /GX /Zd /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /Zd /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D\
 "_CONSOLE" /Fp"$(INTDIR)/bspinfo3.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\bspinfo3\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/bspinfo3.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(OUTDIR)/bspinfo3.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/bspinfo3.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\bspinfo3.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\scriplib.obj"

"$(OUTDIR)\bspinfo3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "bspinfo3 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bspinfo3\Debug"
# PROP BASE Intermediate_Dir "bspinfo3\Debug"
# PROP BASE Target_Dir "bspinfo3"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bspinfo3\Debug"
# PROP Intermediate_Dir "bspinfo3\Debug"
# PROP Target_Dir "bspinfo3"
OUTDIR=.\bspinfo3\Debug
INTDIR=.\bspinfo3\Debug

ALL : "$(OUTDIR)\bspinfo3.exe"

CLEAN : 
	-@erase "$(INTDIR)\bspfile.obj"
	-@erase "$(INTDIR)\bspinfo3.obj"
	-@erase "$(INTDIR)\cmdlib.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\scriplib.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\bspinfo3.exe"
	-@erase "$(OUTDIR)\bspinfo3.ilk"
	-@erase "$(OUTDIR)\bspinfo3.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MT /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG"\
 /D "_CONSOLE" /Fp"$(INTDIR)/bspinfo3.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/"\
 /c 
CPP_OBJS=.\bspinfo3\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/bspinfo3.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib opengl32.lib glaux.lib glu32.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/bspinfo3.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/bspinfo3.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bspfile.obj" \
	"$(INTDIR)\bspinfo3.obj" \
	"$(INTDIR)\cmdlib.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\scriplib.obj"

"$(OUTDIR)\bspinfo3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "bsp - Win32 Release"
# Name "bsp - Win32 Debug"

!IF  "$(CFG)" == "bsp - Win32 Release"

!ELSEIF  "$(CFG)" == "bsp - Win32 Debug"

!ENDIF 

# End Target
################################################################################
# Begin Target

# Name "qbsp3 - Win32 Release"
# Name "qbsp3 - Win32 Debug"

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\qbsp3\writebsp.c

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

DEP_CPP_WRITE=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\writebsp.obj" : $(SOURCE) $(DEP_CPP_WRITE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

DEP_CPP_WRITE=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\writebsp.obj" : $(SOURCE) $(DEP_CPP_WRITE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\brushbsp.c
DEP_CPP_BRUSH=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\brushbsp.obj" : $(SOURCE) $(DEP_CPP_BRUSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\csg.c
DEP_CPP_CSG_C=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\csg.obj" : $(SOURCE) $(DEP_CPP_CSG_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\faces.c
DEP_CPP_FACES=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\faces.obj" : $(SOURCE) $(DEP_CPP_FACES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\gldraw.c

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

DEP_CPP_GLDRA=\
	".\qbsp3\qbsp.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\gldraw.obj" : $(SOURCE) $(DEP_CPP_GLDRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

DEP_CPP_GLDRA=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLAUX.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	

"$(INTDIR)\gldraw.obj" : $(SOURCE) $(DEP_CPP_GLDRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\glfile.c

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

DEP_CPP_GLFIL=\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\glfile.obj" : $(SOURCE) $(DEP_CPP_GLFIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

DEP_CPP_GLFIL=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\glfile.obj" : $(SOURCE) $(DEP_CPP_GLFIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\leakfile.c

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

DEP_CPP_LEAKF=\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\leakfile.obj" : $(SOURCE) $(DEP_CPP_LEAKF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

DEP_CPP_LEAKF=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\leakfile.obj" : $(SOURCE) $(DEP_CPP_LEAKF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\map.c
DEP_CPP_MAP_C=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\portals.c
DEP_CPP_PORTA=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\portals.obj" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\prtfile.c
DEP_CPP_PRTFI=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\prtfile.obj" : $(SOURCE) $(DEP_CPP_PRTFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\qbsp.h

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\qbsp3.c
DEP_CPP_QBSP3=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\qbsp3.obj" : $(SOURCE) $(DEP_CPP_QBSP3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\textures.c
DEP_CPP_TEXTU=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\textures.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qbsp3\tree.c
DEP_CPP_TREE_=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	"..\common\threads.h"\
	".\qbsp3\qbsp.h"\
	

"$(INTDIR)\tree.obj" : $(SOURCE) $(DEP_CPP_TREE_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\threads.c
DEP_CPP_THREA=\
	"..\common\cmdlib.h"\
	"..\common\threads.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\threads.obj" : $(SOURCE) $(DEP_CPP_THREA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\cmdlib.c
DEP_CPP_CMDLI=\
	"..\common\cmdlib.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\cmdlib.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\lbmlib.c
DEP_CPP_LBMLI=\
	"..\common\cmdlib.h"\
	"..\common\lbmlib.h"\
	

"$(INTDIR)\lbmlib.obj" : $(SOURCE) $(DEP_CPP_LBMLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\mathlib.c
DEP_CPP_MATHL=\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	

"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\polylib.c
DEP_CPP_POLYL=\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	

"$(INTDIR)\polylib.obj" : $(SOURCE) $(DEP_CPP_POLYL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\scriplib.c
DEP_CPP_SCRIP=\
	"..\common\cmdlib.h"\
	"..\common\scriplib.h"\
	

"$(INTDIR)\scriplib.obj" : $(SOURCE) $(DEP_CPP_SCRIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\bspfile.c
DEP_CPP_BSPFI=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	

"$(INTDIR)\bspfile.obj" : $(SOURCE) $(DEP_CPP_BSPFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\threads.h

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\cmdlib.h

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\lbmlib.h

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\mathlib.h

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\polylib.h

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\scriplib.h

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\bspfile.h

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\qfiles.h

!IF  "$(CFG)" == "qbsp3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qbsp3 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "qvis3 - Win32 Release"
# Name "qvis3 - Win32 Debug"

!IF  "$(CFG)" == "qvis3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qvis3 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\qvis3\vis.h

!IF  "$(CFG)" == "qvis3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qvis3 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qvis3\qvis3.c
DEP_CPP_QVIS3=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\qfiles.h"\
	"..\common\threads.h"\
	".\qvis3\vis.h"\
	

"$(INTDIR)\qvis3.obj" : $(SOURCE) $(DEP_CPP_QVIS3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qvis3\flow.c
DEP_CPP_FLOW_=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\qfiles.h"\
	".\qvis3\vis.h"\
	

"$(INTDIR)\flow.obj" : $(SOURCE) $(DEP_CPP_FLOW_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\cmdlib.c
DEP_CPP_CMDLI=\
	"..\common\cmdlib.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\cmdlib.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\mathlib.c
DEP_CPP_MATHL=\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	

"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\scriplib.c
DEP_CPP_SCRIP=\
	"..\common\cmdlib.h"\
	"..\common\scriplib.h"\
	

"$(INTDIR)\scriplib.obj" : $(SOURCE) $(DEP_CPP_SCRIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\threads.c
DEP_CPP_THREA=\
	"..\common\cmdlib.h"\
	"..\common\threads.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\threads.obj" : $(SOURCE) $(DEP_CPP_THREA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\bspfile.c
DEP_CPP_BSPFI=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	

"$(INTDIR)\bspfile.obj" : $(SOURCE) $(DEP_CPP_BSPFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
################################################################################
# Begin Target

# Name "qrad3 - Win32 Release"
# Name "qrad3 - Win32 Debug"

!IF  "$(CFG)" == "qrad3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qrad3 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\qrad3\patches.c
DEP_CPP_PATCH=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\lbmlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\threads.h"\
	".\qrad3\qrad.h"\
	

"$(INTDIR)\patches.obj" : $(SOURCE) $(DEP_CPP_PATCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qrad3\lightmap.c
DEP_CPP_LIGHT=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\lbmlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\threads.h"\
	".\qrad3\qrad.h"\
	

"$(INTDIR)\lightmap.obj" : $(SOURCE) $(DEP_CPP_LIGHT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qrad3\qrad.h

!IF  "$(CFG)" == "qrad3 - Win32 Release"

!ELSEIF  "$(CFG)" == "qrad3 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qrad3\qrad3.c
DEP_CPP_QRAD3=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\lbmlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	"..\common\qfiles.h"\
	"..\common\threads.h"\
	".\qrad3\qrad.h"\
	

"$(INTDIR)\qrad3.obj" : $(SOURCE) $(DEP_CPP_QRAD3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\qrad3\trace.c
DEP_CPP_TRACE=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\qfiles.h"\
	

"$(INTDIR)\trace.obj" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\threads.c
DEP_CPP_THREA=\
	"..\common\cmdlib.h"\
	"..\common\threads.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\threads.obj" : $(SOURCE) $(DEP_CPP_THREA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\cmdlib.c
DEP_CPP_CMDLI=\
	"..\common\cmdlib.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\cmdlib.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\mathlib.c
DEP_CPP_MATHL=\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	

"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\polylib.c
DEP_CPP_POLYL=\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\polylib.h"\
	

"$(INTDIR)\polylib.obj" : $(SOURCE) $(DEP_CPP_POLYL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\scriplib.c
DEP_CPP_SCRIP=\
	"..\common\cmdlib.h"\
	"..\common\scriplib.h"\
	

"$(INTDIR)\scriplib.obj" : $(SOURCE) $(DEP_CPP_SCRIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\bspfile.c
DEP_CPP_BSPFI=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	

"$(INTDIR)\bspfile.obj" : $(SOURCE) $(DEP_CPP_BSPFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\lbmlib.c
DEP_CPP_LBMLI=\
	"..\common\cmdlib.h"\
	"..\common\lbmlib.h"\
	

"$(INTDIR)\lbmlib.obj" : $(SOURCE) $(DEP_CPP_LBMLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
################################################################################
# Begin Target

# Name "bspinfo3 - Win32 Release"
# Name "bspinfo3 - Win32 Debug"

!IF  "$(CFG)" == "bspinfo3 - Win32 Release"

!ELSEIF  "$(CFG)" == "bspinfo3 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\bspinfo3\bspinfo3.c
DEP_CPP_BSPIN=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\qfiles.h"\
	

"$(INTDIR)\bspinfo3.obj" : $(SOURCE) $(DEP_CPP_BSPIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\mathlib.c
DEP_CPP_MATHL=\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	

"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\cmdlib.c
DEP_CPP_CMDLI=\
	"..\common\cmdlib.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\cmdlib.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\bspfile.c
DEP_CPP_BSPFI=\
	"..\common\bspfile.h"\
	"..\common\cmdlib.h"\
	"..\common\mathlib.h"\
	"..\common\qfiles.h"\
	"..\common\scriplib.h"\
	

"$(INTDIR)\bspfile.obj" : $(SOURCE) $(DEP_CPP_BSPFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\common\scriplib.c
DEP_CPP_SCRIP=\
	"..\common\cmdlib.h"\
	"..\common\scriplib.h"\
	

"$(INTDIR)\scriplib.obj" : $(SOURCE) $(DEP_CPP_SCRIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
