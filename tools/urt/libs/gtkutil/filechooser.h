
#if !defined(INCLUDED_GTKUTIL_FILECHOOSER_H)
#define INCLUDED_GTKUTIL_FILECHOOSER_H

/// \file
/// GTK+ file-chooser dialogs.

#ifdef WIN32
extern bool g_FileChooser_nativeGUI;
#endif

typedef struct _GtkWidget GtkWidget;
const char* file_dialog(GtkWidget *parent, bool open, const char* title, const char* path = 0, const char* pattern = 0);


/// \brief Prompts the user to browse for a directory.
/// The prompt window will be transient to \p parent.
/// The directory will initially default to \p path, which must be an absolute path.
/// The returned string is allocated with \c g_malloc and must be freed with \c g_free.
char* dir_dialog(GtkWidget *parent, const char* title = "Choose Directory", const char* path = "");

#endif
