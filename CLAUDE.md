# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

GtkRadiant 1.6: an OpenGL level editor for id Tech 1–3 games, plus the q3map2 BSP compiler and q3data. This is a fork of `github.com/TTimo/GtkRadiant` (`1.6-release`). The `gtk3-macos` branch ports it from GTK2/gtkglext to GTK3 so it builds natively on macOS.

## Build

The build uses SCons, driven by `SConstruct` → `config.py` → `SConscript.*`.

```sh
scons                          # release build of all targets: radiant, q3map2, q3data, setup
scons config=debug             # -g, -D_DEBUG (also enables SYNAPSE_VERBOSE module-loading logs)
scons -j8 --no-packs           # parallel build, and skip fetching game packs over svn
scons target=radiant           # editor and all plugin modules only
scons target=q3map2,q3data     # 'q3map2' builds both q3map2 and q3map2_urt
scons -c                       # clean
```

- Config statements (`target=`, `config=`, `cc=`, `cxx=`, `install_directory=`) can be mixed with scons flags. They are pickled to `site.sconf` and reloaded on the next run, so `scons config=debug` sticks until you pass `config=release` or delete `site.sconf`.
- Objects go to `build/<config>/...`. Runnable output goes to `install/`: `radiant.bin`, `modules/*.so`, `q3map2`, `q3map2_urt`, `q3data`. Run the editor with `./install/radiant.bin`.
- The `setup` target svn-checks-out the game packs into `install/installs/<Pack>/`, so it needs `svn` (use `--no-packs` to skip). On Linux, a final step copies libjpeg and libpng next to `radiant.bin`, found through `ldd`.
- Build-time dependencies: `pkg-config` entries for `libxml-2.0`, `gtk+-3.0`, `pangoft2`, `libjpeg`, `libpng` and `zlib`; on Linux also `gl`, `glu` and `egl`. On macOS that is `brew install gtk+3 scons` (Quartz GTK3, no XQuartz), and GL comes from `OpenGL.framework`. `cd apple && make` builds a self-contained `apple/target/GtkRadiant.app` from `install/`. It needs `brew install dylibbundler`, bundles the Homebrew GTK3 libraries, the SVG pixbuf loader, the Adwaita icons and the GTK schemas, and runs through the `Contents/MacOS/radiant` launcher script, which points GTK at that runtime. The Makefile copies with `rsync`, because if the source tree was downloaded it carries a quarantine flag, and `cp` would put that flag on the app. It also refuses Homebrew paths, duplicate rpaths and quarantined files in the output.
- macOS is the only platform the GTK3 port has been built and run on. Upstream ships Linux as a Flatpak (manifest on Flathub), and neither it nor the EGL path has been tried with GTK3. `radiant.sln` (Windows) still targets GTK2, so Windows doesn't build.
- `install.py` is a Python 2 script for the old Windows build only.

### Source lists come from the .vcxproj files

SCons does not glob source files. `utils.vcxproj` parses each project's `.vcxproj` and compiles every `<ClCompile Include=...>` entry. **To add or remove a source file, edit the matching `.vcxproj`** (and its `.vcxproj.filters` for VS), or SCons will not see the change. The list of projects built for each target, and the static libs each plugin module links, are hard-coded in `config.py` (`emit_radiant`, `emit_q3map2`, `emit_q3data`). A new module or a new lib dependency goes there.

On Linux, every module `.so` goes through `utils.CheckUnresolved` (`ldd -r`), which **deletes the module if it has undefined symbols**. A link error in a plugin can therefore show up as a "missing" module.

## Tests

There are no unit tests. `regression_tests/q3map2/<case>/` holds hand-run q3map2 repro cases, each with `maps/`, `textures/`, and a `README.txt` that describes the bug and how to reproduce it. Run them by compiling the map with the built `install/q3map2`, for example `install/q3map2 -fs_basepath <dir containing baseq3> -game quake3 -meta <map>`.

The editor won't start without a game: it needs `install/games/<game>.game` and the pack it points at (`installs/<Pack>/game/synapse.config`, entity defs, and `baseq3/scripts/default_project.proj` under the engine path). Normally the `setup` target fetches the packs. Prefs and `radiant.log` live in `$XDG_STATE_HOME/.radiant/<version>/`, falling back to `~/.radiant/<version>/`; `.game` files go to `$XDG_DATA_HOME/games/` if that is set. A version bump (`include/version.h`) starts with empty prefs. Release builds prompt at startup when console logging is on; debug builds don't.

## Architecture

- **`radiant/`**: the editor executable (C++, GTK3). The code is an MFC-era port, with globals and types in `qe3.h`, `stdafx.h`, and `qedefs.h`. Key areas:
  - `mainframe.cpp`: the window and its commands.
  - `xywindow`, `camwindow`, `zwindow`, `texwindow`: the views.
  - `brush*.cpp`, `pmesh.cpp`, `select.cpp`, `undo.cpp`, `map.cpp`: editing.
  - `preferences.cpp`: preferences, game selection, and the first-run game-pack installer.
  - `pluginmanager.cpp`: module loading.
