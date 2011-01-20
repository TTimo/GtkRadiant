import sys, traceback, platform, re, commands, platform, subprocess

if __name__ != '__main__':
	from SCons.Script import *

import utils

# config = debug release
# aliases are going to be very needed here
# we have dependency situations too
# target =

class Config:
	# not used atm, but useful to keep a list in mind
	# may use them eventually for the 'all' and other aliases expansions?
	target_choices = utils.Enum( 'radiant', 'q3map2', 'setup' )
	config_choices = utils.Enum( 'debug', 'release' )

	# aliases
	# 'all' -> for each choices
	# 'gamecode' for the targets, 'game' 'cgame' 'ui'

	def __init__( self ):
		# initialize defaults
		self.target_selected = [ 'radiant', 'q3map2' ]
		self.config_selected = [ 'release' ]
		# those are global to each config
		self.platform = platform.system()
		self.cc = 'gcc'
		self.cxx = 'g++'
		self.install_directory = 'install'

		# platforms for which to assemble a setup
		self.setup_platforms = [ 'local', 'x86', 'x64', 'win32' ]
		# paks to assemble in the setup
		self.setup_packs = [ 'Q3Pack', 'UrTPack', 'UFOAIPack', 'Q2WPack', 'JAPack', 'ReactionPack' ]

	def __repr__( self ):
		return 'config: target=%s config=%s' % ( self.target_selected, self.config_selected )

	def _processTarget( self, ops ):
		self.target_selected = ops

	def _processConfig( self, ops ):
		self.config_selected = ops

	def _processCC( self, ops ):
		self.cc = ops

	def _processCXX( self, ops ):
		self.cxx = ops

	def _processInstallDir( self, ops ):
		self.install_directory = os.path.normpath( os.path.expanduser( ops[0] ) )

	def _processSetupPlatforms( self, ops ):
		self.setup_platforms = ops

	def _processSetupPacks( self, ops ):
		self.setup_packs = ops

	def setupParser( self, operators ):
		operators['target'] = self._processTarget
		operators['config'] = self._processConfig
		operators['cc'] = self._processCC
		operators['cxx'] = self._processCXX
		operators['install_directory'] = self._processInstallDir
		operators['setup_platforms'] = self._processSetupPlatforms
		operators['setup_packs'] = self._processSetupPacks

	def emit_radiant( self ):
		settings = self
		for config_name in self.config_selected:
			config = {}
			config['name'] = config_name
			config['shared'] = False
			Export( 'utils', 'settings', 'config' )
			build_dir = os.path.join( 'build', config_name, 'radiant' )
			BuildDir( build_dir, '.', duplicate = 0 )
			# left out jpeg6, splines (FIXME: I think jpeg6 is not used at all, can trash?)
			lib_objects = []
			for project in [ 'libs/synapse/synapse.vcproj', 'libs/cmdlib/cmdlib.vcproj', 'libs/mathlib/mathlib.vcproj', 'libs/l_net/l_net.vcproj', 'libs/ddslib/ddslib.vcproj', 'libs/picomodel/picomodel.vcproj', 'libs/md5lib/md5lib.vcproj' ]:
				Export( 'project' )
				lib_objects += SConscript( os.path.join( build_dir, 'SConscript.lib' ) )
			Export( 'lib_objects' )
			radiant = SConscript( os.path.join( build_dir, 'SConscript.radiant' ) )
			Default( InstallAs( os.path.join( self.install_directory, 'radiant.bin' ), radiant ) )

			# PIC versions of the libs for the modules
			shlib_objects_extra = {}
			for project in [ 'libs/synapse/synapse.vcproj', 'libs/mathlib/mathlib.vcproj', 'libs/picomodel/picomodel.vcproj', 'libs/cmdlib/cmdlib.vcproj' ]:
				( libpath, libname ) = os.path.split( project )
				libname = os.path.splitext( libname )[0]
				config['shared'] = True
				Export( 'project', 'config' )
				build_dir = os.path.join( 'build', config_name, 'shobjs' )
				BuildDir( build_dir, '.', duplicate = 0 )
				shlib_objects_extra[libname] = SConscript( os.path.join( build_dir, 'SConscript.lib' ) )

			for project in [ 'plugins/vfspk3/vfspk3.vcproj',
					 'plugins/vfspak/vfspak.vcproj',
					 'plugins/vfswad/vfswad.vcproj',
					 'plugins/eclassfgd/fgd.vcproj',
					 'plugins/entity/entity.vcproj',
					 'plugins/image/image.vcproj',
					 'plugins/model/model.vcproj',
					 'plugins/imagepng/imagepng.vcproj',
					 'plugins/imagewal/imagewal.vcproj',
					 'plugins/imagem8/imagem8.vcproj',
					 'plugins/spritemodel/spritemodel.vcproj',
					 'plugins/textool/TexTool.vcproj',
					# 'plugins/sample/sample.vcproj',
					 'plugins/map/map.vcproj',
					 'plugins/mapxml/mapxml.vcproj',
					 'plugins/shaders/shaders.vcproj',
					 'plugins/surface/surface.vcproj',
					 'plugins/surface_ufoai/surface_ufoai.vcproj',
					 'plugins/surface_quake2/surface_quake2.vcproj',
					 'plugins/surface_heretic2/surface_heretic2.vcproj',
					# FIXME Needs splines
					# 'contrib/camera/camera.vcproj',

					# FIXME What is this? Empty dir for me - remove me?
					# 'contrib/patches/patches.vcproj',
					# 'plugins/archivewad/archivewad.vcproj',

					 'contrib/prtview/PrtView.vcproj',
					 'contrib/hydratoolz/hydratoolz.vcproj',
					 'contrib/bobtoolz/bobToolz_gtk.vcproj',
					 'contrib/gtkgensurf/gtkgensurf.vcproj',
					 'contrib/ufoai/ufoai.vcproj',
					 'contrib/bkgrnd2d/bkgrnd2d.vcproj'
				 ]:
				( libpath, libname ) = os.path.split( project )
				libname = os.path.splitext( libname )[0]
				shlib_objects = shlib_objects_extra['synapse']
				if ( libname == 'entity' ):
					shlib_objects += shlib_objects_extra['mathlib']
				elif ( libname == 'model' ):
					shlib_objects += shlib_objects_extra['picomodel']
