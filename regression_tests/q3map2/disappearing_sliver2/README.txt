DESCRIPTION OF PROBLEM:
=======================

The example map, maps/disappearing_sliver2.map, contains an example of this
bug.  The triangle sliver surface in the middle of the room is not rendered
in the final BSP.

To trigger the bug, compile the map; you don't need -vis or -light.  Only
-bsp (the first q3map2 stage) is necessary to trigger the bug.  The only
entities in the map are a light and a info_player_deathmatch, so the map will
compile for any Q3 mod.


SOLUTION TO PROBLEM:
====================

None yet.  The problem is likely caused by sloppy math operations (significant
loss of precision).
