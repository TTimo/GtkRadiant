
#if !defined( INCLUDED_GTKUTIL_BUTTON_H )
#define INCLUDED_GTKUTIL_BUTTON_H

typedef struct _GtkButton GtkButton;
typedef struct _GtkToggleButton GtkToggleButton;
typedef struct _GtkRadioButton GtkRadioButton;
typedef int gint;
typedef gint gboolean;
typedef unsigned int guint;
class Callback;

void button_connect_callback( GtkButton* button, const Callback& callback );
guint toggle_button_connect_callback( GtkToggleButton* button, const Callback& callback );

void button_set_icon( GtkButton* button, const char* icon );
void toggle_button_set_active_no_signal( GtkToggleButton* item, gboolean active );

void radio_button_set_active( GtkRadioButton* radio, int index );
void radio_button_set_active_no_signal( GtkRadioButton* radio, int index );
int radio_button_get_active( GtkRadioButton* radio );

#endif
