# install the fucking files
# check the md5 to decide for a copy
# we have a site.conf to go through
# and a data list of stuff
# it's called at the end of each build to copy what needs to be
# it could be called only at GtkRadiant link time

# command line: a bunch of config switches default would be debug?, and release flag


import sys, traceback, os, re, filecmp, shutil, pickle, string
from makeversion import get_version

(line, major, minor) = get_version()

paths = {
  'CORERADIANTDIR' : 'C:\\Program Files\\GtkRadiant-1.' + major,
  'RTCWRADIANTDIR' : 'C:\\Program Files\\Return to Castle Wolfenstein - Game of The Year Edition\\Radiant-1.' + major,
  'HLRADIANTDIR'   : 'C:\\Sierra\\Half-Life\\Radiant-1.' + major,
  'SOF2RADIANTDIR' : 'C:\\Program Files\\Soldier of Fortune II - Double Helix\\Radiant-1.' + major,
  'QUAKE2RADIANTDIR' : 'C:\\Quake2\\Radiant-1.' + major,
  'HERETIC2RADIANTDIR' : 'C:\\Heretic2\\Radiant-1.' + major
  }

conf_filename='site.conf.win32'

# site settings ----------------------

if (os.path.exists(conf_filename)):
  site_file = open(conf_filename, 'r')
  p = pickle.Unpickler(site_file)
  paths = p.load()
  print 'Loaded configuration from ' + conf_filename  

# end site settings ------------------

# command line overrides -------------

print '\nCommand line:'
regex = re.compile("(.*)=(.*)")
for i in sys.argv[1:]:
  #print i
  if ( regex.match(i) ):
    (key, val) = string.split(i, '=')
    #print 'key: %s val: %s' % (key, val)
    paths[key] = val
  else:
    print 'Can''t parse command line - ignore: ' + i

# end command line overrides ---------

# save config ------------------------

site_file = open(conf_filename, 'w')
p = pickle.Pickler(site_file)
p.dump(paths)
site_file.close()

print '\nConfiguration:'

for i in paths.keys():
  print '%s -> %s' % (i, paths[i])

# end save config --------------------

q3map2_list = [
#	( ( '..\\libpng\\projects\\msvc\\win32\\zlib\\dll_dbg\\zlibd.dll', '..\\libpng\\projects\\msvc\\win32\\zlib\\dll\\zlib.dll' ), '$CORERADIANTDIR' ),
#	( ( '..\\libpng\\projects\\msvc\\win32\\libpng\\dll_dbg\\libpng13d.dll', '..\\libpng\\projects\\msvc\\win32\\libpng\\dll\\libpng13.dll' ), '$CORERADIANTDIR' ),
#	( ( '..\\libxml2\\win32\\binaries-debug\\libxml2.dll', '..\\libxml2\\win32\\binaries-release\\libxml2.dll' ), '$CORERADIANTDIR' ),
	( ( 'tools\\quake3\\q3map2\\Debug\\Q3Map2.exe', 'tools\\quake3\\q3map2\\Release\\Q3Map2.exe' ), '$CORERADIANTDIR' ),
	]

