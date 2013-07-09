import sys, os, traceback, platform, re, commands, platform, subprocess
import urllib2, zipfile, shutil, pprint

if __name__ != '__main__':
    from SCons.Script import *

import utils

GTK_PREFIX='gtk-2.24.10'
GTK64_PREFIX='gtk-2.22.1_win64'

class Config:
    # aliases
    # 'all' -> for each choices
    # 'gamecode' for the targets, 'game' 'cgame' 'ui'

    def __init__( self ):
        # initialize defaults
        self.target_selected = [ 'radiant', 'q3map2', 'q3data', 'setup' ]
        self.config_selected = [ 'release' ]
        # those are global to each config
        self.platform = platform.system()
        self.cc = 'gcc'
        self.cxx = 'g++'
        self.packs_directory = 'packs'
        self.install_directory = 'install'

        # platforms for which to assemble a setup
        self.setup_platforms = [ 'local', 'x86', 'x64', 'win32' ]
        # paks to assemble in the setup
        self.setup_packs = [ 'Q3Pack', 'UrTPack', 'ETPack', 'QLPack', 'Q2Pack', 'Q2WPack', 'JAPack', 'STVEFPack' ]

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
        
    def _processPacksDir(self, ops ):
        self.packs_directory = os.path.normpath( os.path.expanduser( ops[0] ) )

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
        operators['packs_directory'] = self._processPacksDir
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
            VariantDir( build_dir, '.', duplicate = 0 )
            lib_objects = []
            for project in [ 'libs/synapse/synapse.vcproj', 'libs/cmdlib/cmdlib.vcproj', 'libs/mathlib/mathlib.vcproj', 'libs/l_net/l_net.vcproj' ]:
                Export( 'project' )
                lib_objects += SConscript( os.path.join( build_dir, 'SConscript.lib' ) )
            Export( 'lib_objects' )
            radiant = SConscript( os.path.join( build_dir, 'SConscript.radiant' ) )
            Default( InstallAs( os.path.join( self.install_directory, 'radiant.bin' ), radiant ) )

            # PIC versions of the libs for the modules
            shlib_objects_extra = {}
            for project in [ 'libs/synapse/synapse.vcproj', 'libs/mathlib/mathlib.vcproj', 'libs/picomodel/picomodel.vcproj', 'libs/cmdlib/cmdlib.vcproj', 'libs/splines/splines.vcproj' ]:
                ( libpath, libname ) = os.path.split( project )
                libname = os.path.splitext( libname )[0]
                config['shared'] = True
                Export( 'project', 'config' )
                build_dir = os.path.join( 'build', config_name, 'shobjs' )
                VariantDir( build_dir, '.', duplicate = 0 )
                shlib_objects_extra[libname] = SConscript( os.path.join( build_dir, 'SConscript.lib' ) )

            for project in [ 'plugins/vfsqlpk3/vfsqlpk3.vcproj',
                     'plugins/vfspk3/vfspk3.vcproj',
                     'plugins/vfspak/vfspak.vcproj',
                     'plugins/vfswad/vfswad.vcproj',
                     'plugins/eclassfgd/fgd.vcproj',
                     'plugins/entity/entity.vcproj',
                     'plugins/image/image.vcproj',
                     'plugins/model/model.vcproj',
                     'plugins/imagepng/imagepng.vcproj',
                     'plugins/imagewal/imagewal.vcproj',
                     'plugins/imagehl/imagehl.vcproj',
                     'plugins/imagem8/imagem8.vcproj',
                     'plugins/spritemodel/spritemodel.vcproj',
                     'plugins/textool/textool.vcproj',
                     'plugins/map/map.vcproj',
                     'plugins/mapxml/mapxml.vcproj',
                     'plugins/shaders/shaders.vcproj',
                     'plugins/surface/surface.vcproj',
                     'plugins/surface_idtech2/surface_idtech2.vcproj',
                     'contrib/camera/camera.vcproj',
                     'contrib/prtview/prtview.vcproj',
                     'contrib/hydratoolz/hydratoolz.vcproj',
                     'contrib/bobtoolz/bobtoolz.vcproj',
                     'contrib/gtkgensurf/gtkgensurf.vcproj',
                     'contrib/bkgrnd2d/bkgrnd2d.vcproj'
                 ]:
                ( libpath, libname ) = os.path.split( project )
                libname = os.path.splitext( libname )[0]
                # The old code assigned shlib_objects to shlib_objects_extra['synapse'],
                # and this resulted in a non-copy.  Stuff is added to shlib_objects below.
                # So we need the explicit copy so we don't modify shlib_objects_extra['synapse'].
                shlib_objects = shlib_objects_extra['synapse'][:]
                if ( libname == 'camera' ):
                    shlib_objects += shlib_objects_extra['splines']
                elif ( libname == 'entity' ):
                    shlib_objects += shlib_objects_extra['mathlib']
                elif ( libname == 'map' ):
                    shlib_objects += shlib_objects_extra['cmdlib']
                elif ( libname == 'model' ):
                    shlib_objects += shlib_objects_extra['picomodel']
                    shlib_objects += shlib_objects_extra['mathlib']
                elif ( libname == 'spritemodel' ):
                    shlib_objects += shlib_objects_extra['mathlib']
                elif ( libname == 'textool' ):
                    shlib_objects += shlib_objects_extra['mathlib']
                elif ( libname == 'bobtoolz' ):
                    shlib_objects += shlib_objects_extra['mathlib']
                    shlib_objects += shlib_objects_extra['cmdlib']
                Export( 'project', 'shlib_objects' )
                module = SConscript( os.path.join( build_dir, 'SConscript.module' ) )
                Default( InstallAs( os.path.join( self.install_directory, 'modules/%s.so' % libname ), module ) )

    def emit_q3map2( self, urt = False ):
        if ( urt ):
            compiler_name = 'q3map2_urt'
            sconscript_name = 'SConscript.q3map2.urt'
        else:
            compiler_name = 'q3map2'
            sconscript_name = 'SConscript.q3map2'
        settings = self
        for config_name in self.config_selected:
            config = {}
            config['name'] = config_name
            config['shared'] = False
            Export( 'utils', 'settings', 'config' )                        
            build_dir = os.path.join( 'build', config_name, compiler_name )
            VariantDir( build_dir, '.', duplicate = 0 )
            lib_objects = []
            for project in [ 'tools/quake3/common/quake3-common.vcproj', 'libs/mathlib/mathlib.vcproj', 'libs/l_net/l_net.vcproj', 'libs/ddslib/ddslib.vcproj', 'libs/picomodel/picomodel.vcproj', 'libs/md5lib/md5lib.vcproj' ]:
                Export( 'project' )
                lib_objects += SConscript( os.path.join( build_dir, 'SConscript.lib' ) )
            Export( 'lib_objects' )

            q3map2 = SConscript( os.path.join( build_dir, sconscript_name ) )
            Default( InstallAs( os.path.join( self.install_directory, compiler_name ), q3map2 ) )

    def emit_q3data( self ):
        settings = self
        for config_name in self.config_selected:
            config = {}
            config['name'] = config_name
            config['shared'] = False
            Export( 'utils', 'settings', 'config' )
            build_dir = os.path.join( 'build', config_name, 'q3data' )
            VariantDir( build_dir, '.', duplicate = 0 )
            lib_objects = []
            for project in [ 'libs/mathlib/mathlib.vcproj', 'libs/l_net/l_net.vcproj', 'libs/ddslib/ddslib.vcproj' ]:
                    Export( 'project' )
                    lib_objects += SConscript( os.path.join( build_dir, 'SConscript.lib' ) )
            Export( 'lib_objects' )
            q3data = SConscript( os.path.join( build_dir, 'SConscript.q3data' ) )
            Default( InstallAs( os.path.join( self.install_directory, 'q3data' ), q3data ) )

    def emit( self ):
        if 'radiant' in self.target_selected:
                self.emit_radiant()
        if 'q3map2' in self.target_selected:
                self.emit_q3map2( urt = False )
                self.emit_q3map2( urt = True )
        if 'q3data' in self.target_selected:
                self.emit_q3data()
        if 'setup' in self.target_selected:
                self.Setup()

        if ( self.platform == 'Linux' ):
            finish_command = Command( 'finish', [], self.FinishBuild )
            Depends( finish_command, DEFAULT_TARGETS )
            Default( finish_command )

    def SetupEnvironment( self, env, config, useGtk = False, useGtkGL = False, useJPEG = False, useZ = False, usePNG = False ):
        env['CC'] = self.cc
        env['CXX'] = self.cxx
        ( ret, xml2 ) = commands.getstatusoutput( 'xml2-config --cflags' )
        if ( ret != 0 ):
            print 'xml2-config failed'
            assert( False )
        xml2libs = commands.getoutput( 'xml2-config --libs' )
        env.ParseConfig( 'xml2-config --libs' )
        baseflags = [ '-pipe', '-Wall', '-fmessage-length=0', '-fvisibility=hidden', xml2.split( ' ' ) ]

        if ( useGtk ):
            env.ParseConfig( 'pkg-config gtk+-2.0 --cflags --libs' )
            env.ParseConfig( 'pkg-config x11 --cflags --libs' )
        else:
            # always setup at least glib
            env.ParseConfig( 'pkg-config glib-2.0 --cflags --libs' )

        if ( useGtkGL ):
            env.ParseConfig( 'pkg-config glu --cflags --libs' )
            env.ParseConfig( 'pkg-config gtkglext-1.0 --cflags --libs' )
        if ( useJPEG ):
            env.Append( LIBS = 'jpeg' )
        if ( usePNG ):
            pnglibs = 'png'
            env.Append( LIBS = pnglibs.split( ' ' ) )
            env.ParseConfig( 'pkg-config zlib --cflags --libs' )
            if ( useZ ):
                env.ParseConfig( 'pkg-config zlib --cflags --libs' )

        env.Append( CCFLAGS = baseflags )
        env.Append( CXXFLAGS = baseflags + [ '-fpermissive', '-fvisibility-inlines-hidden' ] )
        env.Append( CPPPATH = [ 'include', 'libs' ] )
        env.Append( CPPDEFINES = [ 'Q_NO_STLPORT' ] )
        if ( config == 'debug' ):
            env.Append( CFLAGS = [ '-g' ] )
            env.Append( CXXFLAGS = [ '-g' ] )
            env.Append( CPPDEFINES = [ '_DEBUG' ] )
        else:
            env.Append( CFLAGS = [ '-O2', '-fno-strict-aliasing' ] )
            env.Append( CXXFLAGS = [ '-O2', '-fno-strict-aliasing' ] )

            # this lets us catch libjpg and libpng libraries that we put in the same directory as radiant.bin
            env.Append( LINKFLAGS = '-Wl,-rpath,.' )
            
        # On Mac, we pad headers so that we may rewrite them for packaging
        if ( self.platform == 'Darwin' ) :
            env.Append( LINKFLAGS = [ '-headerpad_max_install_names' ] )

    def SvnCheckoutOrUpdate( self, svn_url, working_copy ):
        if ( os.path.exists( working_copy ) ):
            cmd = [ 'svn', 'update', working_copy ]
        else:
            cmd = [ 'svn', 'checkout', svn_url, working_copy ]
        print( repr( cmd ) )
        subprocess.check_call( cmd )
        
    def SvnExport( self, working_copy, dest ):
        cmd = [ 'svn', 'export', '--force', working_copy, dest ]
        print( repr( cmd ) )
        
        subprocess.check_call( cmd )

    def FetchGamePacks( self, packs_dir, install_dir ):
        if ( not os.path.exists( packs_dir ) ):
            os.mkdir( packs_dir )
        
        if ( not os.path.exists( install_dir ) ):
            os.mkdir( install_dir )
            
        for pack in self.setup_packs:
            svn_url = 'svn://svn.icculus.org/gtkradiant-gamepacks/%s/trunk' % pack
            working_copy = os.path.join( packs_dir, pack)
            
            self.SvnCheckoutOrUpdate( svn_url, working_copy )
            
            dest_dir = os.path.join( install_dir, pack)
            
            self.SvnExport( working_copy, dest_dir )
        
    def CopyTree( self, src, dst):
        for root, dirs, files in os.walk( src ):
            target_dir = os.path.join( dst, root[root.find( '/' )+1:] )
            print ( target_dir )
            if ( not os.path.exists( target_dir ) ):
                os.mkdir( target_dir )

            for file in files:
                shutil.copy( os.path.join( root, file ), os.path.join( target_dir, file ) )

    def Setup( self ):
        try:
            self.setup_platforms.index( 'local' )
        except:
            pass
        else:
            # Fetch remote game packs and install them
            self.FetchGamePacks( self.packs_directory, os.path.join( self.install_directory, 'installs' ) )
        # NOTE: unrelated to self.setup_platforms - grab support files and binaries and install them
        if ( self.platform == 'Windows' ):
            backup_cwd = os.getcwd()
            for lib_archive in [
                'STLport-5.2.1-GtkRadiant.zip',
                'gtk-bundle-2.24.10-GtkRadiant.zip',
                'gtk-bundle-2.22.1-win64-GtkRadiant.zip',
                'jpeg-9-GtkRadiant.zip',
                'libxml2-2.9.1-GtkRadiant.zip',
                'gtkglext-1.2.0-3-win32.zip',
                ]:
                if ( not os.path.exists( lib_archive ) ):
                    print( 'downloading %s' % lib_archive )
                    archive_web_request = urllib2.urlopen( 'http://gtkradiant.s3-website-us-east-1.amazonaws.com/%s' % lib_archive )
                    archive_File = open( lib_archive, 'wb' )
                    while True:
                        data = archive_web_request.read( 1048576 ) # read 1mb at a time
                        if not data:
                            break
                        archive_File.write( data )

                    archive_web_request.close()
                    archive_File.close()

                    print( 'unpacking %s' % lib_archive )
                    lib_archive_path = os.path.abspath( lib_archive )
                    os.chdir( os.path.dirname( backup_cwd ) )

                    archive_Zip = zipfile.ZipFile( lib_archive_path, 'r' )
                    archive_Zip.extractall()
                    archive_Zip.close()

                    os.chdir( backup_cwd )

            # copy all the dependent runtime data to the install directory
            srcdir = os.path.dirname( backup_cwd )
            for dll in [
                '%s/bin/freetype6.dll' % GTK_PREFIX,
                '%s/bin/intl.dll' % GTK_PREFIX,
                '%s/bin/libasprintf-0.dll' % GTK_PREFIX,
                '%s/bin/libatk-1.0-0.dll' % GTK_PREFIX,
                '%s/bin/libcairo-2.dll' % GTK_PREFIX,
                '%s/bin/libexpat-1.dll' % GTK_PREFIX,
                '%s/bin/libfontconfig-1.dll' % GTK_PREFIX,
                '%s/bin/libgailutil-18.dll' % GTK_PREFIX,
                '%s/bin/libgcc_s_dw2-1.dll' % GTK_PREFIX,
                '%s/bin/libgdk-win32-2.0-0.dll' % GTK_PREFIX,
                '%s/bin/libgdk_pixbuf-2.0-0.dll' % GTK_PREFIX,
                '%s/bin/libgio-2.0-0.dll' % GTK_PREFIX,
                '%s/bin/libglib-2.0-0.dll' % GTK_PREFIX,
                '%s/bin/libgmodule-2.0-0.dll' % GTK_PREFIX,
                '%s/bin/libgobject-2.0-0.dll' % GTK_PREFIX,
                '%s/bin/libgthread-2.0-0.dll' % GTK_PREFIX,
                '%s/bin/libgtk-win32-2.0-0.dll' % GTK_PREFIX,
                '%s/bin/libpango-1.0-0.dll' % GTK_PREFIX,
                '%s/bin/libpangocairo-1.0-0.dll' % GTK_PREFIX,
                '%s/bin/libpangoft2-1.0-0.dll' % GTK_PREFIX,
                '%s/bin/libpangowin32-1.0-0.dll' % GTK_PREFIX,
                '%s/bin/libpng14-14.dll' % GTK_PREFIX,
                '%s/bin/zlib1.dll' % GTK_PREFIX,
                '%s/lib/GNU.Gettext.dll' % GTK_PREFIX,
                '%s/lib/gtk-2.0/2.10.0/engines/libpixmap.dll' % GTK_PREFIX,
                '%s/lib/gtk-2.0/2.10.0/engines/libwimp.dll' % GTK_PREFIX,
                '%s/lib/gtk-2.0/modules/libgail.dll' % GTK_PREFIX,
                'gtkglext-1.2.0/bin/libgdkglext-win32-1.0-0.dll',
                'gtkglext-1.2.0/bin/libgtkglext-win32-1.0-0.dll',
                ]:
                shutil.copy( os.path.join( srcdir, dll ), 'install' )

            for extra in [
                '%s/etc' % GTK_PREFIX,
                '%s/share' % GTK_PREFIX,
                'gtkglext-1.2.0/share',
                ]:
                self.CopyTree( os.path.join( srcdir, extra ), 'install' )
            
            try:
                os.mkdir( 'install/x64' )
            except:
                pass # assume 'already exists'
            for x64_dll in [
                '%s/bin/libpng14-14.dll' % GTK64_PREFIX,
                '%s/bin/libglib-2.0-0.dll' % GTK64_PREFIX,
                '%s/bin/libintl-8.dll' % GTK64_PREFIX,
                '%s/bin/zlib1.dll' % GTK64_PREFIX,
                ]:
                shutil.copy( os.path.join( srcdir, x64_dll ), 'install/x64' )

    def FinishBuild( self, target, source, env ):
        print( 'Lookup and bundle the PNG and JPEG libraries' )
        # radiant.bin doesn't link to jpeg lib directly, grab that from a module
        # Python 2.7 only!
        #module_ldd = subprocess.check_output( 'ldd -r install/modules/image.so', shell = True )
        p = subprocess.Popen( 'ldd -r install/modules/image.so', shell = True, stdout = subprocess.PIPE )
        module_ldd = p.communicate()[0]
#                print( module_ldd )

        def find_library( output, libname ):
            print output
            print libname
            match = filter( lambda l : l.find( libname ) != -1, output.split( '\n' ) )[0]
            return re.split( '.*=> (.*) .*', match )[1]

        jpeg_path = find_library( module_ldd, 'libjpeg' )
        print( 'JPEG library: %s' % repr( jpeg_path ) )
        png_path = find_library( module_ldd, 'libpng' )
        print( 'PNG  library: %s' % repr( png_path ) )

        shutil.copy( jpeg_path, 'install' )
        shutil.copy( png_path, 'install' )

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
