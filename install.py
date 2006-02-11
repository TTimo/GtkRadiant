
import os
import shutil


def copyFile(source, target):
  assert os.path.isfile(source)
  targetFile = target
  if os.path.isdir(targetFile):
    targetFile = os.path.join(target, os.path.basename(source))
  print source, "->", targetFile
  shutil.copyfile(source, targetFile)
  
def copyFileIfExists(source, target):
  if os.path.exists(source):
    copyFile(source, target)
    
def copySvn(source, target):
  assert os.path.isdir(source)
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
  assert os.path.isdir(source)
  assert os.path.isdir(target)
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
