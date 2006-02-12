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


import os.path
import xml.dom
import os
import stat
import string

from xml.dom.minidom import parse

import msi

cwd = os.getcwd()
print("cwd=" + cwd)


def format_guid(guid):
  return "{" + guid.upper() + "}"

def generate_guid():
  os.system("uuidgen > tmp_uuid.txt")
  uuidFile = file("tmp_uuid.txt", "rt")
  guid = format_guid(uuidFile.read(36))
  uuidFile.close()
  os.system("del tmp_uuid.txt")
  return guid
  
def path_components(path):
  directories = []
  remaining = path
  while(remaining != ""):
    splitPath = os.path.split(remaining)
    remaining = splitPath[0]
    directories.append(splitPath[1])
  directories.reverse()
  return directories
      


class Feature:
  def __init__(self, feature, parent, title, desc, display, level, directory, attributes):
    self.feature = feature
    self.parent = parent
    self.title = title
    self.desc = desc
    self.display = display
    self.level = level
    self.directory = directory
    self.attributes = attributes   

class FeatureComponent:
  def __init__(self, feature, component):
    self.feature = feature
    self.component = component
    
class Directory:
  def __init__(self, directory, parent, default):
    self.directory = directory
    self.parent = parent
    self.default = default
    
class Component:
  def __init__(self, name, keypath, directory, attributes):
    self.name = name
    self.keypath = keypath
    self.directory = directory
    self.attributes = attributes
    
class File:
  def __init__(self, file, component, filename, filesize, sequence):
    self.file = file
    self.component = component
    self.filename = filename
    self.filesize = filesize
    self.sequence = sequence
    
class Shortcut:
  def __init__(self, name, directory, component, feature, icon):
    self.name = name
    self.directory = directory
    self.component = component
    self.feature = feature
    self.icon = icon

class ComponentFiles:
  def __init__(self, name, files, directory):
    self.name = name
    self.files = files
    self.directory = directory