#				elif ( libname == 'spritemodel' ):
#					shlib_objects += shlib_objects_extra['mathlib']
#				elif ( libname == 'TexTool' ):
#					shlib_objects += shlib_objects_extra['mathlib']
				elif ( libname == 'map' ):
					shlib_objects += shlib_objects_extra['cmdlib']
				Export( 'project', 'shlib_objects' )
				module = SConscript( os.path.join( build_dir, 'SConscript.module' ) )
				Default( InstallAs( os.path.join( self.install_directory, 'modules/%s.so' % libname ), module ) )

	def emit_q3map2( self ):
		settings = self
		for config_name in self.config_selected:
			config = {}
			config['name'] = config_name
			config['shared'] = False
			Export( 'utils', 'settings', 'config' )
			build_dir = os.path.join( 'build', config_name, 'q3map2' )
			BuildDir( build_dir, '.', duplicate = 0 )
			lib_objects = []
			for project in [ 'libs/cmdlib/cmdlib.vcproj', 'libs/mathlib/mathlib.vcproj', 'libs/l_net/l_net.vcproj', 'libs/ddslib/ddslib.vcproj', 'libs/picomodel/picomodel.vcproj', 'libs/md5lib/md5lib_VC9.vcproj' ]:
				Export( 'project' )
				lib_objects += SConscript( os.path.join( build_dir, 'SConscript.lib' ) )
			Export( 'lib_objects' )
			q3map2 = SConscript( os.path.join( build_dir, 'SConscript.q3map2' ) )
			Default( InstallAs( os.path.join( self.install_directory, 'q3map2' ), q3map2 ) )


	def emit( self ):
		try:
			self.target_selected.index( 'radiant' )
		except:
			pass
		else:
			self.emit_radiant()
		try:
			self.target_selected.index( 'q3map2' )
		except:
			pass
		else:
			self.emit_q3map2()

		try:
			self.target_selected.index( 'setup' )
		except:
			pass
		else:
			self.Setup()

	def SetupEnvironment( self, env, config, useGtk = False, useGtkGL = False, useJPEG = False, useZ = False, usePNG = False ):
		env['CC'] = self.cc
		env['CXX'] = self.cxx
		( ret, xml2 ) = commands.getstatusoutput( 'xml2-config --cflags' )
		if ( ret != 0 ):
			print 'xml2-config failed'
			assert( False )
		xml2libs = commands.getoutput( 'xml2-config --libs' )
		env.Append( LINKFLAGS = xml2libs.split( ' ' ) )
		baseflags = [ '-pipe', '-Wall', '-fmessage-length=0', '-fvisibility=hidden', xml2.split( ' ' ) ]
