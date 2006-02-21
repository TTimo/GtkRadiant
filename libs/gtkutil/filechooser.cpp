/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filechooser.h"

#include "ifiletypes.h"

#include <list>
#include <vector>
#include <gtk/gtkwidget.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkfilechooser.h>
#include <gtk/gtkfilechooserdialog.h>
#include <gtk/gtkstock.h>

#include "string/string.h"
#include "stream/stringstream.h"
#include "container/array.h"
#include "os/path.h"
#include "os/file.h"

#include "messagebox.h"


struct filetype_pair_t
{
  filetype_pair_t()
    : m_moduleName("")
  {
  }
  filetype_pair_t(const char* moduleName, filetype_t type)
    : m_moduleName(moduleName), m_type(type)
  {
  }
  const char* m_moduleName;
  filetype_t m_type;
};

class FileTypeList : public IFileTypeList
{
  struct filetype_copy_t
  {
    filetype_copy_t(const filetype_pair_t& other)
      : m_moduleName(other.m_moduleName), m_name(other.m_type.name), m_pattern(other.m_type.pattern)
    {
    }
    CopiedString m_moduleName;
    CopiedString m_name;
    CopiedString m_pattern;
  };

  typedef std::list<filetype_copy_t> Types;
  Types m_types;
public:

  typedef Types::const_iterator const_iterator;
  const_iterator begin() const
  {
    return m_types.begin();
  }
  const_iterator end() const
  {
    return m_types.end();
  }

  std::size_t size() const
  {
    return m_types.size();
  }

  void addType(const char* moduleName, filetype_t type)
  {
    m_types.push_back(filetype_pair_t(moduleName, type));
  }
};

#ifdef WIN32

class Win32Filters
{
  const FileTypeList& m_types;
  Array<char> m_filters;
public:
  Win32Filters(const FileTypeList& typeList) : m_types(typeList)
  {
    std::size_t len = 0;
    for(FileTypeList::const_iterator i = m_types.begin(); i != m_types.end(); ++i)
    {
      len = len + strlen((*i).m_name.c_str()) + strlen((*i).m_pattern.c_str()) * 2 + 5;
    }
    m_filters.resize(len + 1); // length + null char
    char *w = m_filters.data();
    for(FileTypeList::const_iterator i = m_types.begin(); i != m_types.end(); ++i)
    {
      for(const char *r = (*i).m_name.c_str(); *r!='\0'; r++, w++)
      {
        *w = *r;
      }
      *w++ = ' ';
      *w++ = '(';
      for(const char *r = (*i).m_pattern.c_str(); *r!='\0'; r++, w++)
      {
        *w = *r;
      }
      *w++ = ')';
      *w++ = '\0';
      for(const char *r = (*i).m_pattern.c_str(); *r!='\0'; r++, w++)
      {
        *w = (*r == ',') ? ';' : *r;
      }
      *w++ = '\0';
    }
    m_filters[len] = '\0';
  }
  filetype_pair_t getType(const char *filter) const
  {
    for(FileTypeList::const_iterator i = m_types.begin(); i != m_types.end(); ++i)
    {
      if(string_equal((*i).m_pattern.c_str(), filter))
      {
        return filetype_pair_t((*i).m_moduleName.c_str(), filetype_t((*i).m_name.c_str(), (*i).m_pattern.c_str()));
      }
    }
    return filetype_pair_t();
  }
  const char* getFilters() const
  {
    return m_filters.data();
  }
};

#define WIN32_LEAN_AND_MEAN
#include <gdk/gdkwin32.h>
#include <commdlg.h>

static char szFile[MAX_PATH];       /* filename string */


#define FILEDLG_CUSTOM_FILTER_LENGTH 64
// to be used with the advanced file selector

