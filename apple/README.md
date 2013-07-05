GtkRadiant for Apple OSX
====
This directory provides packaging steps for GtkRadiant for OSX. This document describes compiling the application on OSX as well as generating distributable bundles using the framework provided in this directory.

Dependencies & Compilation
----
Compiling GtkRadiant on OSX requires [MacPorts](http://macports.org). Homebrew is not yet supported. Install the following dependencies:

    sudo port install dylibbundler gcc47 gtkglext scons

You should then be able to compile and run GtkRadiant from the `install` directory.

    scons config=debug
    ./install/radiant.bin &

See the [GtkRadiant developers guide]() for details on build configurations and targets.
    
Building GtkRadiant.app
----
The `Makefile` in this directory will produce a distributable .app bundle for GtkRadiant using `dylibbundler`:

    make
    make image

You may encounter an error or two as `dylibbundler` attempts to fix up shared libraries that GtkRadiant depends on. This is because `dylibbundler` attempts to rewrite the load commands in these libraries, but not all libraries have enough space allocated for this purpose. Recompile the relevant ports with additional header space.

For example:

    export LDFLAGS=-headerpad_max_install_names
    sudo port clean --work --archive pango 
    sudo port -n upgrade --force pango

Repeat this process for any shared libraries which `dylibbundler` complains about. As of July 2nd 2013, Pango is the only port requiring this treatment.
