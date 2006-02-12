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

import datetime
from installer import MSIPackage

def format_date_today():
  return str(datetime.date.today())

package = MSIPackage("gtkradiant-1.5.0.xml")
package.writeMSI("template.msi", "GtkRadiant-1.5.0-" + format_date_today() + ".msi")

#package = MSIPackage("q3a_example_maps.xml")
#package.writeMSI("template.msi", "Q3A-Example-Maps-" + format_date_today() + ".msi")

#package = MSIPackage("wolf_example_maps.xml")
#package.writeMSI("template.msi", "Wolf-Example-Maps-" + format_date_today() + ".msi")

#package = MSIPackage("ja_example_maps.xml")
#package.writeMSI("template.msi", "JA-Example-Maps-" + format_date_today() + ".msi")

#package = MSIPackage("et_example_maps.xml")
#package.writeMSI("template.msi", "ET-Example-Maps-" + format_date_today() + ".msi")

#package = MSIPackage("jk2_example_maps.xml")
#package.writeMSI("template.msi", "JK2-Example-Maps-" + format_date_today() + ".msi")

#package = MSIPackage("sof2_example_maps.xml")
#package.writeMSI("template.msi", "SoF2-Example-Maps-" + format_date_today() + ".msi")

#package = MSIPackage("q2_example_maps.xml")
#package.writeMSI("template.msi", "Q2-Example-Maps-" + format_date_today() + ".msi")

#package = MSIPackage("her2_example_maps.xml")
#package.writeMSI("template.msi", "Heretic2-Example-Maps-" + format_date_today() + ".msi")

#package = MSIPackage("hl_example_maps.xml")
#package.writeMSI("template.msi", "HalfLife-Example-Maps-" + format_date_today() + ".msi")

#package = MSIPackage("ef_example_maps.xml")
#package.writeMSI("template.msi", "STVEF-Example-Maps-" + format_date_today() + ".msi")
