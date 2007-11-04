# version and about message management
# NOTE: this module is meant to be used on all platforms, it is not SCons centric

# version:
# input:
#   include/version.default
# output:
#   include/version.h include/RADIANT_MAJOR include/RADIANT_MINOR
#   the header is used by C/C++ code, the straight text file by setup

# about message
#   for non-official builds, we have a default message
#   otherwise, use environment variable $RADIANT_ABOUTMSG
# input:
#   include/aboutmsg.default
#   or file pointed to by $RADIANT_ABOUTMSG if exists
# ouput:
#   include/aboutmsg.h

import sys, re, string, os

def get_version():
  # version
  f = open('include/version.default', 'r')
  buffer = f.read()
  line = string.split(buffer, '\n')[0]
  f.close()
  sys.stdout.write("version: %s\n" % line)
  exp = re.compile('^1\\.([^\\.]*)\\.([0-9]*)')
  (major, minor) = exp.findall(line)[0]
  sys.stdout.write("minor: %s major: %s\n" % (minor, major))
  return (line, major, minor)  

# you can pass an optional message to append to aboutmsg
def radiant_makeversion(append_about):
  (line, major, minor) = get_version()
  f = open('include/version.h', 'w')
  f.write('// generated header, see makeversion.py\n')
  f.write('#define RADIANT_VERSION "%s"\n' % line)
  f.write('#define RADIANT_MINOR_VERSION "%s"\n' % minor)
  f.write('#define RADIANT_MAJOR_VERSION "%s"\n' % major)
  f.close()
  f = open('include/RADIANT_MINOR', 'w')
  f.write(minor)
  f.close()
  f = open('include/RADIANT_MAJOR', 'w')
  f.write(major)
  f.close()
  f = open('include/version', 'w')
  f.write(line)
  f.close()
  # aboutmsg
  aboutfile = 'include/aboutmsg.default'
  if ( os.environ.has_key('RADIANT_ABOUTMSG') ):
    aboutfile = os.environ['RADIANT_ABOUTMSG']
  sys.stdout.write("about message is in %s\n" % aboutfile)
  f = open(aboutfile, 'r')
  buffer = f.read()
  line = string.split(buffer, '\n')[0]
  f.close()
  # optional additional message
  if ( not append_about is None ):
    line += append_about
  sys.stdout.write("about: %s\n" % line)
  f = open('include/aboutmsg.h', 'w')
  f.write('// generated header, see makeversion.py\n')
  f.write('#define RADIANT_ABOUTMSG "%s"\n' % line)
  f.close()

# can be used as module (scons build), or by direct call
if __name__ == '__main__':
  radiant_makeversion(None)
