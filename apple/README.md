GtkRadiant for Apple OSX
========================

This directory provides packaging steps for GtkRadiant for OSX. This document describes compiling the application on OSX as well as generating distributable bundles using the framework provided in this directory.

Dependencies & Compilation
--------------------------

Directions for OSX Mavericks 10.9 - your mileage may vary:

- Install [MacPorts](http://macports.org).
- Install [XQuartz](http://xquartz.macosforge.org/)

- Install dependencies with MacPorts:

```
sudo port install dylibbundler pkgconfig gtkglext scons
```

- Get the GtkRadiant code and compile:

```
git clone https://github.com/TTimo/GtkRadiant.git
cd GtkRadiant/
scons
```

- Run the build:

(from the GtkRadiant/ directory)
```
./install/radiant.bin
```

XQuartz note: on my configuration XQuartz doesn't automatically start for some reason. I have to open another terminal, and run the following command: `/Applications/Utilities/XQuartz.app/Contents/MacOS/X11.bin`, then start radiant. 
    
Building GtkRadiant.app
-----------------------

The `Makefile` in the 'apple/' directory will produce a distributable .app bundle for GtkRadiant using `dylibbundler`:

```
make
make image
```

Getting help
------------

Get on irc: Quakenet #radiant, or ask on the mailing list, or post something on the issue tracker..
