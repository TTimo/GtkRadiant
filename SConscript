import os, sys, commands, string
from makeversion import get_version
# OS Detection:
OS = commands.getoutput('uname')

Import('GLOBALS')
Import(GLOBALS)

def build_list(s_prefix, s_string):
  s_list = Split(s_string)
  for i in range(len(s_list)):
    s_list[i] = s_prefix + '/' + s_list[i]
  return s_list

# common code ------------------------------------------------------

cmdlib_lib = g_env.StaticLibrary(target='libs/cmdlib', source='libs/cmdlib/cmdlib.cpp')

mathlib_src = 'mathlib.c bbox.c line.c m4x4.c ray.c'
mathlib_lib = g_env.StaticLibrary(target='libs/mathlib', source=build_list('libs/mathlib', mathlib_src))

md5lib_lib = g_env.StaticLibrary(target='libs/md5lib', source='libs/md5lib/md5lib.c')

ddslib_lib = g_env.StaticLibrary(target='libs/ddslib', source='libs/ddslib/ddslib.c')

jpeg_env = g_env.Copy()
jpeg_env.Prepend(CPPPATH = 'libs/jpeg6')
jpeg_src = 'jcomapi.cpp jdcoefct.cpp jdinput.cpp jdpostct.cpp jfdctflt.cpp jpgload.cpp jdapimin.cpp jdcolor.cpp jdmainct.cpp jdsample.cpp jidctflt.cpp jutils.cpp jdapistd.cpp jddctmgr.cpp jdmarker.cpp jdtrans.cpp jmemmgr.cpp jdatasrc.cpp jdhuff.cpp jdmaster.cpp jerror.cpp jmemnobs.cpp'
jpeg_lib = jpeg_env.StaticLibrary(target='libs/jpeg6', source=build_list('libs/jpeg6', jpeg_src))

l_net_lib = g_env.StaticLibrary(target='libs/l_net', source=['libs/l_net/l_net.c', 'libs/l_net/l_net_berkley.c'])

picomodel_src = 'picointernal.c picomodel.c picomodules.c pm_3ds.c pm_ase.c pm_md3.c pm_obj.c\
  pm_ms3d.c pm_mdc.c pm_fm.c pm_md2.c pm_lwo.c pm_terrain.c lwo/clip.c lwo/envelope.c lwo/list.c lwo/lwio.c\
  lwo/lwo2.c lwo/lwob.c lwo/pntspols.c lwo/surface.c lwo/vecmath.c lwo/vmap.c'
picomodel_lib = g_env.StaticLibrary(target='libs/picomodel', source=build_list('libs/picomodel', picomodel_src))

#splines_env = g_env.Copy()
#splines_src = build_list('libs/splines', 'math_angles.cpp math_matrix.cpp math_quaternion.cpp math_vector.cpp q_parse.cpp q_shared.cpp splines.cpp util_str.cpp')
#splines_env['CPPPATH'].append('include')
#splines_lib = splines_env.StaticLibrary(target='libs/splines', source=splines_src)

profile_env = g_env.Copy();
profile_env['CPPPATH'].append('include')
profile_src = 'profile.cpp file.cpp'
profile_lib = profile_env.StaticLibrary(target='libs/profile', source=build_list('libs/profile', profile_src))

gtkutil_env = g_env.Copy();
gtkutil_env['CPPPATH'].append('include')
gtkutil_env.useGlib2()
gtkutil_env.useGtk2()
gtkutil_env.useGtkGLExt()

gtkutil_src = '\
  accelerator.cpp\
  button.cpp\
  clipboard.cpp\
  closure.cpp\
  container.cpp\
  cursor.cpp\
  dialog.cpp\
  entry.cpp\
  frame.cpp\
  filechooser.cpp\
  glfont.cpp\
  glwidget.cpp\
  image.cpp\
  idledraw.cpp\
  menu.cpp\
  messagebox.cpp\
  nonmodal.cpp\
  paned.cpp\
  pointer.cpp\
  toolbar.cpp\
  widget.cpp\
  window.cpp\
  xorrectangle.cpp\
'

