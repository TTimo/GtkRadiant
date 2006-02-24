# Copyright (C) 2001-2006 William Joseph.
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

"""
Builds the ./install directory.

Copies files from various locations:
./setup/data/tools/
./games/<gamepack>/
../<library>/<library>.dll
./include/version.default is used to generate RADIANT_MAJOR and RADIANT_MINOR
"""

import os
import shutil

def assertMessage(condition, message):
  if not condition:
    raise Exception(message)
    
def copyFile(source, target):
  assertMessage(os.path.isfile(source), "failed to find file: " + source)
  print source, "->", target
  shutil.copy2(source, target)
  
def copyFileIfExists(source, target):
  if os.path.exists(source):
    copyFile(source, target)
    
def copySvn(source, target):
  assertMessage(os.path.isdir(source), "failed to find directory: " + source)
  if not os.path.exists(target):
    os.mkdir(target)
  for name in os.listdir(source):
    absolute = os.path.join(source, name)
    absTarget = os.path.join(target, name)
    if os.path.isdir(absolute):
      if name != ".svn":
        copySvn(absolute, absTarget)
    else:
      copyFile(absolute, absTarget)
      
def copyGame(source, game, target):
  assertMessage(os.path.isdir(source), "failed to find directory: " + source)
  assertMessage(os.path.isdir(target), "failed to find directory: " + target)
  root = os.path.join(source, os.path.normpath(game[0]))
  if os.path.exists(root):
    gamename = game[1] + ".game"
    copySvn(os.path.join(root, gamename), os.path.join(target, gamename))
    gamesDir = os.path.join(target, "games")
    if not os.path.exists(gamesDir):
      os.mkdir(gamesDir)
    copyFile(os.path.join(root, "games", gamename), os.path.join(gamesDir, gamename))  
  
thisDir = os.path.dirname(__file__)
gamesRoot = os.path.join(thisDir, "games")
installRoot = os.path.join(thisDir, "install")

if not os.path.exists(installRoot):
  os.mkdir(installRoot)
  
# copy generic data
copySvn(os.path.join(thisDir, os.path.normpath("setup/data/tools")), installRoot)

# root, gamename
games = [
  ("Doom3Pack/tools", "doom3"),
  ("ETPack", "et"),
  ("HalfLifePack", "hl"),
  ("Her2Pack", "heretic2"),
  ("JAPack/Tools", "ja"),
  ("JK2Pack", "jk2"),
  ("Q1Pack", "q1"),
  ("Q2Pack", "q2"),
  ("Q3Pack/tools", "q3"),
  ("Q4Pack/tools", "q4"),
  ("Sof2Pack", "sof2"),
  ("STVEFPack", "stvef"),
  ("WolfPack/bin", "wolf")
]

# copy games
for game in games:
  copyGame(gamesRoot, game, installRoot)

# copy win32 dlls
gtk2Root = os.path.normpath(os.path.join(thisDir, "../gtk2-2.4"))
if os.path.exists(gtk2Root):
  copySvn(os.path.join(gtk2Root, "install"), installRoot)
  
libxml2 = os.path.normpath(os.path.join(thisDir, "../libxml2-2.6/win32/install/libxml2.dll"))
copyFileIfExists(libxml2, installRoot)
  
libpng = os.path.normpath(os.path.join(thisDir, "../libpng-1.2/lib/libpng13.dll"))
copyFileIfExists(libpng, installRoot)
  
libmhash = os.path.normpath(os.path.join(thisDir, "../mhash-0.9/win32/libmhash/Release/libmhash.dll"))
copyFileIfExists(libmhash, installRoot)
  
zlib = os.path.normpath(os.path.join(thisDir, "../zlib1-1.2/zlib1.dll"))
copyFileIfExists(zlib, installRoot)

msvcr71 = os.path.normpath(os.path.join(thisDir, "../msvc_redist/msvcr71.dll"))
copyFileIfExists(msvcr71, installRoot)

dbghelp = os.path.normpath(os.path.join(thisDir, "../msvc_redist/dbghelp.dll"))
copyFileIfExists(dbghelp, installRoot)

# create version files
version = open(os.path.join(thisDir, "include/version.default"), "rt").readline().split(".")
assertMessage(len(version) == 3, "failed to parse include/version.default")
open(os.path.join(thisDir, "install/RADIANT_MAJOR"), "wt").write(str(version[1]))
open(os.path.join(thisDir, "install/RADIANT_MINOR"), "wt").write(str(version[2]))
