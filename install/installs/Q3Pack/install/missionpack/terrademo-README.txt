Terrain Map Demo for Quake III Arena (ver >= 1.27g)
All included media (c) Id Software
Archive built by qeradiant.com,
 thanks for the help to Fubar - Fubar_@btinternet.com
version 2 - 1/9/2001
=====================================================

version 2 update: new q3map (patchlevel 2) included in this archive, solves the 
shader blending bug on terrain maps.

This archive contains a complete set of shaders, textures, models needed to 
build a small terrain map that will run with a Quake III Arena installation 
(version >= 1.27g)

So far terrain maps have only been seen in Quake III: Team Arena, but the 
terrain capabilities are indeed in the Quake III engine and will soon be used in 
mods and maps. The aim of this archive is to provide a starting point to do 
terrain maps if you don't have Quake III: Team Arena. If you have Quake III: 
Team Arena installed and GtkRadiant configured for it, you can use 
missionpack/maps/terrademo.map

If you are going to write your own terrain maps, you will need to redistribute 
some of the shaders in your pk3. Keep in mind that a default Quake III Arena 
installation will not have common/terrain for example.

We also include an update to the tree mapobjects, the shaders were missing. If 
you use the trees in your maps you need to redistribute those shaders too.

There is an update to q3map included, if q3map fails to load the alphamap for a 
terrain entity, it will exit cleanly instead of crashing.

Basic setup:

1) Unzip the archive to your baseq3/ directory, move the new q3map where belongs

2) In order to compile the sample map correctly you need to add a line with the 
new .shader file name in your shaderlist.txt: add "terrademoQ3" at the end the 
file. If you don't do that, q3map will simply not see those new shaders.

3) You can now open terrademoQ3.map and compile it