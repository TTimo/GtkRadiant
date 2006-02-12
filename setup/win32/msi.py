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


import msiquery

class Record:
  def __init__(self, record):
    self.record = record
    
  def setstring(self, index, string):
    self.record.setstring(index, string)

class View:
  def __init__(self, view):
    self.view = view
    
  def fetch(self):
    record = self.view.fetch()
    if(record == None):
      raise Exception("no records available")
    return Record(record)
    
  def update(self, record):
    self.view.update(record.record)

class Database:
  def __init__(self, name):
    self.msiDB = msiquery.MsiDB(name)

  def commit(self):
    result = self.msiDB.commit()
    if(result != 0):
      raise Exception("msi commit failed: error " + str(result))
    
  def openview(self, query):
    view = self.msiDB.openview(query)
    if(view == None):
      raise Exception("msi openview failed")
    return View(view);

  def setproperty(self, propertyName, propertyValue):
    query = "UPDATE `Property` SET `Property`.`Value`='" + propertyValue + "' WHERE `Property`.`Property`='" + propertyName + "'" 
    self.openview(query)

  def setlicense(self, rtfString):
    view = self.openview("SELECT `Control`.`Text` FROM `Control` WHERE `Control`.`Control`='AgreementText'")
    record = view.fetch();
    record.setstring(1, rtfString)
    view.update(record)

