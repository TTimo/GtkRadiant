# Copyright (C) 1999-2006 Id Software, Inc. and contributors.
# For a list of contributors, see the accompanying CONTRIBUTORS file.
# 
# This file is part of GtkRadiant.
# 
# GtkRadiant is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# GtkRadiant is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GtkRadiant; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


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
#   file pointed to by $RADIANT_ABOUTMSG if exists
#   else include/aboutmsg.default
# ouput:
#   include/aboutmsg.h

import sys, re, string, os

import svn

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
def radiant_makeversion(append_about, root = os.getcwd()):
  (line, major, minor) = get_version()
  f = open(os.path.join(root, 'include/version.h'), 'w')
  f.write('// generated header, see makeversion.py\n')
  f.write('#define RADIANT_VERSION "%s"\n' % line)
  f.write('#define RADIANT_MINOR_VERSION "%s"\n' % minor)
  f.write('#define RADIANT_MAJOR_VERSION "%s"\n' % major)
  f.close()
  f = open(os.path.join(root, 'include/RADIANT_MINOR'), 'w')
  f.write(minor)
  f.close()
  f = open(os.path.join(root, 'include/RADIANT_MAJOR'), 'w')
  f.write(major)
  f.close()
  f = open(os.path.join(root, 'include/version'), 'w')
  f.write(line)
  f.close()
  # aboutmsg
  aboutfile = os.path.join(root, 'include/aboutmsg.default')
  if ( os.environ.has_key('RADIANT_ABOUTMSG') ):
    aboutfile = os.environ['RADIANT_ABOUTMSG']
  line = None
  if os.path.isfile(aboutfile):
    sys.stdout.write("about message is in %s\n" % aboutfile)
    f = open(aboutfile, 'r')
    line = f.readline()
    f.close()
  else:
    line = "Custom build based on revision " + str(svn.getRevision(os.getcwd()))
  # optional additional message
  if ( not append_about is None ):
    line += append_about
  sys.stdout.write("about: %s\n" % line)
  f = open(os.path.join(root, 'include/aboutmsg.h'), 'w')
  f.write('// generated header, see makeversion.py\n')
  f.write('#define RADIANT_ABOUTMSG "%s"\n' % line)
  f.close()

# can be used as module (scons build), or by direct call
if __name__ == '__main__':
  root = os.path.dirname(__file__)
  if not os.path.isabs(root):
    root = os.getcwd()
  radiant_makeversion(None, root)
