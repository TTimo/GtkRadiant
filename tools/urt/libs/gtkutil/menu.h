
#if !defined( INCLUDED_GTKUTIL_MENU_H )
#define INCLUDED_GTKUTIL_MENU_H

class Callback;
typedef int gint;
typedef gint gboolean;
typedef struct _GSList GSList;
typedef struct _GtkMenu GtkMenu;
typedef struct _GtkMenuBar GtkMenuBar;
typedef struct _GtkMenuItem GtkMenuItem;
typedef struct _GtkCheckMenuItem GtkCheckMenuItem;
typedef struct _GtkRadioMenuItem GtkRadioMenuItem;
typedef struct _GtkTearoffMenuItem GtkTearoffMenuItem;

void menu_add_item( GtkMenu* menu, GtkMenuItem* item );
GtkMenuItem* menu_separator( GtkMenu* menu );
GtkTearoffMenuItem* menu_tearoff( GtkMenu* menu );
GtkMenuItem* new_sub_menu_item_with_mnemonic( const char* mnemonic );
GtkMenu* create_sub_menu_with_mnemonic( GtkMenuBar* bar, const char* mnemonic );
GtkMenu* create_sub_menu_with_mnemonic( GtkMenu* parent, const char* mnemonic );
GtkMenuItem* create_menu_item_with_mnemonic( GtkMenu* menu, const char* mnemonic, const Callback& callback );
GtkCheckMenuItem* create_check_menu_item_with_mnemonic( GtkMenu* menu, const char* mnemonic, const Callback& callback );
GtkRadioMenuItem* create_radio_menu_item_with_mnemonic( GtkMenu* menu, GSList** group, const char* mnemonic, const Callback& callback );

class Command;
GtkMenuItem* create_menu_item_with_mnemonic( GtkMenu* menu, const char* mnemonic, const Command& command );
class Toggle;
GtkCheckMenuItem* create_check_menu_item_with_mnemonic( GtkMenu* menu, const char* mnemonic, const Toggle& toggle );


typedef struct _GtkCheckMenuItem GtkCheckMenuItem;
void check_menu_item_set_active_no_signal( GtkCheckMenuItem* item, gboolean active );
typedef struct _GtkRadioMenuItem GtkRadioMenuItem;
void radio_menu_item_set_active_no_signal( GtkRadioMenuItem* item, gboolean active );

#endif
