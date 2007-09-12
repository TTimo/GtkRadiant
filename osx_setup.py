# module for OSX setup
# extract everything into a package and a .info

import commands, os, sys

def system( cmd ):
  sys.stdout.write( cmd )
  ret = commands.getstatusoutput( cmd )
  print ret[1]
  if (ret[0] != 0):
    raise 'command failed'

def do_osx_setup( major, minor, target ):
  rev = '0'
  svnroot = 'https://zerowing.idsoftware.com:666/radiant'
  branch = 'branches/1.4'

  startdir = os.getcwd()
  localdir = 'GtkRadiant-osx-1.' + major + '.' + minor + '-' + rev
  workdir = '/tmp/' + localdir
  system( 'rm -r ' + workdir + ' ; mkdir -p ' + workdir )
  system( 'cp ./setup/osx/build.sh ./setup/setup.patch ' + workdir )

  os.chdir( workdir )
  system( 'cvs -d :pserver:anonymous@cvs.icculus.org:/cvs/cvsroot co loki_setup loki_setupdb' )
  system( 'svn export ' + svnroot + '/GtkRadiant/' + branch + ' GtkRadiant' )
  for i in [ 'WolfPack', 'Q2Pack' ]:
    system( 'svn export ' + svnroot + '/' + i + '/' + branch + ' ' + i )

  os.chdir( '/tmp' )
  system( 'tar cvzf ' + localdir + '.tgz ' + localdir )
  system( 'mv ' + localdir + '.tgz ' + startdir )

  os.chdir( startdir )
  md5sum = commands.getoutput( 'md5sum ' + localdir + '.tgz | cut -b -33' )
  md5sum = md5sum[:-1]
  M4_LINE = ' --define=M4_VER_MAJOR=' + major + ' --define=M4_VER_MINOR=' + minor + ' --define=M4_REV=' + rev + ' --define=M4_SETUP_TARGET=' + target + ' --define=M4_MD5SUM=' + md5sum
  system( 'm4 ' + M4_LINE + ' setup/osx/radiant.info.m4 > radiant-1.' + major + '.' + minor + '-' + rev + '.info' )
