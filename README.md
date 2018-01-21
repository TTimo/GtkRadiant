GtkRadiant
==========

![logo](https://icculus.org/gtkradiant/images/logo-radiant.png)

GtkRadiant is an open-source, cross-platform level editor for id Tech based games. It comes with some map compilers and data authoring tools.

Downloads
---------

Ready-to-use GtkRadiant packages are available on the [Downloads page](http://icculus.org/gtkradiant/downloads.html) on GtkRadiant's website. Some [installation instruction](https://icculus.org/gtkradiant/installation.html) may be useful.

Useful links
------------

- [GtkRadiant website](https://icculus.org/gtkradiant/)
- [Documentation](https://icculus.org/gtkradiant/documentation.html)

Supported games
---------------

GtkRadiant provides level editing support for [Quake](https://en.wikipedia.org/wiki/Quake_(video_game)), [Quake2](https://en.wikipedia.org/wiki/Quake_II), [Quake III Arena](https://ioquake3.org), [QuakeLive](https://www.quakelive.com), [Quetoo](http://quetoo.org), [Return to Castle Wolfenstein](https://en.wikipedia.org/wiki/Return_to_Castle_Wolfenstein), [Star Trek Voyager: Elite Force](https://en.wikipedia.org/wiki/Star_Trek:_Voyager_–_Elite_Force), [Star Wars Jedi Knight: Jedi Academy](https://en.wikipedia.org/wiki/Star_Wars_Jedi_Knight:_Jedi_Academy), [Unvanquished](https://www.unvanquished.net), [Urban Terror](http://urbanterror.info), [Wolfenstein: Enemy Territory](http://www.splashdamage.com/content/wolfenstein-enemy-territory-barracks).

How to build
------------

You can find more complete instructions to build on Windows [there](https://icculus.org/gtkradiant/documentation/windows_compile_guide/) and to build on Mac OS [there](apple/README.md).

```sh
# get the source
git clone "https://github.com/TTimo/GtkRadiant.git"

# enter the source tree
cd GtkRadiant

# build everything
scons
```

You can build a specific part like this:

```sh
# only build the GtkRadiant level editor
scons target="radiant"

# only build the q3map2 map compiler and the q3data tool
scons target="q3map2,q3data"
```

Level editor binary (`radiant`) and tools (like `q3map2`) will be found in `install/` directory. 
The build process automatically fetches gamepacks.

Getting in touch
----------------

The [#radiant channel at QuakeNet](https://webchat.quakenet.org/?channels=radiant) is the official GtkRadiant IRC channel. Come and chat about level design, development or bugs, you're welcome. Bugs can be submitted on the [GitHub issue tracker](https://github.com/TTimo/GtkRadiant/issues).

Legal
-----

GtkRadiant source code is copyrighted by [id Software, Inc](http://idsoftware.com/) and various contributors and protected by the [General Public License v2](GPL).