gtkutil_lib = gtkutil_env.StaticLibrary(target='libs/gtkutil', source=build_list('libs/gtkutil', gtkutil_src))

# end static / common libraries ---------------------------------------------------

# q3map ---------------------------------------------------------------------------

q3map_env = g_env.Copy()
q3map_env['CPPPATH'].append('include')
q3map_env.useXML2()
q3map_env.useGlib2()
q3map_env.usePNG()
q3map_env.useMHash()
q3map_env.usePThread()
q3map_env.Prepend(CPPPATH='tools/quake3/common')

q3map_common_src = [
  'common/cmdlib.c',
  'common/imagelib.c',
  'common/inout.c',
  'common/mutex.c',
  'common/polylib.c',
  'common/scriplib.c',
  'common/threads.c',
  'common/unzip.c',
  'common/vfs.c' ]

q3map_src = [
  'q3map2/brush.c',
  'q3map2/brush_primit.c',
  'q3map2/bsp.c',
  'q3map2/facebsp.c',
  'q3map2/fog.c',
  'q3map2/leakfile.c',
  'q3map2/map.c',
  'q3map2/model.c',
  'q3map2/patch.c',
  'q3map2/portals.c',
  'q3map2/prtfile.c',
  'q3map2/surface.c',
  'q3map2/surface_fur.c',
  'q3map2/surface_meta.c',
  'q3map2/tjunction.c',
  'q3map2/tree.c',
  'q3map2/writebsp.c',
  'q3map2/image.c',
  'q3map2/light.c',
  'q3map2/light_bounce.c',
  'q3map2/light_trace.c',
  'q3map2/light_ydnar.c',
  'q3map2/lightmaps_ydnar.c',
  'q3map2/vis.c',
  'q3map2/visflow.c',
  'q3map2/bspfile_abstract.c',
  'q3map2/bspfile_ibsp.c',
  'q3map2/bspfile_rbsp.c',
  'q3map2/decals.c',
  'q3map2/main.c',
  'q3map2/mesh.c',
  'q3map2/path_init.c',
  'q3map2/shaders.c',
  'q3map2/surface_extra.c',
  'q3map2/surface_foliage.c',
  'q3map2/convert_ase.c',
  'q3map2/convert_map.c' ]

q3map_full_src = [ ]
for i in q3map_common_src + q3map_src:
  q3map_full_src.append('tools/quake3/' + i)

q3map_libs = ['mathlib', 'l_net', 'jpeg6', 'picomodel', 'ddslib']

q3map_prog = q3map_env.Program(target='q3map2.' + g_cpu, source=q3map_full_src, LIBS=q3map_libs, LIBPATH='libs')
q3map_env.Depends(q3map_prog, mathlib_lib)
q3map_env.Depends(q3map_prog, l_net_lib)
q3map_env.Depends(q3map_prog, jpeg_lib)
q3map_env.Depends(q3map_prog, picomodel_lib)
q3map_env.Depends(q3map_prog, ddslib_lib)
q3map_env.Install(INSTALL, q3map_prog)

# end q3map2 ----------------------------------------------------------------------

# q3data ---------------------------------------------------------------------------

q3data_env = q3map_env.Copy()

q3data_common_src = [
  'common/aselib.c',
  'common/bspfile.c',
  'common/cmdlib.c',
  'common/imagelib.c',
  'common/inout.c',
  'common/md4.c',
  'common/scriplib.c',
  'common/trilib.c',
  'common/unzip.c',
  'common/vfs.c'
  ]

q3data_src = [
  'q3data/3dslib.c',
  'q3data/compress.c',
  'q3data/images.c',
  'q3data/md3lib.c',
  'q3data/models.c',
  'q3data/p3dlib.c',
  'q3data/polyset.c',
  'q3data/q3data.c',
  'q3data/stripper.c',
  'q3data/video.c' ]

