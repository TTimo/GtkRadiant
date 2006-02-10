Package: radiant
Version: 1.M4_VER_MAJOR.M4_VER_MINOR
Revision: M4_REV 
Description: GtkRadiant - level editor for Id technology games
DescDetail: <<
You need to be running fink unstable to install GtkRadiant
Installation instructions: http://www.qeradiant.com/wikifaq/index.php?Installation%20on%20OSX 
<<
DescPort: <<
Add comments about dependencies here..
<<
Maintainer: Timothee Besset <ttimo@idsoftware.com>
License: Restrictive
Homepage: http://www.qeradiant.com/

# Dependencies:
BuildDepends: gtk+2-dev, gtkglext1, libiconv-dev, scons, libxml, gtkglext1, libglade, glib, gtk+
Depends: libglade, libxml-shlibs, gettext, libglade-shlibs, gtk+-shlibs, glib-shlibs, gtk+2-shlibs, dlcompat-shlibs, libxml2-shlibs, libiconv, glib2-shlibs, libpng-shlibs, x11, libpng3-shlibs, gtkglext1-shlibs, libglade-shlibs, glib-shlibs, gtk+-shlibs

Source: http://zerowing.idsoftware.com/osx/GtkRadiant-osx-1.M4_VER_MAJOR.M4_VER_MINOR-M4_REV.tgz
Source-MD5: M4_MD5SUM

PatchScript: echo "Dummy PatchScript" 
CompileScript: sh ./build.sh 

# Install Phase:
InstallScript: <<
 mkdir -p %i/games/radiant-setup
 mkdir -p %i/bin
 cp GtkRadiant/M4_SETUP_TARGET %i/games/radiant-setup/M4_SETUP_TARGET
 ln -s /sw/games/radiant-setup/M4_SETUP_TARGET %i/bin/radiant-setup
<<