#		baseflags += [ '-m32' ]

		if ( self.platform == 'Darwin' ):
			env.Append( CPPPATH = [ '/Developer/SDKs/MacOSX10.4u.sdk/usr/X11R6/include' ] )

		if ( useGtk ):
			( ret, gtk2 ) = commands.getstatusoutput( 'pkg-config gtk+-2.0 --cflags' )
			if ( ret != 0 ):
				print 'pkg-config gtk+-2.0 failed'
				assert( False )
			baseflags += gtk2.split( ' ' )
			gtk2libs = commands.getoutput( 'pkg-config gtk+-2.0 --libs' )
			env.Append( LINKFLAGS = gtk2libs.split( ' ' ) )
		else:
			# always setup at least glib
			( ret, glib ) = commands.getstatusoutput( 'pkg-config glib-2.0 --cflags' )
			if ( ret != 0 ):
				print 'pkg-config glib-2.0 failed'
				assert( False )
			baseflags += glib.split( ' ' )
			gliblibs = commands.getoutput( 'pkg-config glib-2.0 --libs' )
			env.Append( LINKFLAGS = gliblibs.split( ' ' ) )

		if ( useGtkGL ):
			( ret, gtkgl ) = commands.getstatusoutput( 'pkg-config gtkglext-1.0 --cflags' )
			if ( ret != 0 ):
				print 'pkg-config gtkglext-1.0 failed'
				assert( False )
			baseflags += gtkgl.split( ' ' )
			gtkgllibs = commands.getoutput( 'pkg-config gtkglext-1.0 --libs' )
			env.Append( LINKFLAGS = gtkgllibs.split( ' ' ) )
		if ( useJPEG ):
			env.Append( LIBS = 'jpeg' )
		if ( usePNG ):
			pnglibs = 'png z'
			env.Append( LIBS = pnglibs.split( ' ' ) )
		if ( useZ ):
			env.Append( LIBS = 'z' )

		env.Append( CCFLAGS = baseflags )
		env.Append( CXXFLAGS = baseflags + [ '-fpermissive', '-fvisibility-inlines-hidden' ] )
		env.Append( CPPPATH = [ 'include', 'libs' ] )
		env.Append( CPPDEFINES = [ 'Q_NO_STLPORT' ] )
		if ( config == 'debug' ):
			env.Append( CFLAGS = [ '-g' ] )
			env.Append( CXXFLAGS = [ '-g' ] )
			env.Append( CPPDEFINES = [ '_DEBUG' ] )
		else:
			env.Append( CFLAGS = [ '-O3', '-Winline', '-ffast-math', '-fno-unsafe-math-optimizations', '-fno-strict-aliasing' ] )
			env.Append( CXXFLAGS = [ '-O3', '-Winline', '-ffast-math', '-fno-unsafe-math-optimizations','-fno-strict-aliasing' ] )

	def CheckoutOrUpdate( self, svnurl, path ):
		if ( os.path.exists( path ) ):
                        cmd = [ 'svn', 'update', path ]
		else:
                        cmd = [ 'svn', 'checkout', svnurl, path ]
                print( repr( cmd ) )
                subprocess.check_call( cmd )


	def FetchGamePaks( self, path ):
		for pak in self.setup_packs:
			if ( pak == 'Q3Pack' or pak == 'UrTPack' or pak == 'UFOAIPack' or pak == 'Q2WPack' or pak == 'JAPack' or pak == 'ReactionPack' ):
				svnurl = 'https://zerowing.idsoftware.com/svn/radiant.gamepacks/%s/trunk' % pak
				self.CheckoutOrUpdate( svnurl, os.path.join( path, 'installs', pak ) )

	def Setup( self ):
		try:
			self.setup_platforms.index( 'local' )
		except:
			pass
		else:
			# special case, fetch external paks under the local install directory
			self.FetchGamePaks( self.install_directory )
		# NOTE: unrelated to self.setup_platforms - grab support files and binaries and install them
		if ( self.platform == 'Windows' ):
			depsfile = 'GtkR-deps-1.6-3.zip'
			if ( not os.path.exists( depsfile ) ):
				cmd = [ 'wget', '-N', 'http://zerowing.idsoftware.com/files/radiant/developer/1.6.1/%s' % depsfile ]
				print( repr( cmd ) )
				subprocess.check_call( cmd )

				# extract one directoy above
				f = os.path.abspath( depsfile )
				backup_cwd = os.getcwd()
				os.chdir( os.path.dirname( backup_cwd ) )
				cmd = [ 'unzip', '-o', f ]
				print( repr( cmd ) )
				subprocess.check_call( cmd )
				os.chdir( backup_cwd )

				# copy all the dependent runtime data to the install directory
				srcdir = os.path.dirname( backup_cwd )
				for f in [
					'libxml2/bin/libxml2.dll',
					'gtk2/bin/libglib-2.0-0.dll',
					'gtk2/bin/libgobject-2.0-0.dll',
					'gtk2/bin/libgdk-win32-2.0-0.dll',
					'gtk2/bin/libgtk-win32-2.0-0.dll',
					'gtk2/bin/intl.dll',
					'gtk2/bin/libatk-1.0-0.dll',
					'gtk2/bin/libcairo-2.dll',
					'gtk2/bin/libgdk_pixbuf-2.0-0.dll',
					'gtk2/bin/libgmodule-2.0-0.dll',
					'gtk2/bin/libpng13.dll',
					'gtk2/bin/libpango-1.0-0.dll',
                                        'gtk2/bin/libpangoft2-1.0-0.dll',
					'gtk2/bin/libpangocairo-1.0-0.dll',
					'gtk2/bin/libpangowin32-1.0-0.dll',
					'gtk2/lib/libgtkglext-win32-1.0-0.dll',
					'gtk2/lib/libgdkglext-win32-1.0-0.dll',
					'gtk2/lib/iconv.dll',
                                        'gtk2/zlib1.dll',
#                                       'freetype-dev_2.4.2-1_win32/bin/freetype6.dll',
#                                       'fontconfig-dev_2.8.0-2_win32/bin/libfontconfig-1.dll',
#                                       'expat_2.0.1-1_win32/bin/libexpat-1.dll',
                                        ]:
                                        cmd = [ 'cp', '-v', os.path.join( srcdir, f ), 'install' ]
                                        print( repr( cmd ) )
                                        subprocess.check_call( cmd )
				for d in [
					'gtk2/etc',
					'gtk2/share',
#                                       'fontconfig-dev_2.8.0-2_win32/etc',
#                                       'fontconfig-dev_2.8.0-2_win32/share',
#                                       'freetype-dev_2.4.2-1_win32/share',
					]:
                                        cmd = [ 'cp', '-r', '-v', os.path.join( srcdir, d ), 'install' ]
					print( repr( cmd ) )
					subprocess.check_call( cmd )