all = [
	( ( 'radiant\\Debug\\GtkRadiant.exe', 'radiant\\Release\\GtkRadiant.exe' ) ,  '$CORERADIANTDIR' ),
	( ( 'contrib\\bobtoolz\\Debug\\bobToolz.dll', 'contrib\\bobtoolz\\Release\\bobToolz.dll' ), '$CORERADIANTDIR\\plugins' ),
	( ( 'contrib\\camera\\Debug\\camera.dll', 'contrib\\camera\\Release\\camera.dll' ), '$CORERADIANTDIR\\plugins' ),
	( ( 'plugins\\entity\\Debug\\entity.dll', 'plugins\\entity\\Release\\entity.dll' ), '$CORERADIANTDIR\\modules' ),
	( ( 'plugins\\eclassfgd\\Debug\\fgd.dll', 'plugins\\eclassfgd\\Release\\fgd.dll' ), '$CORERADIANTDIR\\modules' ),
	( ( 'contrib\\gtkgensurf\\Debug\\gensurf.dll', 'contrib\\gtkgensurf\\Release\\gensurf.dll' ), '$CORERADIANTDIR\\plugins' ),
	( ( 'contrib\\hydratoolz\\Debug\\hydratoolz.dll', 'contrib\\hydratoolz\\Release\\hydratoolz.dll' ), '$HLRADIANTDIR\\plugins' ),	
	( ( 'plugins\\image\\Debug\\image.dll', 'plugins\\image\\Release\\image.dll' ), '$CORERADIANTDIR\\modules' ),
	( ( 'plugins\\imagewal\\Debug\\imagewal.dll', 'plugins\\imagewal\\Release\\imagewal.dll' ), '$QUAKE2RADIANTDIR\\modules' ),
	( ( 'plugins\\imagehl\\Debug\\imagehl.dll', 'plugins\\imagehl\\Release\\imagehl.dll' ), '$CORERADIANTDIR\\modules' ),
	( ( 'plugins\\imagepng\\Debug\\imagepng.dll', 'plugins\\imagepng\\Release\\imagepng.dll' ), '$CORERADIANTDIR\\modules' ),
	( ( 'plugins\\map\\Debug\\map.dll', 'plugins\\map\\Release\\map.dll' ), '$CORERADIANTDIR\\modules' ),	
	( ( 'plugins\\mapxml\\Debug\\mapxml.dll', 'plugins\\mapxml\\Release\\mapxml.dll' ), '$CORERADIANTDIR\\modules' ),
	( ( 'plugins\\model\\Debug\\model.dll', 'plugins\\model\\Release\\model.dll' ), '$CORERADIANTDIR\\modules' ),
	( ( 'contrib\\prtview\\Debug\\PrtView.dll', 'contrib\\prtview\\Release\\PrtView.dll' ), '$CORERADIANTDIR\\plugins' ),
	( ( 'tools\\quake3\\q3data\\Debug\\q3data.exe', 'tools\\quake3\\q3data\\Release\\q3data.exe' ), '$CORERADIANTDIR' ),
	( ( 'plugins\\shaders\\Debug\\shaders.dll', 'plugins\\shaders\\Release\\shaders.dll' ), '$CORERADIANTDIR\\modules' ),	
	( ( 'plugins\\spritemodel\\Debug\\spritemodel.dll', 'plugins\\spritemodel\\Release\\spritemodel.dll' ), '$CORERADIANTDIR\\modules' ),
#	( ( 'Debug\\TexTool.dll', 'Release\\TexTool.dll' ), '$CORERADIANTDIR\\plugins' ),
	( ( 'plugins\\vfspk3\\Debug\\vfspk3.dll', 'plugins\\vfspk3\\Release\\vfspk3.dll' ), '$CORERADIANTDIR\\modules' ),
	( ( 'plugins\\vfspak\\Debug\\vfspak.dll', 'plugins\\vfspak\\Release\\vfspak.dll' ), '$QUAKE2RADIANTDIR\\modules' ),
	( ( 'plugins\\vfswad\\Debug\\vfswad.dll', 'plugins\\vfswad\\Release\\vfswad.dll' ), '$CORERADIANTDIR\\modules' ),
  ( ( 'plugins\\surface\\Debug\\surface.dll', 'plugins\\surface\\Release\\surface.dll' ), '$CORERADIANTDIR\\modules' ),
  ( ( 'plugins\\surface_quake2\\Debug\\surface_quake2.dll', 'plugins\\surface_quake2\\Release\\surface_quake2.dll' ), '$QUAKE2RADIANTDIR\\modules' ),
  ( ( 'tools\\quake2\\q2map\\Debug\\q2map.exe', 'tools\\quake2\\q2map\\Release\\q2map.exe' ) ,  '$QUAKE2RADIANTDIR' ),
  ( ( 'tools\\quake2\\qdata\\Debug\\qdata3.exe', 'tools\\quake2\\qdata\\Release\\qdata3.exe' ) ,  '$QUAKE2RADIANTDIR' ),
  ( ( 'plugins\\vfspak\\Debug\\vfspak.dll', 'plugins\\vfspak\\Release\\vfspak.dll' ), '$HERETIC2RADIANTDIR\\modules' ),
  ( ( 'plugins\\imagem8\\Debug\\imagem8.dll', 'plugins\\imagem8\\Release\\imagem8.dll' ), '$HERETIC2RADIANTDIR\\modules' ),
  ( ( 'plugins\\surface_heretic2\\Debug\\surface_heretic2.dll', 'plugins\\surface_heretic2\\Release\\surface_heretic2.dll' ), '$HERETIC2RADIANTDIR\\modules' ),
  ( ( 'tools\\quake2\\qdata_heretic2\\Debug\\qdata3.exe', 'tools\\quake2\\qdata_heretic2\\Release\\qdata3.exe' ) ,  '$HERETIC2RADIANTDIR' ),
	( ( 'contrib\\bkgrnd2d\\Debug\\bkgrnd2d.dll', 'contrib\\bkgrnd2d\\Release\\bkgrnd2d.dll' ), '$CORERADIANTDIR\\plugins' ),
	]

config = 0
q3map2 = 0

# config check
for i in sys.argv:
	if ( i == 'release' ):
		config = 1
	elif ( i == 'q3map2' ):
		q3map2 = 1

if ( config == 1 ):
	print 'installing release binaries'
else:
	print 'installing debug binaries'

def expand(path_in):
	for matches in paths.keys():
		exp = re.compile('\\$%s' % matches)
		if ( not re.match(exp, path_in) is None ):
			#print "Got a match for %s on %s" % ( matches, path_in )
			path_in = re.sub(exp, paths[matches], path_in)	
			#print "Processed to: %s" % path_in
	return path_in

# don't bother about stderr
sys.stderr = sys.stdout

if ( q3map2 == 0 ):
	stuff = q3map2_list
	stuff += all
else:
	stuff = q3map2_list

for entries in stuff:
	try:
		src = expand(entries[0][config])
		#print "src basename: %s -> %s" % (src, os.path.basename(src))
		dst = entries[1]
		dst = os.path.join( entries[1], os.path.basename(src) )
		dst = expand(dst)
		#print "src: %s dst: %s" % (src, dst)
		if (os.path.isfile(src)):
			if (os.path.isfile(dst)):
				if (not filecmp.cmp(src, dst)):
					shutil.copy(src, dst)
					print "%s: OK - updated" % dst
				else:
					print "%s: OK - already up to date" % dst
			else:
				shutil.copy(src, dst)
				print "%s: OK - installed" % dst
		else:	
			print "%s: FAILED - not found" % src
			if (os.path.isfile(dst)):
				print "delete %s" % dst
				os.remove(dst)
	except:
		print "%s: FAILED" % src
		traceback.print_exc()
		
