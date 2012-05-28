
#if !defined( INCLUDED_GTKUTIL_PANED_H )
#define INCLUDED_GTKUTIL_PANED_H

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkHPaned GtkHPaned;
GtkHPaned* create_split_views( GtkWidget* topleft, GtkWidget* topright, GtkWidget* botleft, GtkWidget* botright );

#endif