# parse the config statement line to produce/update an existing config list
# the configs expose a list of keywords and accepted values, which the engine parses out
class ConfigParser:
	def __init__( self ):
		self.operators = {}

	def _processOp( self, ops ):
		assert( len( ops ) == 1 )
		op = ops.pop()
		if ( op == 'clear' ):
			self.configs = []
			self.current_config = None
		elif ( op == 'pop' ):
			self.configs.pop()
			self.current_config = None
		elif ( op == 'push' ):
			self.configs.append( self.current_config )
			self.current_config = Config()
			self._setupParser( self.current_config )

	def _setupParser( self, c ):
		self.operators = { 'op' : self._processOp }
		c.setupParser( self.operators )

	def _parseStatement( self, s ):
		statement_re = re.compile( '(.*)=(.*)' )
		value_list_re = re.compile( '([^,]*),?' )
		if ( not statement_re.match( s ) ):
			print 'syntax error (statement match): %s' % repr( s )
			return
		statement_split = statement_re.split( s )
		if ( len( statement_split ) != 4 ):
			print 'syntax error (statement split): %s' % repr( s )
			return
		( foo, name, value, bar ) = statement_split
		value_split = value_list_re.split( value )
		if ( len( value_split ) < 2 or len( value_split ) % 2 != 1 ):
			print 'syntax error (value split): %s' % ( repr( value_split ) )
			return
		try:
			value_array = []
			value_split.reverse()
			value_split.pop()
			while ( len( value_split ) != 0 ):
				value_array.append( value_split.pop() )
				value_split.pop()
		except:
			print traceback.print_exception( sys.exc_type, sys.exc_value, sys.exc_traceback )
			print 'syntax error (value to array): %s' % ( repr( value_split ) )
			return

		return ( name, value_array )

	def parseStatements( self, _configs, statements ):
		self.current_config = None
		self.configs = _configs
		if ( self.configs is None ):
			self.configs = []
		for s in statements:

			if ( self.current_config is None ):
				# use a provided config, or create a default one
				if ( len( self.configs ) > 0 ):
					self.current_config = self.configs.pop()
				else:
					self.current_config = Config()
				# setup the operator table for this config
				# NOTE: have that in self._processOp too
				self._setupParser( self.current_config )

			ret = self._parseStatement( s )
			if ( ret is None ):
				print 'stop statement parse at %s' % repr( s )
				break
			( name, value_array ) = ret
			try:
				processor = self.operators[name]
			except:
				print 'unknown operator %s - stop statement parse at %s' % ( repr( name ), repr( s ) )
				break
			processor( value_array )

		if ( not self.current_config is None ):
			self.configs.append( self.current_config )
		# make sure there is at least one config
		if ( len( self.configs ) == 0 ):
			print 'pushing a default config'
			self.configs.append( Config() )
		return self.configs

import unittest

class TestConfigParse( unittest.TestCase ):

	def setUp( self ):
		self.parser = ConfigParser()

	def testBasicParse( self ):
		# test basic config parsing
		# needs to cleanly stop at the first config statement that is not recognized
		configs = self.parser.parseStatements( None, [ 'game=missionpack', 'config=qvm', 'foobar' ] )
		print repr( configs )

	def testMultiParse( self ):
		# multiple configs seperated by commas
		configs = self.parser.parseStatements( None, [ 'target=server,game,cgame' ] )
		print repr( configs )

	def testOp( self ):
		# test the operator for multiple configs
		configs = self.parser.parseStatements( None, [ 'target=core', 'config=release', 'op=push', 'target=game,cgame,ui', 'config=debug' ] )
		print repr( configs )

if __name__ == '__main__':
	unittest.main()
