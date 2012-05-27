
#if !defined(INCLUDED_GTKUTIL_GLWIDGET_H)
#define INCLUDED_GTKUTIL_GLWIDGET_H

typedef struct _GtkWidget GtkWidget;
typedef int    gint;
typedef gint   gboolean;

GtkWidget* glwidget_new(gboolean zbuffer);
void glwidget_swap_buffers(GtkWidget* widget);
gboolean glwidget_make_current(GtkWidget* widget);
void glwidget_destroy_context(GtkWidget* widget);
void glwidget_create_context(GtkWidget* widget);

extern void (*GLWidget_sharedContextCreated)();
extern void (*GLWidget_sharedContextDestroyed)();


#endif
