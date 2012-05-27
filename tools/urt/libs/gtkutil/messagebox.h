
#if !defined(INCLUDED_GTKUTIL_MESSAGEBOX_H)
#define INCLUDED_GTKUTIL_MESSAGEBOX_H

#include "qerplugin.h"

typedef struct _GtkWidget GtkWidget;
/// \brief Shows a modal message-box.
EMessageBoxReturn gtk_MessageBox(GtkWidget *parent, const char* text, const char* title = "GtkRadiant", EMessageBoxType type = eMB_OK, EMessageBoxIcon icon = eMB_ICONDEFAULT);

#endif
