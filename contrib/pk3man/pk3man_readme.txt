Pk3Man
------

Pk3Man is a plugin for Q3Radiant (GtkEdition).
It allows the creation and editing of Q3 Pk3 files
from within the Q3Radiant Editor.

It does NOT operate in the same way as WinZip so ill
try to documented the differences here. Please read these
before emailing me with suggestions or problems.

***************************************************************

1. Any modified or newly created Pk3 is only committed to disk,
   when you save.

2. Pk3Man uses your current base folder set in Q3Radiant (ie. BaseQ3)
   and adds files using there current path relative to this. This means
   that when you have finished your map, you just add the required files,
   and their paths are automatically correct (if they were in the correct place for
   Q3 to start with)

4. You can change the stored path of a file using the rename option. Rename
   the file with its full new relative path and name.

5. The wizzard will scan a .map file and add any textures/sounds/env maps e.t.c that
   are referenced, except those in the exclusion list.

6. files that have been added (and not saved) are shown with an appending * in the
   statusbar when selected.

7. The exclusion list applies to Pk3's and to individual files. eg 'pak?.pk3' would
   eliminate all ID pk3's, and 'textures/mickey/*.tga' would eliminate all tga's in
   a sub folder of textures called 'mickey'


************************************************************

Please email and bugs/suggestions to me here : mickey@planetquake.com


------------------------------------------------------------------------
Remember this is a beta. I hold no responsibility for any damage done to
files on your computer due to the action of this plugin.
-------------------------------------------------------------------------

