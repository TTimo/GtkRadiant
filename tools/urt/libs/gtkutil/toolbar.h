
#if !defined(INCLUDED_GTKUTIL_TOOLBAR_H)
#define INCLUDED_GTKUTIL_TOOLBAR_H

class Callback;
typedef struct _GtkButton GtkButton;
typedef struct _GtkToggleButton GtkToggleButton;
typedef struct _GtkToolbar GtkToolbar;
class Command;
class Toggle;

GtkButton* toolbar_append_button(GtkToolbar* toolbar, const char* description, const char* icon, const Callback& callback);
GtkButton* toolbar_append_button(GtkToolbar* toolbar, const char* description, const char* icon, const Command& command);
GtkToggleButton* toolbar_append_toggle_button(GtkToolbar* toolbar, const char* description, const char* icon, const Callback& callback);
GtkToggleButton* toolbar_append_toggle_button(GtkToolbar* toolbar, const char* description, const char* icon, const Toggle& toggle);

#endif
