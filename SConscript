import os, sys, commands, string
from makeversion import get_version
# OS Detection:
OS = commands.getoutput('uname')

Import('GLOBALS')
Import(GLOBALS)

# make scons link shared libs against static libs
g_env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1

# make gcc accept default parameters in function typedefs
g_env['CXXFLAGS'] += '-fpermissive '

g_env['CXXFLAGS'] += '-fPIC '
g_env['CCFLAGS'] += '-fPIC '

def build_list(s_prefix, s_string):
	s_list = Split(s_string)
	for i in range(len(s_list)):
		s_list[i] = s_prefix + '/' + s_list[i]
	return s_list

# common code ------------------------------------------------------

cmdlib_lib = g_env.StaticLibrary(target='libs/cmdlib', source='libs/cmdlib/cmdlib.cpp')


mathlib_src = 'mathlib.c bbox.c linear.c m4x4.c ray.c'
mathlib_lib = g_env.StaticLibrary(target='libs/mathlib', source=build_list('libs/mathlib', mathlib_src))


md5lib_lib = g_env.StaticLibrary(target='libs/md5lib', source='libs/md5lib/md5lib.c')


ddslib_lib = g_env.StaticLibrary(target='libs/ddslib', source='libs/ddslib/ddslib.c')


jpeg_env = g_env.Copy()
jpeg_env.Prepend(CPPPATH = 'libs/jpeg6')
jpeg_src = 'jcomapi.cpp jdcoefct.cpp jdinput.cpp jdpostct.cpp jfdctflt.cpp jpgload.cpp jdapimin.cpp jdcolor.cpp jdmainct.cpp jdsample.cpp jidctflt.cpp jutils.cpp jdapistd.cpp jddctmgr.cpp jdmarker.cpp jdtrans.cpp jmemmgr.cpp jdatasrc.cpp jdhuff.cpp jdmaster.cpp jerror.cpp jmemnobs.cpp'
jpeg_lib = jpeg_env.StaticLibrary(target='libs/jpeg', source=build_list('libs/jpeg6', jpeg_src))


net_lib = g_env.StaticLibrary(target='libs/l_net', source=['libs/l_net/l_net.c', 'libs/l_net/l_net_berkley.c'])


picomodel_src = 'picointernal.c picomodel.c picomodules.c pm_3ds.c pm_ase.c pm_md3.c pm_obj.c\
  pm_ms3d.c pm_mdc.c pm_fm.c pm_md2.c pm_lwo.c lwo/clip.c lwo/envelope.c lwo/list.c lwo/lwio.c\
  lwo/lwo2.c lwo/lwob.c lwo/pntspols.c lwo/surface.c lwo/vecmath.c lwo/vmap.c'
picomodel_lib = g_env.StaticLibrary(target='libs/picomodel', source=build_list('libs/picomodel', picomodel_src))


synapse_env = g_env.Copy()
synapse_env.useGlib2()
synapse_env.useXML2()
synapse_env['CPPPATH'].append('include')
synapse_src = 'synapse.cpp'
synapse_lib = synapse_env.StaticLibrary(target='libs/synapse', source=build_list('libs/synapse', synapse_src))
# scons 0.95. Doesn't recognize archive compatible for dynamic modules
# see thread: http://scons.tigris.org/servlets/BrowseList?listName=users&by=thread&from=168952&to=168952&first=1&count=2


splines_env = g_env.Copy()
splines_src = build_list('libs/splines', 
'math_angles.cpp math_matrix.cpp math_quaternion.cpp math_vector.cpp q_parse.cpp q_shared.cpp splines.cpp util_str.cpp')
splines_env['CPPPATH'].append('include')
splines_lib = splines_env.StaticLibrary(target='libs/splines', source=splines_src)


# end static / common libraries ---------------------------------------------------

# q3map ---------------------------------------------------------------------------

q3map_env = g_env.Copy()
q3map_env['CPPPATH'].append('include')
q3map_env.useXML2()
q3map_env.useGlib2()
q3map_env.usePNG()
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

q3map_full_src.append('libs/libmathlib.a')
q3map_full_src.append('libs/libl_net.a')
q3map_full_src.append('libs/libjpeg.a')
q3map_full_src.append('libs/libpicomodel.a')
q3map_full_src.append('libs/libmd5lib.a')
q3map_full_src.append('libs/libddslib.a')

q3map_env.Program(target='q3map2.' + g_cpu, source=q3map_full_src )
q3map_env.Install(INSTALL, 'q3map2.' + g_cpu)

# end q3map2 ----------------------------------------------------------------------

# q3data ---------------------------------------------------------------------------

