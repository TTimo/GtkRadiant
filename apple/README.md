GtkRadiant for Apple OSX
========================

This directory provides packaging steps for GtkRadiant for OSX. This document describes compiling the application on OSX as well as generating distributable bundles using the framework provided in this directory.

Dependencies & Compilation
--------------------------

GtkRadiant uses the native (Quartz) GTK3 from [Homebrew](https://brew.sh), so XQuartz and MacPorts are not needed:

```
brew install gtk+3 scons
```

- Get the GtkRadiant code and compile:

```
git clone https://github.com/TTimo/GtkRadiant.git
cd GtkRadiant/
scons
```

`svn` is needed to fetch the game packs; add `--no-packs` to build without them.

- Run the build:

(from the GtkRadiant/ directory)
```
./install/radiant.bin
```

The OpenGL views use a legacy (2.1) profile CGL context, the only macOS profile with the fixed-function pipeline that Radiant draws with.

Building GtkRadiant.app
-----------------------

The `Makefile` in the 'apple/' directory builds `target/GtkRadiant.app` from `../install`, bundling the Homebrew GTK3 runtime (libraries, the SVG pixbuf loader, the Adwaita icons and the GTK GSettings schemas) with `dylibbundler`:

```
brew install dylibbundler
make
make image
```

`make image` wraps the app in `target/GtkRadiant-<version>.dmg`.

- The app doesn't need Homebrew to run, but it needs the macOS release the Homebrew bottles were built for or newer; `make` writes that into `LSMinimumSystemVersion`.
- `.game` files are left out because they hold absolute paths from the build machine. On first launch, choose "Configure editor for another game" and point it at your Quake 3 folder.
- The app is only ad-hoc signed. Gatekeeper will refuse to open it on another Mac until it is signed and notarized with a Developer ID.

Getting help
------------

Get on irc: Quakenet #radiant, or ask on the mailing list, or post something on the issue tracker..
