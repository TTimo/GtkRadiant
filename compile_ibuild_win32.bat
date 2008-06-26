rem silly build script for compiling with MSVC using the IncrediBuild console
rem can't figure how to use the buildbot ShellCommand to pass the correct CFG= line
rem because of the configuration name with a | in it (Release|Win32)

buildconsole.exe radiant.sln /BUILD /CFG="Release|Win32"