class MSIPackage:
  def __init__(self, packageFile):
    self.code = ""
    self.name = ""
    self.version = ""
    self.target = ""
    self.license = ""
    self.cabList = []
    self.featureCount = 0
    self.featureTable = []
    self.featurecomponentsTable = []
    self.componentCache = {}
    self.componentCount = 0
    self.componentTable = {}
    self.directoryTree = {}
    self.directoryCount = 0
    self.directoryTable = []
    self.fileCount = 0
    self.fileTable = []
    self.shortcutCount = 0
    self.shortcutTable = []
    self.createPackage(packageFile)
    
  def addDirectory(self, directoryName, parentKey, directory):
    if(not directory.has_key(directoryName)):
      directoryKey = "d" + str(self.directoryCount)
      self.directoryCount = self.directoryCount + 1
      print("adding msi directory " + directoryKey + " parent=" + parentKey + " name=" + directoryName)
      self.directoryTable.append(Directory(directoryKey, parentKey, directoryKey + "|" + directoryName))
      directory[directoryName] = (directoryKey, {})
    else:
      print("ignored duplicate directory " + directoryName)
    return directory[directoryName]
    
  def parseComponentTree(self, treeElement, parent, directory, directoryPath, component):
    files = []
    for childElement in treeElement.childNodes:
      if (childElement.nodeName == "file"):
        fileName = childElement.getAttribute("name")
        filePath = os.path.join(directoryPath, fileName)
        if(fileName != "" and os.path.exists(filePath)):
          print("found file " + filePath)
          file = (fileName, os.path.getsize(filePath), filePath)
          files.append(file)
        else:
          raise Exception("file not found " + filePath)

      if (childElement.nodeName == "dir"):
        directoryName = childElement.getAttribute("name")
        print("found directory " + directoryName)
        directoryPair = self.addDirectory(directoryName, parent, directory)   
        self.parseComponentTree(childElement, directoryPair[0], directoryPair[1], os.path.join(directoryPath, directoryName), component)
    
    count = len(files) 
    if(count != 0):
      componentKey = "c" + str(self.componentCount)
      self.componentCount = self.componentCount + 1
      msiComponent = ComponentFiles(componentKey, files, parent);
      print("adding msi component " + msiComponent.name + " with " + str(count) + " file(s)")
      component.append(msiComponent)
      
  def parseComponent(self, componentElement, rootPath):
    shortcut = componentElement.getAttribute("shortcut")
    icon = componentElement.getAttribute("icon")
    component = []
    subDirectory = componentElement.getAttribute("subdirectory")
    directoryPair = ("TARGETDIR", self.directoryTree)
    for directoryName in path_components(subDirectory):
      directoryPair = self.addDirectory(directoryName, directoryPair[0], directoryPair[1])
    self.parseComponentTree(componentElement, directoryPair[0], directoryPair[1], rootPath, component)
    component.reverse()
    print("component requires " + str(len(component)) + " msi component(s)")
    return (component, shortcut, icon)
    
  def parseComponentXML(self, filename, rootPath):
    componentDocument = parse(filename)
    print("parsing component file " + filename)
    componentElement = componentDocument.documentElement
    return self.parseComponent(componentElement, rootPath)
    
  def componentForName(self, name, rootPath):
    if(self.componentCache.has_key(name)):
      return self.componentCache[name]
    else:
      component = self.parseComponentXML(name, rootPath)
      self.componentCache[name] = component
      return component
    
  def parseFeature(self, featureElement, parent, index):
    featureName = "ft" + str(self.featureCount)
    self.featureCount = self.featureCount + 1
    title = featureElement.getAttribute("name")
    desc = featureElement.getAttribute("desc")
    print("adding msi feature " + featureName + " title=" + title)
    feature = Feature(featureName, parent, title, desc, index, 1, "TARGETDIR", 8)
    self.featureTable.append(feature)
    featureComponents = {}
    indexChild = 2
    for childElement in featureElement.childNodes:
      if (childElement.nodeName == "feature"):
        self.parseFeature(childElement, featureName, indexChild)
        indexChild = indexChild + 2
      elif (childElement.nodeName == "component"):
        componentName = os.path.normpath(os.path.join(cwd, childElement.getAttribute("name")))
        if(featureComponents.has_key(componentName)):
          raise Exception("feature \"" + title + "\" contains more than one reference to \"" + componentName + "\"")
        featureComponents[componentName] = ""
        componentSource = os.path.normpath(childElement.getAttribute("root"))
        print("found component reference " + componentName)
        componentPair = self.componentForName(componentName, componentSource)
        component = componentPair[0]
        for msiComponent in component:
          print("adding msi featurecomponent " + featureName + " name=" + msiComponent.name)
          self.featurecomponentsTable.append(FeatureComponent(featureName, msiComponent.name))

          if(not self.componentTable.has_key(msiComponent.name)):
            keyPath = ""
            for fileTuple in msiComponent.files:
              fileKey = "f" + str(self.fileCount)
              self.fileCount = self.fileCount + 1
              if(keyPath == ""):
                keyPath = fileKey
                print("component " + msiComponent.name + " keypath=" + keyPath)
              print("adding msi file " + fileKey + " name=" + fileTuple[0] + " size=" + str(fileTuple[1]))
              self.fileTable.append(File(fileKey, msiComponent.name, fileKey + "|" + fileTuple[0], fileTuple[1], self.fileCount))
              self.cabList.append("\"" + fileTuple[2] + "\" " + fileKey + "\n")
            self.componentTable[msiComponent.name] = Component(msiComponent.name, keyPath, msiComponent.directory, 0)
        
        shortcut = componentPair[1]
        if(shortcut != ""):
          shortcutName = "sc" + str(self.shortcutCount)
          self.shortcutCount = self.shortcutCount + 1
          self.shortcutTable.append(Shortcut(shortcutName + "|" + shortcut, "ProductShortcutFolder", component[0].name, featureName, componentPair[2]))
          print("adding msi shortcut " + shortcut)

  def parsePackage(self, packageElement):
    index = 2
    self.code = packageElement.getAttribute("code")
    if(self.code == ""):
      raise Exception("invalid package code")
    self.version = packageElement.getAttribute("version")
    if(self.version == ""):
      raise Exception("invalid package version")
    self.name = packageElement.getAttribute("name")
    if(self.name == ""):
      raise Exception("invalid package name")
    self.target = packageElement.getAttribute("target")
    if(self.target == ""):
      raise Exception("invalid target directory")
    self.license = packageElement.getAttribute("license")
    if(self.license == ""):
      raise Exception("invalid package license agreement")
    for childElement in packageElement.childNodes:
      if (childElement.nodeName == "feature"):
        self.parseFeature(childElement, "", index)
        index = index + 2

  def parsePackageXML(self, filename):
    document = parse(filename)
    print("parsing package file " + filename)
    self.parsePackage(document.documentElement)
    
  def createPackage(self, packageFile):
    self.directoryTable.append(Directory("TARGETDIR", "", "SourceDir"))
    self.directoryTable.append(Directory("ProgramMenuFolder", "TARGETDIR", "."))
    self.directoryTable.append(Directory("SystemFolder", "TARGETDIR", "."))
    self.parsePackageXML(packageFile)
    if(self.shortcutCount != 0):
      self.directoryTable.append(Directory("ProductShortcutFolder", "ProgramMenuFolder", "s0|" + self.name))
  
  def writeFileTable(self, name):
    tableFile = file(name, "wt")
    tableFile.write("File\tComponent_\tFileName\tFileSize\tVersion\tLanguage\tAttributes\tSequence\ns72\ts72\tl255\ti4\tS72\tS20\tI2\ti2\nFile\tFile\n")
    for row in self.fileTable:
      tableFile.write(row.file + "\t" + row.component + "\t" + row.filename + "\t" + str(row.filesize) + "\t" + "" + "\t" + "" + "\t" + "0" + "\t" + str(row.sequence) + "\n")
    
  def writeComponentTable(self, name):
    tableFile = file(name, "wt")
    tableFile.write("Component\tComponentId\tDirectory_\tAttributes\tCondition\tKeyPath\ns72\tS38\ts72\ti2\tS255\tS72\nComponent\tComponent\n")
    for k, row in self.componentTable.iteritems():
      tableFile.write(row.name + "\t" + generate_guid() + "\t" + row.directory + "\t" + str(row.attributes) + "\t" + "" + "\t" + row.keypath + "\n")
    
  def writeFeatureComponentsTable(self, name):
    tableFile = file(name, "wt")
    tableFile.write("Feature_\tComponent_\ns38\ts72\nFeatureComponents\tFeature_\tComponent_\n")
    for row in self.featurecomponentsTable:
      tableFile.write(row.feature + "\t" + row.component + "\n")
    
  def writeDirectoryTable(self, name):
    tableFile = file(name, "wt")
    tableFile.write("Directory\tDirectory_Parent\tDefaultDir\ns72\tS72\tl255\nDirectory\tDirectory\n")
    for row in self.directoryTable:
      tableFile.write(row.directory + "\t" + row.parent + "\t" + row.default + "\n")
    
  def writeFeatureTable(self, name):
    tableFile = file(name, "wt")
    tableFile.write("Feature\tFeature_Parent\tTitle\tDescription\tDisplay\tLevel\tDirectory_\tAttributes\ns38\tS38\tL64\tL255\tI2\ti2\tS72\ti2\nFeature\tFeature\n")
    for row in self.featureTable:
      tableFile.write(row.feature + "\t" + row.parent + "\t" + row.title + "\t" + row.desc + "\t" + str(row.display) + "\t" + str(row.level) + "\t" + row.directory + "\t" + str(row.attributes) + "\n")

  def writeMediaTable(self, name):
    tableFile = file(name, "wt")
    tableFile.write("DiskId\tLastSequence\tDiskPrompt\tCabinet\tVolumeLabel\tSource\ni2\ti2\tL64\tS255\tS32\tS72\nMedia\tDiskId\n")
    tableFile.write("1" + "\t" + str(self.fileCount) + "\t" + "" + "\t" + "#archive.cab" + "\t" + "" + "\t" + "" + "\n")

  def writeShortcutTable(self, name):
    tableFile = file(name, "wt")
    tableFile.write("Shortcut\tDirectory_\tName\tComponent_\tTarget\tArguments\tDescription\tHotkey\tIcon_\tIconIndex\tShowCmd\tWkDir\ns72\ts72\tl128\ts72\ts72\tS255\tL255\tI2\tS72\tI2\tI2\tS72\nShortcut\tShortcut\n")
    for row in self.shortcutTable:
      tableFile.write(row.component + "\t" + row.directory + "\t" + row.name + "\t" + row.component + "\t" + row.feature + "\t" + "" + "\t" + "" + "\t" + "" + "\t" + row.icon + "\t" + "" + "\t" + "" + "\t" + "" + "\n")
  
  def writeRemoveFileTable(self, name):
    tableFile = file(name, "wt")
    tableFile.write("FileKey\tComponent_\tFileName\tDirProperty\tInstallMode\ns72\ts72\tL255\ts72\ti2\nRemoveFile\tFileKey\n")
    count = 0
    for row in self.shortcutTable:
      tableFile.write("rf" + str(count) + "\t" + row.component + "\t" + "" + "\t" + row.directory + "\t" + "2" + "\n")
      count = count + 1
      
  def writeCustomActionTable(self, name):
    tableFile = file(name, "wt")
    tableFile.write("Action\tType\tSource\tTarget\ns72\ti2\tS72\tS255\nCustomAction\tAction\n")
    tableFile.write("caSetTargetDir\t51\tTARGETDIR\t" + self.target)
  
  def writeUpgradeTable(self, name):
    tableFile = file(name, "wt")
    tableFile.write("UpgradeCode\tVersionMin\tVersionMax\tLanguage\tAttributes\tRemove\tActionProperty\ns38\tS20\tS20\tS255\ti4\tS255\ts72\nUpgrade\tUpgradeCode\tVersionMin\tVersionMax\tLanguage\tAttributes\n")
    tableFile.write(format_guid(self.code) + "\t\t" + self.version + "\t1033\t1\t\tRELATEDPRODUCTS")
  
  def writeMSILicense(self, msiName, licenseName):
    if(not os.path.exists(licenseName)):
      raise Exception("file not found: " + licenseName)
    print("license=\"" + licenseName + "\"")
    licenseFile = file(licenseName, "rt")
    text = licenseFile.read(1024)
    rtfString = ""
    while(text != ""):
      rtfString += text
      text = licenseFile.read(1024)
    msiDB = msi.Database(msiName)
    msiDB.setlicense(rtfString[:-1])
    msiDB.commit()

  def writeMSIProperties(self, msiName):
    msiDB = msi.Database(msiName)
    print("ProductCode=" + format_guid(self.code))
    msiDB.setproperty("ProductCode", format_guid(self.code))
    print("UpgradeCode=" + format_guid(self.code))
    msiDB.setproperty("UpgradeCode", format_guid(self.code))
    print("ProductName=" + self.name)
    msiDB.setproperty("ProductName", self.name)
    print("ProductVersion=" + self.version)
    msiDB.setproperty("ProductVersion", self.version)
    msiDB.setproperty("RELATEDPRODUCTS", "")
    msiDB.setproperty("SecureCustomProperties", "RELATEDPRODUCTS")
    msiDB.commit()

  def writeMSI(self, msiTemplate, msiName):
    msiWorkName = "working.msi"
    if(os.system("copy " + msiTemplate + " " + msiWorkName) != 0):
      raise Exception("copy failed")
    os.system("msiinfo " + msiWorkName + " /w 2 /v " + generate_guid() + " /a \"Radiant Community\" /j \"" + self.name + "\" /o \"This installation database contains the logic and data needed to install " + self.name + "\"")

    self.writeMSIProperties(msiWorkName)
    self.writeMSILicense(msiWorkName, self.license)
    
    self.writeFileTable("File.idt")
    os.system("msidb -d " + msiWorkName + " -i -f \"" + cwd + "\" File.idt")
    os.system("del File.idt")
    self.writeComponentTable("Component.idt")
    os.system("msidb -d " + msiWorkName + " -i -f \"" + cwd + "\" Component.idt")
    os.system("del Component.idt")
    self.writeFeatureComponentsTable("FeatureComponents.idt")
    os.system("msidb -d " + msiWorkName + " -i -f \"" + cwd + "\" FeatureComponents.idt")
    os.system("del FeatureComponents.idt")
    self.writeDirectoryTable("Directory.idt")
    os.system("msidb -d " + msiWorkName + " -i -f \"" + cwd + "\" Directory.idt")
    os.system("del Directory.idt")
    self.writeFeatureTable("Feature.idt")
    os.system("msidb -d " + msiWorkName + " -i -f \"" + cwd + "\" Feature.idt")
    os.system("del Feature.idt")
    self.writeMediaTable("Media.idt")
    os.system("msidb -d " + msiWorkName + " -i -f \"" + cwd + "\" Media.idt")
    os.system("del Media.idt")
    self.writeShortcutTable("Shortcut.idt")
    os.system("msidb -d " + msiWorkName + " -i -f \"" + cwd + "\" Shortcut.idt")
    os.system("del Shortcut.idt")
    self.writeRemoveFileTable("RemoveFile.idt")
    os.system("msidb -d " + msiWorkName + " -i -f \"" + cwd + "\" RemoveFile.idt")
    os.system("del RemoveFile.idt")
    self.writeCustomActionTable("CustomAction.idt")
    os.system("msidb -d " + msiWorkName + " -i -f \"" + cwd + "\" CustomAction.idt")
    os.system("del CustomAction.idt")
    self.writeUpgradeTable("Upgrade.idt")
    os.system("msidb -d " + msiWorkName + " -i -f \"" + cwd + "\" Upgrade.idt")
    os.system("del Upgrade.idt")

    cabText = file("archive_files.txt", "wt")
    for cabDirective in self.cabList:
      cabText.write(cabDirective)
    cabText.close()
    if(os.system("cabarc -m LZX:21 n archive.cab @archive_files.txt") != 0):
      raise Exception("cabarc returned error")
    os.system("del archive_files.txt")
    os.system("msidb -d " + msiWorkName + " -a archive.cab")
    os.system("del archive.cab")
    
    print("running standard MSI validators ...")
    if(os.system("msival2 " + msiWorkName + " darice.cub > darice.txt") != 0):
      raise Exception("MSI VALIDATION ERROR: see darice.txt")
    print("running Logo Program validators ...")
    if(os.system("msival2 " + msiWorkName + " logo.cub > logo.txt") != 0):
      raise Exception("MSI VALIDATION ERROR: see logo.txt")
    print("running XP Logo Program validators ...")
    if(os.system("msival2 " + msiWorkName + " XPlogo.cub > XPlogo.txt") != 0):
      raise Exception("MSI VALIDATION ERROR: see XPlogo.txt")
    
    msiNameQuoted = "\"" + msiName + "\""
    if(os.path.exists(os.path.join(".\\", msiName)) and os.system("del " + msiNameQuoted) != 0):
      raise Exception("failed to delete old target")
    if(os.system("rename " + msiWorkName + " " + msiNameQuoted) != 0):
      raise Exception("failed to rename new target")
    