q3data_env = q3map_env.Copy()

q3data_common_src = [
  'common/aselib.c',
  'common/bspfile.c',
  'common/cmdlib.c',
  'common/imagelib.c',
  'common/inout.c',
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

q3data_full_src.append('libs/libmathlib.a')
q3data_full_src.append('libs/libl_net.a')

q3data_env.Program( target = 'q3data.' + g_cpu, source = q3data_full_src )
q3data_env.Install( INSTALL, 'q3data.' + g_cpu )

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

q2_tools_q2map_full_src.append('libs/libl_net.a')

q2_tools_qdata3_full_src = [ ]
for i in q2_tools_common_src + q2_tools_qdata3_src:
  q2_tools_qdata3_full_src.append('tools/quake2/' + i)

q2_tools_qdata3_full_src.append('libs/libl_net.a')

if ( OS != 'Darwin' ):
	q2_tools_env.Program(target='quake2_tools/q2map', source=q2_tools_q2map_full_src )
	q2_tools_env.Install(INSTALL + '/q2', 'quake2_tools/q2map' )

	q2_tools_env.Program(target='quake2_tools/qdata3', source=q2_tools_qdata3_full_src )
	q2_tools_env.Install(INSTALL + '/q2', 'quake2_tools/qdata3' )


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

heretic2_tools_qdata3_full_src.append('libs/libl_net.a')

heretic2_tools_env['CCFLAGS'] += '-D_LINUX '

if ( OS != 'Darwin' ):
	heretic2_tools_env.Program(target='heretic2_tools/qdata3', source=heretic2_tools_qdata3_full_src )
	heretic2_tools_env.Install(INSTALL + '/heretic2', 'heretic2_tools/qdata3' )

	heretic2_q2map_env = q2_tools_env
	heretic2_q2map_env.Install(INSTALL + '/heretic2', 'quake2_tools/q2map' )

# end heretic2_tools ----------------------------------------------------------------------



# radiant, modules and plugins ----------------------------------------------------

module_env = g_env.Copy()
module_env['CPPPATH'].append('include')
if ( OS == 'Darwin' ):
	module_env['LINKFLAGS'] += '-dynamiclib -ldl '
else:
	module_env['LINKFLAGS'] += '-ldl '
module_env['LIBPREFIX'] = ''
module_env.useGlib2()
module_env.useXML2()

module_env.SharedLibrarySafe(target='fgd', source=['plugins/eclassfgd/plugin.cpp', 'libs/libsynapse.a'])
module_env.Install(INSTALL + '/modules', 'fgd.so')

vfspk3_lst=build_list('plugins/vfspk3', 'vfspk3.cpp vfs.cpp unzip.cpp')
vfspk3_lst.append('libs/libsynapse.a')
module_env.SharedLibrarySafe(target='vfspk3', source=vfspk3_lst)
module_env.Install(INSTALL + '/modules', 'vfspk3.so')

vfswad_lst=build_list('plugins/vfswad', 'unwad.cpp vfs.cpp vfswad.cpp')
vfswad_lst.append('libs/libsynapse.a')
module_env.SharedLibrarySafe(target='vfswad', source=vfswad_lst)
module_env.Install(INSTALL + '/modules', 'vfswad.so')

vfspak_lst=build_list('plugins/vfspak', 'vfspak.cpp vfs.cpp')
vfspak_lst.append('libs/libsynapse.a')
module_env.SharedLibrarySafe(target='vfspak', source=vfspak_lst)
module_env.Install(INSTALL + '/q2/modules', 'vfspak.so')
module_env.Install(INSTALL + '/heretic2/modules', 'vfspak.so')

shaders_lst=build_list('plugins/shaders', 'shaders.cpp plugin.cpp')
shaders_lst.append('libs/libsynapse.a')
module_env.SharedLibrarySafe(target='shaders', source=shaders_lst)
module_env.Install(INSTALL + '/modules', 'shaders.so')

image_lst=build_list('plugins/image', 'jpeg.cpp image.cpp lbmlib.cpp')
image_lst.append('libs/libsynapse.a')
image_lst.append('libs/libjpeg.a')
module_env.SharedLibrarySafe(target='image', source=image_lst)
module_env.Install(INSTALL + '/modules', 'image.so')

imagewal_lst=build_list('plugins/imagewal', 'wal.cpp imagewal.cpp')
imagewal_lst.append('libs/libsynapse.a')
module_env.SharedLibrarySafe(target='imagewal', source=imagewal_lst)
module_env.Install(INSTALL + '/q2/modules', 'imagewal.so')

imagem8_lst=build_list('plugins/imagem8', 'm8.cpp m32.cpp imagem8.cpp')
imagem8_lst.append('libs/libsynapse.a')
module_env.SharedLibrarySafe(target='imagem8', source=imagem8_lst)
module_env.Install(INSTALL + '/heretic2/modules', 'imagem8.so')

imagehl_lst=build_list('plugins/imagehl', 'imagehl.cpp lbmlib.cpp')
imagehl_lst.append('libs/libsynapse.a')
module_env.SharedLibrarySafe(target='imagehl', source=imagehl_lst)
module_env.Install(INSTALL + '/modules', 'imagehl.so')

imagepng_lst=build_list('plugins/imagepng', 'plugin.cpp')
imagepng_lst.append('libs/libsynapse.a')
module_env.usePNG()
module_env.SharedLibrarySafe(target='imagepng', source=imagepng_lst)
module_env.Install(INSTALL + '/modules', 'imagepng.so')

map_lst=build_list('plugins/map', 'plugin.cpp parse.cpp write.cpp')
map_lst.append('libs/libsynapse.a')
map_lst.append('libs/libcmdlib.a')
module_env.SharedLibrarySafe(target='map', source=map_lst)
module_env.Install(INSTALL + '/modules', 'map.so')

mapxml_lst=build_list('plugins/mapxml', 'plugin.cpp xmlparse.cpp xmlwrite.cpp')
mapxml_lst.append('libs/libsynapse.a')
module_env.SharedLibrarySafe(target='mapxml', source=mapxml_lst)
module_env.Install(INSTALL + '/modules', 'mapxml.so')

model_lst=build_list('plugins/model', 'plugin.cpp model.cpp cpicomodel.cpp cpicosurface.cpp remap.cpp')
model_lst.append('libs/libsynapse.a')
model_lst.append('libs/libpicomodel.a')
model_lst.append('libs/libmathlib.a')
module_env.SharedLibrarySafe(target='model', source=model_lst)
module_env.Install(INSTALL + '/modules', 'model.so')

entity_lst = build_list('plugins/entity', 'plugin.cpp entity_entitymodel.cpp miscmodel.cpp eclassmodel.cpp entity.cpp light.cpp')
entity_lst.append('libs/libsynapse.a')
entity_lst.append('libs/libmathlib.a')
module_env.SharedLibrarySafe(target='entity', source=entity_lst)
module_env.Install(INSTALL + '/modules', 'entity.so')

bob_env = module_env.Copy()
bob_env.useGtk2()
bob_lst = build_list('contrib/bobtoolz/',
'dialogs/dialogs-gtk.cpp bobToolz-GTK.cpp bsploader.cpp cportals.cpp DBobView.cpp \
DBrush.cpp DEntity.cpp DEPair.cpp DListener.cpp DMap.cpp DPatch.cpp DPlane.cpp DPoint.cpp \
DShape.cpp DTrainDrawer.cpp DTreePlanter.cpp DVisDrawer.cpp DWinding.cpp funchandlers-GTK.cpp \
lists.cpp misc.cpp ScriptParser.cpp shapes.cpp visfind.cpp')
bob_lst.append('libs/libsynapse.a')
bob_lst.append('libs/libmathlib.a')
bob_lst.append('libs/libcmdlib.a')
bob_env['CPPPATH'].append('contrib/bobtoolz/dialogs')
bob_env.SharedLibrarySafe(target='bobtoolz', source=bob_lst)
bob_env.Install(INSTALL + '/plugins', 'bobtoolz.so')

camera_lst = build_list('contrib/camera', 
'camera.cpp dialogs.cpp dialogs_common.cpp funchandlers.cpp listener.cpp misc.cpp renderer.cpp')
camera_lst.append('libs/libsynapse.a')
camera_lst.append('libs/libsplines.a')
bob_env.SharedLibrarySafe(target='camera', source=camera_lst)
bob_env.Install(INSTALL + '/plugins', 'camera.so')

prtview_lst = build_list('contrib/prtview', 
'AboutDialog.cpp ConfigDialog.cpp LoadPortalFileDialog.cpp portals.cpp prtview.cpp')
prtview_lst.append('libs/libsynapse.a')
prtview_env = bob_env.Copy()
prtview_env['CXXFLAGS'] += '-DGTK_PLUGIN '
prtview_env.SharedLibrarySafe(target='prtview', source=prtview_lst)
prtview_env.Install(INSTALL + '/plugins', 'prtview.so')

gensurf_lst = build_list('contrib/gtkgensurf',
'bitmap.cpp dec.cpp face.cpp font.cpp gendlgs.cpp genmap.cpp gensurf.cpp heretic.cpp plugin.cpp view.cpp triangle.c')
gensurf_lst.append('libs/libsynapse.a')
bob_env.SharedLibrarySafe(target='gensurf', source=gensurf_lst)
bob_env.Install(INSTALL + '/plugins', 'gensurf.so')

surface_lst = build_list('plugins/surface', 'surfdlg_plugin.cpp surfacedialog.cpp')
surface_lst.append('libs/libsynapse.a')
surface_lst.append('libs/libmathlib.a')
surface_env = module_env.Copy()
surface_env.useGtk2()
surface_env.SharedLibrarySafe(target='surface', source=surface_lst)
surface_env.Install(INSTALL + '/modules', 'surface.so')

surface_quake2_lst = build_list('plugins/surface_quake2', 'surfdlg_plugin.cpp surfacedialog.cpp surfaceflagsdialog_quake2.cpp')
surface_quake2_lst.append('libs/libsynapse.a')
surface_quake2_lst.append('libs/libmathlib.a')
surface_quake2_env = module_env.Copy()
surface_quake2_env.useGtk2()
surface_quake2_env.SharedLibrarySafe(target='surface_quake2', source=surface_quake2_lst)
surface_quake2_env.Install(INSTALL + '/q2/modules', 'surface_quake2.so')

surface_heretic2_lst = build_list('plugins/surface_heretic2', 'surfdlg_plugin.cpp surfacedialog.cpp surfaceflagsdialog_heretic2.cpp')
surface_heretic2_lst.append('libs/libsynapse.a')
surface_heretic2_lst.append('libs/libmathlib.a')
surface_heretic2_env = module_env.Copy()
surface_heretic2_env.useGtk2()
surface_heretic2_env.SharedLibrarySafe(target='surface_heretic2', source=surface_heretic2_lst)
surface_heretic2_env.Install(INSTALL + '/heretic2/modules', 'surface_heretic2.so')

bkgrnd2d_list = build_list( 'contrib/bkgrnd2d', 'bkgrnd2d.cpp plugin.cpp dialog.cpp' )
bkgrnd2d_list.append( 'libs/libsynapse.a' )
bkgrnd2d_env = module_env.Copy()
bkgrnd2d_env.useGtk2()
bkgrnd2d_env.SharedLibrarySafe( target='bkgrnd2d', source=bkgrnd2d_list )
bkgrnd2d_env.Install( INSTALL + '/plugins', 'bkgrnd2d.so' )

radiant_env = g_env.Copy()
radiant_env['CPPPATH'].append('include')
radiant_env['LINKFLAGS'] += '-ldl '
if ( OS == 'Darwin' ):
  radiant_env['CXXFLAGS'] += '-fno-common '
  radiant_env['CCFLAGS'] += '-fno-common '
  radiant_env['LINKFLAGS'] += '-lX11 -lGL -lGLU '
radiant_env['LIBPREFIX'] = ''
radiant_env.useGlib2()
radiant_env.useXML2()
radiant_env.useGtk2()
radiant_env.useGtkGLExt()

radiant_src=[ 'qgl.c', 'brush.cpp', 'brush_primit.cpp', 'brushscript.cpp', 'camwindow.cpp', 'csg.cpp',
 'dialog.cpp', 'dialoginfo.cpp', 'drag.cpp', 'eclass.cpp', 'eclass_def.cpp', 'error.cpp', 'feedback.cpp',
 'file.cpp', 'findtexturedialog.cpp', 'glinterface.cpp', 'glwidget.cpp', 'glwindow.cpp', 'groupdialog.cpp', 
 'gtkdlgs.cpp', 'gtkmisc.cpp', 'main.cpp', 'mainframe.cpp', 'map.cpp', 'missing.cpp', 'parse.cpp', 
 'patchdialog.cpp', 'pluginentities.cpp', 'pluginmanager.cpp', 'pmesh.cpp', 'points.cpp', 'preferences.cpp', 
 'profile.cpp', 'qe3.cpp', 'qgl_ext.cpp', 'select.cpp', 'selectedface.cpp', 'surfacedialog.cpp',
 'surfaceplugin.cpp', 'targetname.cpp', 'texmanip.cpp', 'texwindow.cpp', 'undo.cpp', 'vertsel.cpp', 
 'watchbsp.cpp', 'winding.cpp', 'xywindow.cpp', 'z.cpp', 'zwindow.cpp', 'filters.cpp', 'bp_dlg.cpp', 'ui.cpp' ]

for i in range(len(radiant_src)):
  radiant_src[i] = 'radiant/' + radiant_src[i]

radiant_src.append('libs/libmathlib.a')
radiant_src.append('libs/libcmdlib.a')
radiant_src.append('libs/libl_net.a')
radiant_src.append('libs/libsynapse.a')

radiant_env.Program(target='radiant.' + g_cpu, source=radiant_src)
radiant_env.Install(INSTALL, 'radiant.' + g_cpu)

# setup -------------------------------------------------------------------------------------------

class setup_builder:

  g_dryrun = 0
  
  def system(self, cmd):
    if (self.g_dryrun):
      print cmd
    else:
      sys.stdout.write(cmd)
      ret = commands.getstatusoutput(cmd)
      print ret[1]
      if (ret[0] != 0):
        raise 'command failed'

  def copy_core(self):
    # binaries and misc
    self.system('mkdir -p %s/modules' % self.SETUP_BIN_DIR)
    self.system('mkdir -p %s/plugins' % self.SETUP_BIN_DIR)
    self.system('cp install/%s %s' % (self.EDITOR_BIN, self.SETUP_BIN_DIR))
    self.system('cp install/modules/*.so %s/modules' % self.SETUP_BIN_DIR )
    self.system('cp install/plugins/*.so %s/plugins' % self.SETUP_BIN_DIR )
    self.system('cp install/q3map2.%s %s' % ( g_cpu, self.SETUP_BIN_DIR ) )
    self.M4_STDC = ''
    if (not self.g_darwin):    
      # fugly
      # copy libgcc_s and stdc++ over to distribute it and reduce potential ABI fuckups
      ret = commands.getstatusoutput('ldd -r install/' + self.EDITOR_BIN + ' 2>/dev/null | grep libgcc_s | sed -e \'s/.* => \\([^ ]*\\) .*/\\1/\'')
      if (ret[0] != 0):
        raise 'ldd command failed'
      self.system('cp ' + ret[1] + ' ' + self.SETUP_BIN_DIR)
      ret = commands.getstatusoutput('ldd -r install/' + self.EDITOR_BIN + ' 2>/dev/null | grep libstdc++ | sed -e \'s/.* => \\([^ ]*\\) .*/\\1/\'')
      if (ret[0] != 0):
        raise 'ldd command failed'
      lines = string.split(ret[1], '\n')
      self.M4_STDC = '"' 
      for i in lines:
        self.system('cp ' + i + ' ' + self.SETUP_BIN_DIR)
        self.M4_STDC += os.path.basename(i) + ' \n'
      self.M4_STDC += '"'
    # hack for symlink
    # setup process generates the wrapper at install time
    # but we need a dummy executable for symlink in loki_setup
    self.system('echo -n "#!/bin/sh\necho If you read this then there was a bug during setup. Report the bug and try running %s directly from it\'s installation directory.\n" > %s/radiant' % (self.EDITOR_BIN, self.SETUP_BIN_DIR));
    self.system('echo -n "#!/bin/sh\necho If you read this then there was a bug during setup. Report the bug and try running %s directly from it\'s installation directory.\n" > %s/q3map2' % (self.EDITOR_BIN, self.SETUP_BIN_DIR));
    ## this goes to the core install directory
    DEST = self.SETUP_DIR + '/core'
    self.system('mkdir -p ' + DEST + '/modules/bitmaps')
    # general content stuff
    self.system('cp -R plugins/model/bitmaps/* ' + DEST + '/modules/bitmaps')
    self.system('cp -R setup/data/tools/* ' + DEST)
    self.system('cp -R radiant/bitmaps ' + DEST)
    self.system('cp setup/changelog.txt ' + DEST)
    self.system('cp setup/openurl.sh ' + DEST)
    self.system('cp tools/quake3/q3map2/changelog.q3map2.txt ' + DEST)
    # documentation
    self.system('cp -R docs/manual/Q3Rad_Manual ' + DEST)
    self.system('cp -R docs/manual/quake3/Compile_Manual ' + DEST)
    self.system('cp -R docs/manual/quake3/Model_Manual ' + DEST)
    self.system('cp -R docs/manual/quake3/Terrain_Manual ' + DEST)
    # copy plugins media
    self.system('mkdir -p ' + DEST + '/plugins/bitmaps')
    self.system('cp -R contrib/bobtoolz/bitmaps/* ' + DEST + '/plugins/bitmaps')
    self.system('cp -R contrib/bobtoolz/bt ' + DEST + '/plugins')
    self.system('cp -R contrib/camera/bitmaps/* ' + DEST + '/plugins/bitmaps' )
    self.system('cp -R contrib/bkgrnd2d/bitmaps/* ' + DEST + '/plugins/bitmaps' )
  
  def copy_q3(self):
    # binaries
    self.system('mkdir -p ' + self.SETUP_BIN_DIR + '/q3')
    if ( self.g_darwin == 0 ):
      self.system('cp setup/linux/bspc ' + self.SETUP_BIN_DIR + '/q3')

    # goes in core
    DEST = self.SETUP_DIR + '/core/q3'
    self.system('mkdir -p ' + DEST)
    self.system('cp setup/data/tools/synapse.config ' + DEST)
    self.system('cp setup/data/tools/game.xlink ' + DEST)
    self.system('cp -R docs/manual/quake3/Team_Arena_Mapping_Help ' + DEST)
    self.system('cp -R docs/manual/quake3/New_Teams_For_Q3TA ' + DEST)
    self.system('cp -R docs/manual/quake3/Q3AShader_Manual ' + DEST)
  
    # goes in the game install path
    DEST = self.SETUP_DIR + '/q3'
    self.system('mkdir -p ' + DEST)
    self.system('cp -R setup/data/baseq3 ' + DEST)
    self.system('cp -R setup/data/missionpack ' + DEST)

  def copy_wolf(self):
    # binaries
    self.system('mkdir -p ' + self.SETUP_BIN_DIR + '/wolf')
    if ( self.g_darwin == 0 ):
      self.system('cp ../WolfPack/bin/Linux/bspc ' + self.SETUP_BIN_DIR + '/wolf')
  
    # goes in core
    DEST = self.SETUP_DIR + '/core/wolf'
    self.system('mkdir -p ' + DEST)
    self.system('cp ../WolfPack/synapse.config ' + DEST)
    self.system('cp -R ../WolfPack/docs ' + DEST)
    self.system('cp ../WolfPack/game.xlink ' + DEST)
    self.system('cp ../WolfPack/bin/aascfg_lg.c ' + DEST)
    self.system('cp ../WolfPack/bin/aascfg_sm.c ' + DEST)
    self.system('cp ../WolfPack/bin/bspc.ai ' + DEST)

    # goes in the game install path
    DEST = self.SETUP_DIR + '/wolf/main'
    self.system('mkdir -p ' + DEST)
    self.system('cp ../WolfPack/astro-skies.pk3 ' + DEST)
    self.system('cp ../WolfPack/common-astro-spog.pk3 ' + DEST)
    self.system('cp ../WolfPack/lights.pk3 ' + DEST)      
    self.system('cp -R ../WolfPack/scripts ' + DEST)
    self.system('cp -R ../WolfPack/maps ' + DEST)
    self.system('cp -R ../WolfPack/models ' + DEST)
  
  def copy_et(self):
    # goes in core
    DEST = self.SETUP_DIR + '/core/et'
    self.system('mkdir -p ' + DEST)    
    self.system('cp -R ../ETPack/bitmaps ' + DEST)
    self.system('cp -R ../ETPack/docs ' + DEST)
    self.system('cp ../ETPack/game.xlink ' + DEST)
    self.system('cp ../ETPack/synapse.config ' + DEST)
    
    # goes in game install path
    DEST = self.SETUP_DIR + '/et/etmain'
    self.system('mkdir -p ' + DEST)
    self.system('cp ../ETPack/astro-skies.pk3 ' + DEST)
    self.system('cp ../ETPack/common.pk3 ' + DEST)
    self.system('cp ../ETPack/goldrush.pcx ' + DEST)    
    self.system('cp ../ETPack/lights.pk3 ' + DEST)
    self.system('cp ../ETPack/mapmedia.pk3 ' + DEST)
    self.system('cp -R ../ETPack/scripts ' + DEST)
    self.system('cp -R ../ETPack/maps ' + DEST)
    self.system('cp -R ../ETPack/models ' + DEST)

  def copy_q2(self):
    # binaries
    self.system('cp -R install/q2 %s' % (self.SETUP_BIN_DIR))

    # goes in core
    DEST = self.SETUP_DIR + '/core/q2'
    self.system('mkdir -p ' + DEST + '/modules')
    self.system('cp ../Q2Pack/game.xlink ' + DEST)
    self.system('cp ../Q2Pack/synapse.config ' + DEST)
    self.system('cp install/q2/q2map install/q2/qdata3 ' + DEST) 
    self.system('cp -R install/q2/modules ' + DEST )

    # goes in game install path
    DEST = self.SETUP_DIR + '/q2'
    self.system('mkdir -p ' + DEST +  '/baseq2')
    self.system('cp -R ../Q2Pack/baseq2/* ' + DEST + '/baseq2')

  def copy_her2(self):
    # binaries
    self.system('cp -R install/heretic2 %s' % (self.SETUP_BIN_DIR))

    # goes in core
    DEST = self.SETUP_DIR + '/core/heretic2'
    self.system('mkdir -p ' + DEST + '/modules')
    self.system('cp ../Her2Pack/game.xlink ' + DEST)
    self.system('cp ../Her2Pack/synapse.config ' + DEST)
    self.system('cp install/q2/q2map install/heretic2/qdata3 ' + DEST)
    self.system('cp -R install/heretic2/modules ' + DEST )

    # goes in game install path
    DEST = self.SETUP_DIR + '/heretic2'
    self.system('mkdir -p ' + DEST +  '/base')
    self.system('cp -R ../Her2Pack/base/* ' + DEST + '/base')

  def build_setup(self):
    self.system( 'cp -R ' + self.SETUP_IMAGE_OS + '/* ' + self.SETUP_DIR )
    self.system( 'cp -fR ' + self.SETUP_IMAGE + '/* ' + self.SETUP_DIR )
    self.system('cp setup/license.txt ' + self.SETUP_DIR)
    self.system('cp setup/linux/README ' + self.SETUP_DIR)
    OS_DEFS=''
    if (self.g_darwin):
      OS_DEFS='--define=M4_OSX'
    M4_LINE = OS_DEFS + ' --define=M4_VER_MAJOR=' + self.major + ' --define=M4_VER_MINOR=' + self.minor + ' --define=M4_VER=' + self.line 
    M4_LINE += ' --define=M4_GAME_ET=%d' % self.DO_GAME_ET
    M4_LINE += ' --define=M4_GAME_Q2=%d' % self.DO_GAME_Q2
    if ( self.M4_STDC != '' ):
      M4_LINE += ' --define=M4_STDC=' + self.M4_STDC 
    # setup.xml
    self.system('m4 ' + M4_LINE + ' ' + self.SETUP_DIR + '/setup.data/setup.xml.in > ' + self.SETUP_DIR + '/setup.data/setup.xml')
    # postinstall.sh
    self.system('m4 ' + M4_LINE + ' ' + self.SETUP_DIR + '/setup.data/postinstall.sh.in > ' + self.SETUP_DIR + '/setup.data/postinstall.sh')
    # config.sh
    self.system('m4 ' + M4_LINE + ' ' + self.SETUP_DIR + '/setup.data/config.sh.in > ' + self.SETUP_DIR + '/setup.data/config.sh')
    # setup.sh
    self.system('m4 ' + M4_LINE + ' ' + self.SETUP_DIR + '/setup.sh.in > ' + self.SETUP_DIR + '/setup.sh')
    self.system('chmod +x ' +self.SETUP_DIR + '/setup.sh')
    self.system('find ' + self.SETUP_DIR + ' -name .svn | while read i ; do rm -r "$i" ; done')
    # pack it up
    self.system('setup/linux/makeself/makeself.sh ' + self.SETUP_DIR + ' ' + self.SETUP_TARGET + ' "GtkRadiant ' + self.line + ' setup" ./setup.sh')
    if (self.g_darwin):
      def build_fink_deb(self):
        print "Building installer .deb\n"
        self.F_REV = '1'
        self.FINKINFO_DIR = '/sw/fink/10.2/unstable/main/finkinfo/games'
        self.TARBALL_DIR='radiant-' + self.F_REV + '.' + self.major 
        self.TARBALL_NAME='radiant-' + self.F_REV + '.' + self.major + '.tar.gz'
        self.TARBALL_DEST='/sw/src'

        # prepare package description
        self.system('mkdir -p ' + self.FINKINFO_DIR)
        self.system('m4 ' + M4_LINE + ' --define=M4_SETUP_TARGET=' + self.SETUP_TARGET + '  --define=M4_F_REV=' + self.F_REV + ' ' + 'setup/osx/radiant.info.m4 > ' + self.FINKINFO_DIR + '/radiant-' + self.TARBALL_DIR + '.info')

        # build the tarball
        self.system('if [ -r /tmp/' + self.TARBALL_DIR + ' ] ; then rm -r ' '/tmp/' + self.TARBALL_DIR + ' ; fi')
        self.system('mkdir -p ' '/tmp/' + self.TARBALL_DIR)
        self.system('cp ' + self.SETUP_TARGET + ' ' + '/tmp/' + self.TARBALL_DIR)
        self.system('cd /tmp ; tar -cvzf ' + self.TARBALL_NAME + ' ' + self.TARBALL_DIR + ' ; cp ' + self.TARBALL_NAME + ' ' + self.TARBALL_DEST + '/')
        self.system('/sw/bin/fink rebuild radiant')

      	build_fink_deb(self)
      
  def spawn_setup(self, env, target, source):
    if ( OS == 'Darwin' ):
      self.g_darwin = 1
    else:
      self.g_darwin = 0
    (self.line, self.major, self.minor) = get_version()
    print 'Setup: GtkRadiant %s' % self.line  
    if ( self.g_darwin ):
      self.SETUP_IMAGE_OS = '../loki_setup/image'
    else:
      self.SETUP_IMAGE_OS = 'setup/linux/setup_image.Linux'
    self.SETUP_IMAGE = 'setup/linux/setup_image'
    self.SETUP_DIR = '/tmp/radiant-setup.%d' % os.getpid()
    self.EDITOR_BIN='radiant.' + g_cpu
    if ( self.g_darwin ):
      self.SETUP_BIN_DIR = self.SETUP_DIR + '/bin/Darwin/ppc'
      self.SETUP_TARGET = 'osx-radiant-%s.run' % self.line
    else:
      self.SETUP_BIN_DIR = self.SETUP_DIR + '/bin/Linux/x86'
      self.SETUP_TARGET = 'linux-radiant-%s.run' % self.line
    # TODO: eval a conf file instead
    self.DO_CORE=1
    self.DO_GAME_Q3=1
    self.DO_GAME_WOLF=1
    self.DO_GAME_ET=1
    self.DO_GAME_Q2=1
    self.DO_GAME_HER2=1
    if ( self.g_darwin ):
      self.DO_GAME_Q2=0
      self.DO_GAME_ET=0
      self.DO_GAME_HER2=0
    # verbose a bit
    print 'version: %s major: %s minor: %s\neditor core: %d\nq3: %d\nwolf: %d\nq2: %d\nher2: %d' % (self.line, self.major, self.minor, self.DO_CORE, self.DO_GAME_Q3, self.DO_GAME_WOLF, self.DO_GAME_Q2, self.DO_GAME_HER2)
    if (self.DO_CORE):
      self.copy_core()
    if (self.DO_GAME_Q3):
      self.copy_q3()
    if (self.DO_GAME_WOLF):
      self.copy_wolf()
    if (self.DO_GAME_ET):
      self.copy_et()
    if (self.DO_GAME_Q2):
      self.copy_q2()
    if (self.DO_GAME_HER2):
      self.copy_her2()
    self.build_setup()
    return 0
    
def spawn_setup(env, target, source):
  setup = setup_builder()
  setup.spawn_setup(env, target, source)

# NOTE: could modify g_env to add the deps auto when calling SharedLibrarySafe ..
if (SETUP == '1'):
  g_env.Command('foo', INSTALL + '/radiant.' + g_cpu, [ spawn_setup ] )
  depends_list = [ 
    INSTALL + '/modules/entity.so',
    INSTALL + '/modules/fgd.so',
    INSTALL + '/modules/imagehl.so',
    INSTALL + '/modules/image.so',
    INSTALL + '/modules/imagepng.so',
    INSTALL + '/modules/map.so',
    INSTALL + '/modules/mapxml.so',
    INSTALL + '/modules/model.so',
    INSTALL + '/modules/shaders.so',
    INSTALL + '/modules/surface.so',
    INSTALL + '/modules/vfspk3.so',
    INSTALL + '/modules/vfswad.so',
    INSTALL + '/plugins/bobtoolz.so',
    INSTALL + '/plugins/camera.so',
    INSTALL + '/plugins/prtview.so',
    INSTALL + '/plugins/gensurf.so',
    INSTALL + '/plugins/bkgrnd2d.so',
    INSTALL + '/q3map2.' + g_cpu,
    INSTALL + '/radiant.' + g_cpu,
    INSTALL + '/q3data.' + g_cpu ]
  if ( OS != 'Darwin' ):
    depends_list += [
      INSTALL + '/q2/modules/imagewal.so',
      INSTALL + '/q2/modules/surface_quake2.so',
      INSTALL + '/q2/modules/vfspak.so',
      INSTALL + '/q2/q2map',
      INSTALL + '/q2/qdata3',
      INSTALL + '/heretic2/modules/imagem8.so',
      INSTALL + '/heretic2/modules/surface_heretic2.so',
      INSTALL + '/heretic2/modules/vfspak.so',
      INSTALL + '/heretic2/qdata3',
      INSTALL + '/heretic2/q2map' ]
  g_env.Depends( 'foo', depends_list )

# end setup ---------------------------------------------------------------------------------------
