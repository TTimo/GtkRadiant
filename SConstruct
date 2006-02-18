# scons build script
# http://scons.sourceforge.net

import commands, re, sys, os, pickle, string, popen2
from makeversion import radiant_makeversion, get_version

# to access some internal stuff
import SCons

conf_filename='site.conf'
# there is a default hardcoded value, you can override on command line, those are saved between runs
# we only handle strings
serialized=['CC', 'CXX', 'JOBS', 'BUILD', 'SETUP']

# help -------------------------------------------

Help("""
Usage: scons [OPTIONS] [TARGET] [CONFIG]

[OPTIONS] and [TARGET] are covered in command line options, use scons -H

[CONFIG]: KEY="VALUE" [...]
a number of configuration options saved between runs in the """ + conf_filename + """ file
erase """ + conf_filename + """ to start with default settings again

CC
CXX
	Specify C and C++ compilers (defaults gcc and g++)
	ex: CC="gcc-3.2"
	You can use ccache and distcc, for instance:
	CC="ccache distcc gcc" CXX="ccache distcc g++"

JOBS
	Parallel build
	ex: JOBS="4" is a good setting on SMP machines

BUILD
	Use debug/release to select build settings
	ex: BUILD="release" - default is debug
"""
)

# end help ---------------------------------------
  
# sanity -----------------------------------------

# get a recent python release
# that is broken in current version:
# http://sourceforge.net/tracker/index.php?func=detail&aid=794145&group_id=30337&atid=398971
#EnsurePythonVersion(2,1)
# above 0.90
EnsureSConsVersion( 0, 96 )
print 'SCons ' + SCons.__version__

# end sanity -------------------------------------

# system detection -------------------------------

# TODO: detect Darwin / OSX

# CPU type
g_cpu = commands.getoutput('uname -m')
exp = re.compile('.*i?86.*')
if (g_cpu == 'Power Macintosh' or g_cpu == 'ppc'):
  g_cpu = 'ppc'
elif exp.match(g_cpu):
  g_cpu = 'x86'
else:
  g_cpu = 'cpu'

# OS
OS = commands.getoutput('uname')
print "OS=\"" + OS + "\""

if (OS == 'Linux'):
  # libc .. do the little magic!
  libc = commands.getoutput('/lib/libc.so.6 |grep "GNU C "|grep version|awk -F "version " \'{ print $2 }\'|cut -b -3')

# end system detection ---------------------------

# default settings -------------------------------

CC='gcc'
CXX='g++'
JOBS='1'
BUILD='debug'
INSTALL='#install'
SETUP='0'
g_build_root = 'build'

# end default settings ---------------------------

# site settings ----------------------------------

site_dict = {}
if (os.path.exists(conf_filename)):
	site_file = open(conf_filename, 'r')
	p = pickle.Unpickler(site_file)
	site_dict = p.load()
	print 'Loading build configuration from ' + conf_filename
	for k, v in site_dict.items():
		exec_cmd = k + '=\"' + v + '\"'
		print exec_cmd
		exec(exec_cmd)

# end site settings ------------------------------

# command line settings --------------------------

for k in serialized:
	if (ARGUMENTS.has_key(k)):
		exec_cmd = k + '=\"' + ARGUMENTS[k] + '\"'
		print 'Command line: ' + exec_cmd
		exec(exec_cmd)

# end command line settings ----------------------

# sanity check -----------------------------------

if (SETUP == '1' and BUILD != 'release' and BUILD != 'info'):
  print 'Forcing release build for setup'
  BUILD = 'release'

def GetGCCVersion(name):
  ret = commands.getstatusoutput('%s -dumpversion' % name)
  if ( ret[0] != 0 ):
    return None
  vers = string.split(ret[1], '.')
  if ( len(vers) == 2 ):
    return [ vers[0], vers[1], 0 ]
  elif ( len(vers) == 3 ):
    return vers
  return None

ver_cc = GetGCCVersion(CC)
ver_cxx = GetGCCVersion(CXX)

if ( ver_cc is None or ver_cxx is None or ver_cc[0] < '3' or ver_cxx[0] < '3' or ver_cc != ver_cxx ):
  print 'Compiler version check failed - need gcc 3.x or later:'
  print 'CC: %s %s\nCXX: %s %s' % ( CC, repr(ver_cc), CXX, repr(ver_cxx) )
  Exit(1)

# end sanity check -------------------------------

# save site configuration ----------------------

for k in serialized:
	exec_cmd = 'site_dict[\'' + k + '\'] = ' + k
	exec(exec_cmd)

site_file = open(conf_filename, 'w')
p = pickle.Pickler(site_file)
p.dump(site_dict)
site_file.close()

# end save site configuration ------------------

# general configuration, target selection --------

SConsignFile( "scons.signatures" )

g_build = g_build_root + '/' + BUILD

SetOption('num_jobs', JOBS)

LINK = CXX
# common flags
warningFlags = '-W -Wall -Wcast-align -Wcast-qual -Wno-unused-parameter '
warningFlagsCXX = '-Wno-non-virtual-dtor -Wreorder ' # -Wold-style-cast
CCFLAGS = '' + warningFlags
CXXFLAGS = '-pipe -DQ_NO_STLPORT ' + warningFlags + warningFlagsCXX
CPPPATH = []
if (BUILD == 'debug'):
	CXXFLAGS += '-g -D_DEBUG '
	CCFLAGS += '-g -D_DEBUG '
elif (BUILD == 'release'):
	CXXFLAGS += '-O2 '
	CCFLAGS += '-O2 '
else:
	print 'Unknown build configuration ' + BUILD
	sys.exit( 0 )