q3data_full_src = [ ]
for i in q3data_common_src + q3data_src:
  q3data_full_src.append('tools/quake3/' + i)

q3data_prog = q3data_env.Program( target = 'q3data.' + g_cpu, source = q3data_full_src, LIBS=['mathlib', 'l_net'], LIBPATH='libs' )
q3data_env.Depends(q3data_prog, mathlib_lib)
q3data_env.Depends(q3data_prog, l_net_lib)
q3data_env.Install( INSTALL, q3data_prog )

# end q3data ----------------------------------------------------------------------

# q2_tools ---------------------------------------------------------------------------

q2_tools_env = g_env.Copy()
q2_tools_env['CPPPATH'].append('include')
q2_tools_env.useXML2()
q2_tools_env.usePThread()
q2_tools_env.Prepend(CPPPATH='tools/quake2/common')

q2_tools_common_src = [
  'common/bspfile.c',
  'common/cmdlib.c',
  'common/inout.c',
  'common/l3dslib.c',
  'common/lbmlib.c',
  'common/mathlib.c',
  'common/md4.c',
  'common/path_init.c',
  'common/polylib.c',
  'common/scriplib.c',
  'common/threads.c',
  'common/trilib.c'
]


q2_tools_q2map_src = [
  'q2map/brushbsp.c',
  'q2map/csg.c',
  'q2map/faces.c',
  'q2map/flow.c',
  'q2map/glfile.c',
  'q2map/leakfile.c',
  'q2map/lightmap.c',
  'q2map/main.c',
  'q2map/map.c',
  'q2map/nodraw.c',
  'q2map/patches.c',
  'q2map/portals.c',
  'q2map/prtfile.c',
  'q2map/qbsp.c',
  'q2map/qrad.c',
  'q2map/qvis.c',
  'q2map/textures.c',
  'q2map/trace.c',
  'q2map/tree.c',
  'q2map/writebsp.c'
]

q2_tools_qdata3_common_src = [
  'common/bspfile.c',
  'common/cmdlib.c',
  'common/inout.c',
  'common/l3dslib.c',
  'common/lbmlib.c',
  'common/mathlib.c',
  'common/md4.c',
  'common/path_init.c',
  'common/scriplib.c',
  'common/threads.c',
  'common/trilib.c'
]

q2_tools_qdata3_src = [
  'qdata/images.c',
  'qdata/models.c',
  'qdata/qdata.c',
  'qdata/sprites.c',
  'qdata/tables.c',
  'qdata/video.c'
]

q2_tools_q2map_full_src = [ ]
for i in q2_tools_common_src + q2_tools_q2map_src:
  q2_tools_q2map_full_src.append('tools/quake2/' + i)

q2_tools_qdata3_full_src = [ ]
for i in q2_tools_common_src + q2_tools_qdata3_src:
  q2_tools_qdata3_full_src.append('tools/quake2/' + i)

if ( OS != 'Darwin' ):
  q2_tools_q2map_prog = q2_tools_env.Program(target='quake2_tools/q2map', source=q2_tools_q2map_full_src, LIBS='l_net', LIBPATH='libs')
  q2_tools_env.Depends(q2_tools_q2map_prog, l_net_lib)
  q2_tools_env.Install(INSTALL, q2_tools_q2map_prog )

  q2_tools_qdata3_prog = q2_tools_env.Program(target='quake2_tools/qdata3', source=q2_tools_qdata3_full_src, LIBS='l_net', LIBPATH='libs')
  q2_tools_env.Depends(q2_tools_qdata3_prog, l_net_lib)
  q2_tools_env.Install(INSTALL, q2_tools_qdata3_prog )


# end q2_tools ----------------------------------------------------------------------

# qdata3_heretic2 ---------------------------------------------------------------------------

