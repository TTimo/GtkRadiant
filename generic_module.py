# Copyright (C) 2001-2006 William Joseph.
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


import os
import sys

def svnAddText(filename):
  os.system("svn add " + filename);
  os.system("svn ps svn:eol-style native " + filename);

def createHeaderTemplate(filename, name):
  file = open(filename, "wt")
  file.write("\n")
  file.write("#if !defined(INCLUDED_" + name.upper() + "_H)\n")
  file.write("#define INCLUDED_" + name.upper() + "_H\n")
  file.write("\n")
  file.write("#endif\n")

def createCPPTemplate(filename, name):
  file = open(filename, "wt")
  file.write("\n")
  file.write("#include \"" + name + ".h\"\n")
  file.write("\n")
  file.close()

if __name__ == "__main__":
  if not len(sys.argv) == 2:
    print "usage: " + sys.argv[0] + " <module-path>"
    
  path = sys.argv[1]
  name = os.path.split(path)[1];
  
  h = os.path.normpath(path + ".h")
  print "writing " + h;
  createHeaderTemplate(h, name)
  svnAddText(h)

  cpp = os.path.normpath(path + ".cpp")
  print "writing " + cpp;
  createCPPTemplate(cpp, name)
  svnAddText(cpp)
  