const char* file_dialog_show_win32(GtkWidget* parent, bool open, const char* title, const char* path, const char* pattern)
{
  const char* r;
  char* w;
  filetype_t type;
  FileTypeList typelist;

  if(pattern == 0)
  {
    pattern = "*";
  }

  GlobalFiletypes().getTypeList(pattern, &typelist);

  Win32Filters filters(typelist);

  // win32 dialog stores the selected "save as type" extension in the second null-terminated string
  char customfilter[FILEDLG_CUSTOM_FILTER_LENGTH];

  static OPENFILENAME ofn;       /* common dialog box structure   */ 
  static char szDirName[MAX_PATH];    /* directory string              */ 
  static char szFile[MAX_PATH];       /* filename string               */ 
  static char szFileTitle[MAX_PATH];  /* file title string             */ 
  static int i, cbString;        /* integer count variables       */ 
  static HANDLE hf;              /* file handle                   */ 

  // do that the native way
  /* Place the terminating null character in the szFile. */  
  szFile[0] = '\0';
  customfilter[0] = customfilter[1] = customfilter[2] = '\0';
  
  /* Set the members of the OPENFILENAME structure. */     
  ofn.lStructSize = sizeof(OPENFILENAME); 
  ofn.hwndOwner = (HWND)GDK_WINDOW_HWND(parent->window);
  ofn.nFilterIndex = 0;
  ofn.lpstrFilter = filters.getFilters();
  ofn.lpstrCustomFilter = customfilter;
  ofn.nMaxCustFilter = sizeof(customfilter);
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile); 
  ofn.lpstrFileTitle = 0; // we don't need to get the name of the file
  if(path)
  {
    // szDirName: Radiant uses unix convention for paths internally
    //   Win32 (of course) and Gtk (who would have thought) expect the '\\' convention
    // copy path, replacing dir separators as appropriate
    for(r=path, w=szDirName; *r!='\0'; r++)
      *w++ = (*r=='/') ? '\\' : *r;
    // terminate string
    *w = '\0';
    ofn.lpstrInitialDir = szDirName;
  }
  else ofn.lpstrInitialDir = 0;
  ofn.lpstrTitle = title;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY; 

  /* Display the Open dialog box. */
  // it's open or close depending on 'open' parameter
  if (open)
  {
    if (!GetOpenFileName(&ofn))
      return 0; // canceled
  }
  else
  {
    if (!GetSaveFileName(&ofn))
      return 0; // canceled
  }

  if(!string_equal(pattern, "*"))
  {
    type = filters.getType(customfilter+1).m_type;
  }

  // don't return an empty filename
  if(szFile[0] == '\0') return 0;

  // convert back to unix format
  for(w=szFile; *w!='\0'; w++)
  {
    if(*w=='\\')
    {
      *w = '/';
    }
  }
  // when saving, force an extension depending on filetype
  /* \todo SPoG - file_dialog should return filetype information separately.. not force file extension.. */
  if(!open && !string_equal(pattern, "*"))
  {
    // last ext separator
    const char* extension = path_get_extension(szFile);
    // no extension
    if(string_empty(extension))
    {
      strcat(szFile, type.pattern+1);
    }
    else
    {
      strcpy(szFile + (extension - szFile), type.pattern+2);
    }
  }

  return szFile;
}

#endif


class GTKMasks
{
  const FileTypeList& m_types;
public:
  std::vector<CopiedString> m_filters;
  std::vector<CopiedString> m_masks;

  GTKMasks(const FileTypeList& types) : m_types(types)
  {
    m_masks.reserve(m_types.size());
    for(FileTypeList::const_iterator i = m_types.begin(); i != m_types.end(); ++i)
    {
      std::size_t len = strlen((*i).m_name.c_str()) + strlen((*i).m_pattern.c_str()) + 3;
      StringOutputStream buffer(len + 1); // length + null char

      buffer << (*i).m_name.c_str() << " <" << (*i).m_pattern.c_str() << ">";

      m_masks.push_back(buffer.c_str());
    }

    m_filters.reserve(m_types.size());
    for(FileTypeList::const_iterator i = m_types.begin(); i != m_types.end(); ++i)
    {
      m_filters.push_back((*i).m_pattern);
    }
  }

  filetype_pair_t GetTypeForGTKMask(const char *mask) const
  {
    std::vector<CopiedString>::const_iterator j = m_masks.begin();
    for(FileTypeList::const_iterator i = m_types.begin(); i != m_types.end(); ++i, ++j)
    {
      if(string_equal((*j).c_str(), mask))
      {
        return filetype_pair_t((*i).m_moduleName.c_str(), filetype_t((*i).m_name.c_str(), (*i).m_pattern.c_str()));
      }
    }
    return filetype_pair_t();
  }

};