heretic2_tools_env = g_env.Copy()
heretic2_tools_env['CPPPATH'].append('include')
heretic2_tools_env.useXML2()
heretic2_tools_env.usePThread()
heretic2_tools_env.Prepend(CPPPATH='tools/quake2/qdata_heretic2')
heretic2_tools_env.Prepend(CPPPATH='tools/quake2/qdata_heretic2/qcommon')
heretic2_tools_env.Prepend(CPPPATH='tools/quake2/qdata_heretic2/common')

heretic2_tools_qdata3_common_src = [
  'qdata_heretic2/common/bspfile.c',
  'qdata_heretic2/common/cmdlib.c',
  'qdata_heretic2/common/inout.c',
  'qdata_heretic2/common/l3dslib.c',
  'qdata_heretic2/common/lbmlib.c',
  'qdata_heretic2/common/mathlib.c',
  'qdata_heretic2/common/md4.c',
  'qdata_heretic2/common/path_init.c',
  'qdata_heretic2/common/qfiles.c',
  'qdata_heretic2/common/scriplib.c',
  'qdata_heretic2/common/threads.c',
  'qdata_heretic2/common/token.c',
  'qdata_heretic2/common/trilib.c'
]

heretic2_tools_qdata3_qcommon_src = [
  'qdata_heretic2/qcommon/reference.c',
  'qdata_heretic2/qcommon/resourcemanager.c',
  'qdata_heretic2/qcommon/skeletons.c'
]

heretic2_tools_qdata3_src = [
  'qdata_heretic2/animcomp.c',
  'qdata_heretic2/book.c',
  'qdata_heretic2/fmodels.c',
  'qdata_heretic2/images.c',
  'qdata_heretic2/jointed.c',
  'qdata_heretic2/models.c',
  'qdata_heretic2/pics.c',
  'qdata_heretic2/qdata.c',
  'qdata_heretic2/qd_skeletons.c',
  'qdata_heretic2/sprites.c',
  'qdata_heretic2/svdcmp.c',
  'qdata_heretic2/tables.c',
  'qdata_heretic2/tmix.c',
  'qdata_heretic2/video.c'
]

heretic2_tools_qdata3_full_src = [ ]
for i in heretic2_tools_qdata3_common_src + heretic2_tools_qdata3_qcommon_src + heretic2_tools_qdata3_src:
  heretic2_tools_qdata3_full_src.append('tools/quake2/' + i)


heretic2_tools_env['CCFLAGS'] += '-D_LINUX '

if ( OS != 'Darwin' ):
  heretic2_tools_prog = heretic2_tools_env.Program(target='h2data', source=heretic2_tools_qdata3_full_src, LIBS='l_net', LIBPATH='libs')
  heretic2_tools_env.Depends(heretic2_tools_prog, l_net_lib)
  heretic2_tools_env.Install(INSTALL + '/heretic2', heretic2_tools_prog )

# end heretic2_tools ----------------------------------------------------------------------



# radiant, modules and plugins ----------------------------------------------------

module_env = g_env.Copy()
module_env['CPPPATH'].append('include')
if ( OS == 'Darwin' ):
  module_env['LINKFLAGS'] += '-dynamiclib -ldl '
else:
  module_env['LINKFLAGS'] += '-ldl '
module_env['LIBPREFIX'] = ''


vfspk3_env = module_env.Copy()
vfspk3_lst = build_list('plugins/vfspk3', 'vfspk3.cpp vfs.cpp archive.cpp')
vfspk3_env.useGlib2()
vfspk3_lib = vfspk3_env.SharedLibrarySafe(target='vfspk3', source=vfspk3_lst)
vfspk3_env.Install(INSTALL + '/modules', vfspk3_lib)

archivepak_env = module_env.Copy()
archivepak_lst = build_list('plugins/archivepak', 'plugin.cpp archive.cpp pak.cpp')
archivepak_lib = archivepak_env.SharedLibrarySafe(target='archivepak', source=archivepak_lst, LIBS='cmdlib', LIBPATH='libs')
archivepak_env.Depends(archivepak_lib, cmdlib_lib)
archivepak_env.Install(INSTALL + '/modules', archivepak_lib)

