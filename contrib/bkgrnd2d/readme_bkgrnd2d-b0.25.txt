November 25 2003
bkgrnd2d v 0.25 beta for radiant 1.3.13
by SCDS_reyalP (email hellsownpuppy@yahoo.com)

WARNING:
This is an beta release. It is provided with absolutely NO WARRANTY.
If it turns your data to mush and melts your CPU, don't blame me.

Overview:
This little plugin allows you to display an image in the the gtkradiant 2d
windows. This is useful for layout sketches, maps made from
existing plans, building geometry based on photgraphs, viewing terrain
alphamaps in relation to your terrain, and so on.

Installation:
extract the .dll and bitmaps into your gtkradiant/plugins directory, and
restart radiant. Be sure to use directory names, to ensure the bitmaps go
in your plugins/bitmaps directory.

Uninstallation:
Close radiant, delete the bkgrnd2d.dll from the plugins directory, and 
delete the bkgrnd2*.bmp files from the plugins/bitmaps directory.

User Interface:
- The plugin adds 4 buttons to the radiant plugin toolbar. The first 3
  toggle the display of a background image in the specified view. The fourth
  brings up a configuration dialog. The configuration dialog can also be
  opened from the plugins menu.

- If an image has not been loaded, or it's display size and location have
  not been set, pushing one of the toggle buttons will bring up the dialog
  with the corresponding page selected.

- The configuration dialog is non-modal, meaning that you can leave it open
  while you work in radiant. If it gets lost behind another window, clicking
  on the configuration button will bring it to the forground.

Usage:
- bring up the configuration dialog.

- Choose the "Browse..." button. This will prompt you for an image file.
  The file *MUST* be inside your basegame directory (baseq3, main, etmain or
  whatever your chosen game uses). The image must be in a format supported by
  the game in use. For q3 based games this is truecolor .jpg, .tga and
  sometimes .png. For q2 this is .wal

- Use one of the following methods to set the size (in game units) that the
  file is displayed. 
  1) select 1 or more brushes or entities and choose "from selection"
     This will use the total dimensions off all selected brushes and entities
  	 to size the image. 
  2) For the X/Y view only, choose 'Size to min/max keys' This will look in
     the worldspawn entity for the keys mapcoordsmins and mapcoordsmaxs (also
  	 used for ET tracemap generation and command map sizing) and use those
   	 dimensions to size the image.

- Use the toggle buttons to show or hide the loaded images. The buttons will
  press or unpress whenever you click them, but an image will only be
  displayed once you have successfully loaded a file and set its size/postion.

- Set the opacity of the image using the slider in the configuration dialog.
  
- If any of these commands do not produce the expected results, there may be
  an information in the radiant console. Please include this when reporting
  bugs.


Notes and limitations:
- This plugin is compiled for GtkRadiant 1.3.13. It may or may not work with
  later versions. It will *NOT* work with version 1.3.12 and below. If you
  build from source (see below) you can build it for other versions.

- As mentioned above, the image *MUST* be inside your basegame directory, or 
  another directory in which radiant looks for game files.

- To prevent the image from being distorted, you should size it to the
  original images aspect ratio. mapcoordsmaxs/mapcoordsmins and command maps
  should always be square.

- If you want a specific pixel to world unit relationship, you must arrange
  that yourself.

- On load, the image is converted to a texture whose dimensions are powers
  of 2. If the original image dimensions are not powers of 2, some detail will
  be lost due to resampling. If it is too large to fit on a single texture,
  resolution is reduced.

- radiants gamma and mipmap options are applied to the image.

- If the image has an alpha channel, it will be included in the blending
  process. 0 is transparent, 255 is opaque. .tga images are recommended if
  you want to have an alpha channel.

- since the plugin will only use true color files, you cannot use a terrain
  indexmap (aka alphamap) or heightmap directly. You can of course, save a
  copy of your indexmap in a 32 bit format.

- There is no unload command.

- You put the plugin in a game specific plugin directory, rather than the
  radiant plugin directory. 

- You cannot set the image size with sub-unit precision.

- Only win32 binaries are included.  The source is available from:
  http://www.cyberonic.net/~gdevault/rfm/mapstuff/bkgrnd2d-b0.25-src.zip
  If you want to use it on another platform you will need a buildable gtkradiant
  source tree to build it. For any non-windows platform you will also have to
  figure out the compile options. I suggest ripping those off from some other
  plugin.

TODO:
- make file selection paterns match supported filetypes
- large images without downsampling
- bitmap and pcx support for indexmaps
- automatic size from indexmapped entities
- render under the grid instead of blending
- mac/*nix support
- remember/save/restore settings
- texture options independant of radiant prefs
- clean up icky code

Changes from 0.1
- all 2d views supported
- new ui
- file selection patterns, default directory improved

Changes from 0.2
- tooltips in dialog
- various code cleanup

