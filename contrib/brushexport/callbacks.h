typedef struct _GtkWidget GtkWidget;
typedef struct _GtkButton GtkButton;

namespace callbacks {

void OnDestroy(GtkWidget*, gpointer);
void OnExportClicked(GtkButton*, gpointer);
void OnAddMaterial(GtkButton*, gpointer);
void OnRemoveMaterial(GtkButton*, gpointer);
void OnExportMatClicked(GtkButton* button, gpointer);

}// callbacks