archivewad_env = module_env.Copy()
archivewad_lst = build_list('plugins/archivewad', 'plugin.cpp archive.cpp wad.cpp')
archivewad_lib = archivewad_env.SharedLibrarySafe(target='archivewad', source=archivewad_lst, LIBS='cmdlib', LIBPATH='libs')
archivewad_env.Depends(archivewad_lib, cmdlib_lib)
archivewad_env.Install(INSTALL + '/modules', archivewad_lib)

archivezip_env = module_env.Copy()
archivezip_lst = build_list('plugins/archivezip', 'plugin.cpp archive.cpp pkzip.cpp zlibstream.cpp')
archivezip_env.useZLib()
archivezip_lib = archivezip_env.SharedLibrarySafe(target='archivezip', source=archivezip_lst, LIBS='cmdlib', LIBPATH='libs')
archivezip_env.Depends(archivezip_lib, cmdlib_lib)
archivezip_env.Install(INSTALL + '/modules', archivezip_lib)

shaders_env = module_env.Copy()
shaders_lst = build_list('plugins/shaders', 'shaders.cpp plugin.cpp')
shaders_env.useGlib2()
shaders_lib = shaders_env.SharedLibrarySafe(target='shaders', source=shaders_lst, LIBS='cmdlib', LIBPATH='libs')
shaders_env.Depends(shaders_lib, cmdlib_lib)
shaders_env.Install(INSTALL + '/modules', shaders_lib)

image_env = module_env.Copy()
image_lst = build_list('plugins/image', 'bmp.cpp jpeg.cpp image.cpp pcx.cpp tga.cpp dds.cpp')
image_lib = image_env.SharedLibrarySafe(target='image', source=image_lst, LIBS=['jpeg6', 'ddslib'], LIBPATH='libs')
image_env.Depends(image_lib, jpeg_lib)
image_env.Depends(image_lib, ddslib_lib)
image_env.Install(INSTALL + '/modules', image_lib)

imagehl_lst=build_list('plugins/imagehl', 'imagehl.cpp hlw.cpp mip.cpp sprite.cpp')
imagehl_lib = module_env.SharedLibrarySafe(target='imagehl', source=imagehl_lst)
module_env.Install(INSTALL + '/modules', imagehl_lib)

imageq2_lst = build_list('plugins/imageq2', 'imageq2.cpp wal.cpp wal32.cpp')
imageq2_lib = module_env.SharedLibrarySafe(target='imageq2', source=imageq2_lst)
module_env.Install(INSTALL + '/modules', imageq2_lib)

mapq3_env = module_env.Copy()
mapq3_lst=build_list('plugins/mapq3', 'plugin.cpp parse.cpp write.cpp')
mapq3_lib = mapq3_env.SharedLibrarySafe(target='mapq3', source=mapq3_lst, LIBS='cmdlib', LIBPATH='libs')
mapq3_env.Depends(mapq3_lib, cmdlib_lib)
mapq3_env.Install(INSTALL + '/modules', mapq3_lib)

imagepng_env = module_env.Copy()
imagepng_lst = build_list('plugins/imagepng', 'plugin.cpp')
imagepng_env.usePNG()
imagepng_lib = imagepng_env.SharedLibrarySafe(target='imagepng', source=imagepng_lst)
imagepng_env.Install(INSTALL + '/modules', imagepng_lib)

mapxml_env = module_env.Copy()
mapxml_lst = build_list('plugins/mapxml', 'plugin.cpp xmlparse.cpp xmlwrite.cpp')
mapxml_lib = mapxml_env.SharedLibrarySafe(target='mapxml', source=mapxml_lst)
mapxml_env.useXML2()
mapxml_env.useGlib2()
mapxml_env.Install(INSTALL + '/modules', mapxml_lib)

