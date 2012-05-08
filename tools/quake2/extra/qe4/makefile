
TARGETOS=WINNT

!include <ntwin32.mak>

# This line allows NMAKE to work as well

all: gengl.exe

# Update the object file if necessary

gengl.obj: gengl.c gengl.h
    $(cc) $(cflags) $(cvars) $(cdebug) $(cf) gengl.c

render.obj: render.c gengl.h
    $(cc) $(cflags) $(cvars) $(cdebug) $(cf) render.c

gengl.res: gengl.rc genglrc.h
    rc -r gengl.rc

gengl.exe: gengl.obj gengl.res render.obj 
    $(link) $(linkdebug) /NODEFAULTLIB $(guilflags) -out:gengl.exe \
    gengl.obj render.obj gengl.res $(guilibsdll) opengl32.lib glu32.lib