LINKFLAGS = ''
if ( OS == 'Linux' ):

  # static
  # 2112833 /opt/gtkradiant/radiant.x86
  # 35282 /opt/gtkradiant/modules/archivezip.so
  # 600099 /opt/gtkradiant/modules/entity.so
  
  # dynamic
  # 2237060 /opt/gtkradiant/radiant.x86
  # 110605 /opt/gtkradiant/modules/archivezip.so
  # 730222 /opt/gtkradiant/modules/entity.so
  
  # EVIL HACK - force static-linking for libstdc++ - create a symbolic link to the static libstdc++ in the root
  os.system("ln -s `g++ -print-file-name=libstdc++.a`")
  
  #if not os.path.exists("./install"):
  #  os.mkdir("./install")
  #os.system("cp `g++ -print-file-name=libstdc++.so` ./install")
  
  CXXFLAGS += '-fno-exceptions -fno-rtti '
  LINKFLAGS += '-Wl,-fini,fini_stub -L. -static-libgcc '
if ( OS == 'Darwin' ):
  CCFLAGS += '-force_cpusubtype_ALL -fPIC '
  CXXFLAGS += '-force_cpusubtype_ALL -fPIC -fno-exceptions -fno-rtti '
  CPPPATH.append('/sw/include')
  CPPPATH.append('/usr/X11R6/include')
  LINKFLAGS += '-L/sw/lib -L/usr/lib -L/usr/X11R6/lib '

CPPPATH.append('libs')

# extend the standard Environment a bit
class idEnvironment(Environment):

  def __init__(self):
    Environment.__init__(self,
      ENV = os.environ, 
      CC = CC,
      CXX = CXX,
      LINK = LINK,
      CCFLAGS = CCFLAGS,
      CXXFLAGS = CXXFLAGS,
      CPPPATH = CPPPATH,
      LINKFLAGS = LINKFLAGS)

  def useGlib2(self):
    self['CXXFLAGS'] += '`pkg-config glib-2.0 --cflags` '
    self['CCFLAGS'] += '`pkg-config glib-2.0 --cflags` '
    self['LINKFLAGS'] += '-lglib-2.0 '
    
  def useXML2(self):
    self['CXXFLAGS'] += '`xml2-config --cflags` '      
    self['CCFLAGS'] += '`xml2-config --cflags` '      
    self['LINKFLAGS'] += '-lxml2 '

  def useGtk2(self):
    self['CXXFLAGS'] += '`pkg-config gtk+-2.0 --cflags` '
    self['CCFLAGS'] += '`pkg-config gtk+-2.0 --cflags` '
    self['LINKFLAGS'] += '-lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lpango-1.0 -lgdk_pixbuf-2.0 '
   
  def useGtkGLExt(self):
    self['CXXFLAGS'] += '`pkg-config gtkglext-1.0 --cflags` '
    self['CCFLAGS'] += '`pkg-config gtkglext-1.0 --cflags` '
    self['LINKFLAGS'] += '-lgtkglext-x11-1.0 -lgdkglext-x11-1.0 '      
    
  def usePNG(self):
    self['CXXFLAGS'] += '`libpng-config --cflags` '
    self['CCFLAGS'] += '`libpng-config --cflags` '
    self['LINKFLAGS'] += '`libpng-config --ldflags` '
    
  def useMHash(self):
    self['LINKFLAGS'] += '-lmhash '
  
  def useZLib(self):
    self['LINKFLAGS'] += '-lz '
    
  def usePThread(self):
    if ( OS == 'Darwin' ):
      self['LINKFLAGS'] += '-lpthread -Wl,-stack_size,0x400000 '
    else:
      self['LINKFLAGS'] += '-lpthread '

  def CheckLDD(self, target, source, env):
    file = target[0]
    if (not os.path.isfile(file.abspath)):
        print('ERROR: CheckLDD: target %s not found\n' % target[0])
        Exit(1)
    # not using os.popen3 as I want to check the return code
    ldd = popen2.Popen3('`which ldd` -r %s' % target[0], 1)
    stdout_lines = ldd.fromchild.readlines()
    stderr_lines = ldd.childerr.readlines()
    ldd_ret = ldd.wait()
    del ldd
    have_undef = 0
    if ( ldd_ret != 0 ):
        print "ERROR: ldd command returned with exit code %d" % ldd_ret
        os.system('rm %s' % target[0])
        Exit()
    for i_line in stderr_lines:
        print repr(i_line)
        regex = re.compile('undefined symbol: (.*)\t\\((.*)\\)\n')
        if ( regex.match(i_line) ):
            symbol = regex.sub('\\1', i_line)
            try:
                env['ALLOWED_SYMBOLS'].index(symbol)
            except:
                have_undef = 1
        else:
            print "ERROR: failed to parse ldd stderr line: %s" % i_line
            os.system('rm %s' % target[0])
            Exit(1)
    if ( have_undef ):
        print "ERROR: undefined symbols"
        os.system('rm %s' % target[0])
        Exit(1)
  
  def SharedLibrarySafe(self, target, source, LIBS=[], LIBPATH='.'):
    result = self.SharedLibrary(target, source, LIBS=LIBS, LIBPATH=LIBPATH)
    if (OS != 'Darwin'):
      AddPostAction(target + '.so', self.CheckLDD)
    return result

g_env = idEnvironment()

# export the globals
GLOBALS = 'g_env INSTALL SETUP g_cpu'

radiant_makeversion('\\ngcc version: %s.%s.%s' % ( ver_cc[0], ver_cc[1], ver_cc[2] ) )

# end general configuration ----------------------

# targets ----------------------------------------

Default('.')

Export('GLOBALS ' + GLOBALS)
BuildDir(g_build, '.', duplicate = 0)
SConscript(g_build + '/SConscript')

# end targets ------------------------------------