model_env = module_env.Copy()
model_lst = build_list('plugins/model', 'plugin.cpp model.cpp')
model_lib = model_env.SharedLibrarySafe(target='model', source=model_lst, LIBS=['mathlib', 'picomodel'], LIBPATH='libs')
model_env.Depends(model_lib, mathlib_lib)
model_env.Depends(model_lib, picomodel_lib)
model_env.Install(INSTALL + '/modules', model_lib)

md3model_lst=build_list('plugins/md3model', 'plugin.cpp mdl.cpp md3.cpp md2.cpp mdc.cpp mdlimage.cpp md5.cpp')
md3model_lib = module_env.SharedLibrarySafe(target='md3model', source=md3model_lst)
module_env.Install(INSTALL + '/modules', md3model_lib)

entity_lst = build_list('plugins/entity', 'plugin.cpp entity.cpp eclassmodel.cpp generic.cpp group.cpp light.cpp miscmodel.cpp doom3group.cpp skincache.cpp angle.cpp angles.cpp colour.cpp filters.cpp model.cpp namedentity.cpp origin.cpp scale.cpp targetable.cpp rotation.cpp modelskinkey.cpp')
entity_lib = module_env.SharedLibrarySafe(target='entity', source=entity_lst)
module_env.Install(INSTALL + '/modules', entity_lib)

bob_env = module_env.Copy()
bob_lst = build_list('contrib/bobtoolz/',
'dialogs/dialogs-gtk.cpp bobToolz-GTK.cpp bsploader.cpp cportals.cpp DBobView.cpp \
DBrush.cpp DEntity.cpp DEPair.cpp DMap.cpp DPatch.cpp DPlane.cpp DPoint.cpp \
DShape.cpp DTrainDrawer.cpp DTreePlanter.cpp DVisDrawer.cpp DWinding.cpp funchandlers-GTK.cpp \
lists.cpp misc.cpp ScriptParser.cpp shapes.cpp visfind.cpp')
bob_lib = bob_env.SharedLibrarySafe(target='bobtoolz', source=bob_lst, LIBS=['mathlib', 'cmdlib', 'profile'], LIBPATH='libs')
bob_env.Depends(bob_lib, mathlib_lib)
bob_env.Depends(bob_lib, cmdlib_lib)
bob_env.Depends(bob_lib, profile_lib)
bob_env.useGlib2()
bob_env.useGtk2()
bob_env.Install(INSTALL + '/plugins', bob_lib)

#camera_lst = build_list('contrib/camera', 
#'camera.cpp dialogs.cpp dialogs_common.cpp funchandlers.cpp listener.cpp misc.cpp renderer.cpp')
#camera_lst.append('libs/libsplines.a')
#bob_env.SharedLibrarySafe(target='camera', source=camera_lst)
#bob_env.Install(INSTALL + '/plugins', 'camera.so')

prtview_env = module_env.Copy()
prtview_lst = build_list('contrib/prtview', 'AboutDialog.cpp ConfigDialog.cpp LoadPortalFileDialog.cpp portals.cpp prtview.cpp')
prtview_env.useGlib2()
prtview_env.useGtk2()
prtview_lib = prtview_env.SharedLibrarySafe(target='prtview', source=prtview_lst, LIBS='profile', LIBPATH='libs')
prtview_env.Depends(prtview_lib, profile_lib)
prtview_env.Install(INSTALL + '/plugins', prtview_lib)

#gensurf_lst = build_list('contrib/gtkgensurf',
#'bitmap.cpp dec.cpp face.cpp font.cpp gendlgs.cpp genmap.cpp gensurf.cpp heretic.cpp plugin.cpp view.cpp triangle.c')
#bob_env.SharedLibrarySafe(target='gensurf', source=gensurf_lst)
#bob_env.Install(INSTALL + '/plugins', 'gensurf.so')