static char g_file_dialog_file[1024];

const char* file_dialog_show(GtkWidget* parent, bool open, const char* title, const char* path, const char* pattern)
{
  filetype_t type;

  if(pattern == 0)
  {
    pattern = "*";
  }

  FileTypeList typelist;
  GlobalFiletypes().getTypeList(pattern, &typelist);

  GTKMasks masks(typelist);

  if (title == 0)
    title = open ? "Open File" : "Save File";
    
  GtkWidget* dialog;
  if (open)
  {
    dialog = gtk_file_chooser_dialog_new(title,
                                        GTK_WINDOW(parent),
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                        NULL);
  }
  else
  {
    dialog = gtk_file_chooser_dialog_new(title,
                                        GTK_WINDOW(parent),
                                        GTK_FILE_CHOOSER_ACTION_SAVE,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                        NULL);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "unnamed");
  }

  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);

  // we expect an actual path below, if the path is 0 we might crash
  if (path != 0 && !string_empty(path))
  {
    ASSERT_MESSAGE(path_is_absolute(path), "file_dialog_show: path not absolute: " << makeQuoted(path));

    Array<char> new_path(strlen(path)+1);

    // copy path, replacing dir separators as appropriate
    Array<char>::iterator w = new_path.begin();
    for(const char* r = path; *r != '\0'; ++r)
    {
      *w++ = (*r == '/') ? G_DIR_SEPARATOR : *r;
    }
    // remove separator from end of path if required
    if(*(w-1) == G_DIR_SEPARATOR)
    {
      --w;
    }
    // terminate string
    *w = '\0';

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), new_path.data());
  }

  // we should add all important paths as shortcut folder...
  // gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog), "/tmp/", NULL);

  
  for(std::size_t i = 0; i < masks.m_filters.size(); ++i)
  {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, masks.m_filters[i].c_str());
    gtk_file_filter_set_name(filter, masks.m_masks[i].c_str());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
  }

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
  {
    strcpy(g_file_dialog_file, gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));

    if(!string_equal(pattern, "*"))
    {
      GtkFileFilter* filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(dialog));
      if(filter != 0) // no filter set? some file-chooser implementations may allow the user to set no filter, which we treat as 'all files'
      {
        type = masks.GetTypeForGTKMask(gtk_file_filter_get_name(filter)).m_type;
        // last ext separator
        const char* extension = path_get_extension(g_file_dialog_file);
        // no extension
        if(string_empty(extension))
        {
          strcat(g_file_dialog_file, type.pattern+1);
        }
        else
        {
          strcpy(g_file_dialog_file + (extension - g_file_dialog_file), type.pattern+2);
        }
      }
    }

    // convert back to unix format
    for(char* w = g_file_dialog_file; *w!='\0'; w++)
    {
      if(*w=='\\') 
      {
        *w = '/';
      }
    }
  }
  else
  {
    g_file_dialog_file[0] = '\0';
  }

  gtk_widget_destroy(dialog);

  // don't return an empty filename
  if(g_file_dialog_file[0] == '\0') return NULL;

  return g_file_dialog_file;
}

char* dir_dialog(GtkWidget* parent, const char* title, const char* path)
{
  GtkWidget* dialog = gtk_file_chooser_dialog_new(title,
				        GTK_WINDOW(parent),
				        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
				        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				        NULL);

  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);

  if(!string_empty(path))
  {
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);
  }

  char* filename = 0;
  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
  {
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
  }

  gtk_widget_destroy(dialog);

  return filename;
}


#ifdef WIN32
bool g_FileChooser_nativeGUI = true;
#endif

const char* file_dialog(GtkWidget* parent, bool open, const char* title, const char* path, const char* pattern)
{
  for(;;)
  {
    const char* file =
#ifdef WIN32
	                g_FileChooser_nativeGUI
      ? file_dialog_show_win32(parent, open, title, path, pattern) :
#endif
        file_dialog_show(parent, open, title, path, pattern);

    if(open
      || file == 0
      || !file_exists(file)
      || gtk_MessageBox(parent, "The file specified already exists.\nDo you want to replace it?", title, eMB_NOYES, eMB_ICONQUESTION) == eIDYES)
    {
      return file;
    }
  }
}