- **GL views** (`glwidget.cpp`, `glwindow.cpp`): each view is a `GtkDrawingArea` with its own offscreen compatibility-profile context (CGL legacy profile on macOS, EGL on Linux). All contexts share objects with one root context, as gtkglext's share list did, but GL state stays per view. A view renders into an FBO in `gtk_glwidget_make_current()`, and `gtk_glwidget_swap_buffers()` reads it back into a cairo surface that is painted in the `"draw"` signal. Hook view rendering to `"draw"`, because GTK3 has no `expose-event`. A swap outside a draw just queues one. The old GDK_XOR rubber band is now an overlay set with `gtk_glwidget_set_xor_rectangle()`. Radiant's own GL calls go through the `qgl*` pointers that `qgl.c` loads with `dlopen` (`OpenGL.framework` on macOS). On high-dpi screens the FBO has the display's resolution. `gtk_glwidget_scale_gl_functions()` (installed right after `QGL_Init`, before plugins copy the `qgl*` pointers) scales `glViewport`, `glScissor`, `glLineWidth`, `glPointSize` and `glLineStipple` by the current view's scale factor, and unscales `GL_VIEWPORT` queries. GL text is rendered with a font for that scale. Radiant code keeps working in window coordinates; only something that reads pixels back, like `WXY_Print`, has to multiply by `gtk_widget_get_scale_factor()`.
- **`libs/synapse/`**: the module system. Each module shared library exports `Synapse_EnumerateInterfaces()`, which returns a `CSynapseClient`. The client declares APIs as `(major, minor)` string pairs with `AddAPI(..., SYN_PROVIDE | SYN_REQUIRE | SYN_REQUIRE_ANY)` and fills function tables in `RequestAPI()`. See `plugins/image/image.cpp` for a compact example. The server in radiant scans `install/modules/` and `install/plugins/`, then resolves the dependency graph using the active game's `synapse.config`. That config file picks which minor is used for each game, for example the VFS (pk3/pak/wad), the map format, or the shader system.
- **`include/i*.h`, `qerplugin.h`**: the plugin API. Each header defines a `*_MAJOR` name and a C struct of function pointers (`_QERFuncTable_1` is the core radiant table). Changing a table's layout breaks every module that uses it, because the size is checked at `AddAPI`.
- **`plugins/`**: core modules that ship with the editor: VFS backends, image loaders, model and sprite loaders, the entity module, the shader parser, map I/O (`map`, `mapxml`), and the surface inspector (`surface`, `surface_idtech2`). `plugins/sample` is out of date and is not built.
- **`contrib/`**: optional toolbar and menu plugins (bobtoolz, gtkgensurf, prtview, camera, bkgrnd2d, hydratoolz), built as modules the same way.
- **`libs/`**: static helper libs (cmdlib, mathlib, l_net, picomodel, splines, ddslib, md5lib), each with its own `.vcxproj`. The modules use PIC copies of these libs (`build/<config>/shobjs`).
- **`tools/quake3/q3map2/`**: the BSP, vis, and light compiler (C). Per-game BSP and surface-flag definitions are in `game_*.h`, and are registered in `q3map2.h`. It shares `tools/quake3/common/` (vfs, polylib, image I/O) with q3data. `tools/urt/` is a separate q3map2 fork for Urban Terror that reuses `common/` instead of its own copies. `tools/quake2/` is VS-only and is not built by SCons.

### Game support

Games are described by `install/games/*.game` XML files. The first-run `CGameInstall` dialog in `preferences.cpp` writes these files, pointing the tools path at `install/installs/<Pack>/game/`, which holds `synapse.config`, entity defs, and shaders. Game-specific behavior in the editor is mostly string comparisons on `g_pGameDescription->mGameFile` (such as `"q3.game"` or `"quetoo.game"`), spread across `radiant/*.cpp`. Grep for the game file name when changing behavior for one game.

## Conventions

- Format with uncrustify using `uncrustify.cfg`: tabs for indentation, spaces inside parentheses (`foo( a, b )`, `if ( x ) {`), and opening braces on the same line.
- GTK3-deprecated widgets (GtkTable, GtkHBox/VBox and so on) are still used and compile with `-Wno-deprecated-declarations`. Removed GTK2 API (direct struct fields such as `widget->allocation`, GdkGC, GtkColorSelectionDialog fields, `gtk_widget_draw(w, NULL)`) has to be replaced with GTK3 accessors.
- Use radiant's own containers and strings (`Str` in `libs/str.h`, `gtkr_list.h`, `gtkr_vector.h`). `Q_NO_STLPORT` is always defined.
- User-visible strings are wrapped in `_()` for gettext. `TRANSLATING` explains how to regenerate `radiant.pot`.