#bkgrnd2d_list = build_list( 'contrib/bkgrnd2d', 'bkgrnd2d.cpp plugin.cpp dialog.cpp' )
#bkgrnd2d_list.append( 'libs/libsynapse.a' )
#bkgrnd2d_env = module_env.Copy()
#bkgrnd2d_env.useGtk2()
#bkgrnd2d_env.SharedLibrarySafe( target='bkgrnd2d', source=bkgrnd2d_list )
#bkgrnd2d_env.Install( INSTALL + '/plugins', 'bkgrnd2d.so' )

radiant_env = g_env.Copy()
radiant_env['CPPPATH'].append('include')
radiant_env['LINKFLAGS'] += '-ldl -lGL '
if ( OS == 'Darwin' ):
  radiant_env['CXXFLAGS'] += '-fno-common '
  radiant_env['CCFLAGS'] += '-fno-common '
  radiant_env['LINKFLAGS'] += '-lX11 -lGL -lGLU '
radiant_env['LIBPREFIX'] = ''
radiant_env.useGlib2()
radiant_env.useXML2()
radiant_env.useGtk2()
radiant_env.useGtkGLExt()

radiant_src = [
'autosave.cpp',
'brush.cpp',
'brushmanip.cpp',
'brushmodule.cpp',
'brushnode.cpp',
'brushtokens.cpp',
'brushxml.cpp',
'brush_primit.cpp',
'build.cpp',
'camwindow.cpp',
'clippertool.cpp',
'commands.cpp',
'console.cpp',
'csg.cpp',
'dialog.cpp',
'eclass.cpp',
'eclass_def.cpp',
'eclass_doom3.cpp',
'eclass_fgd.cpp',
'eclass_xml.cpp',
'entity.cpp',
'entityinspector.cpp',
'entitylist.cpp',
'environment.cpp',
'error.cpp',
'feedback.cpp',
'filetypes.cpp',
'filters.cpp',
'findtexturedialog.cpp',
'glwidget.cpp',
'grid.cpp',
'groupdialog.cpp',
'gtkdlgs.cpp',
'gtkmisc.cpp',
'help.cpp',
'image.cpp',
'main.cpp',
'mainframe.cpp',
'map.cpp',
'mru.cpp',
'nullmodel.cpp',
'parse.cpp',
'patch.cpp',
'patchdialog.cpp',
'patchmanip.cpp',
'patchmodule.cpp',
'plugin.cpp',
'pluginapi.cpp',
'pluginmanager.cpp',
'pluginmenu.cpp',
'plugintoolbar.cpp',
'points.cpp',
'preferencedictionary.cpp',
'preferences.cpp',
'qe3.cpp',
'qgl.cpp',
'referencecache.cpp',
'renderer.cpp',
'renderstate.cpp',
'scenegraph.cpp',
'select.cpp',
'selection.cpp',
'server.cpp',
'shaders.cpp',
'sockets.cpp',
'surfacedialog.cpp',
'texmanip.cpp',
'textures.cpp',
'texwindow.cpp',
'timer.cpp',
'treemodel.cpp',
'undo.cpp',
'url.cpp',
'view.cpp',
'watchbsp.cpp',
'winding.cpp',
'windowobservers.cpp',
'xmlstuff.cpp',
'xywindow.cpp',
]

for i in range(len(radiant_src)):
  radiant_src[i] = 'radiant/' + radiant_src[i]

radiant_libs = ['mathlib', 'cmdlib', 'l_net', 'profile', 'gtkutil']
radiant_prog = radiant_env.Program(target='radiant.' + g_cpu, source=radiant_src, LIBS=radiant_libs, LIBPATH='libs')
radiant_env.Depends(radiant_prog, mathlib_lib)
radiant_env.Depends(radiant_prog, cmdlib_lib)
radiant_env.Depends(radiant_prog, l_net_lib)
radiant_env.Depends(radiant_prog, profile_lib)
radiant_env.Depends(radiant_prog, gtkutil_lib)
radiant_env.Install(INSTALL, radiant_prog)


# end setup ---------------------------------------------------------------------------------------
