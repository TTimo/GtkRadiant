// tree.cpp: implementation of the CTree class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tree.h"
#include <string>
#include "mainwnd.h"

extern CMainWnd *g_pMainWnd;


static void button_release (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
 	if (event->button==3)
		g_pMainWnd->HandleDrop();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTree::CTree()
{
	m_Tree=NULL;
	m_Root=NULL;

}

CTree::~CTree()
{

}

void CTree::Init(GtkWidget *win)
{
	m_pMainWnd=win;

	// create the tree
	m_Tree=gtk_tree_new();

	// Add tree to passed scroll window
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(win),(GtkWidget*)m_Tree);

	// Show it
	gtk_widget_show(m_Tree);

	// connect signal
	gtk_signal_connect (GTK_OBJECT(m_Tree), "select_child",
		                GTK_SIGNAL_FUNC(cb_select_child), m_Tree);

	// create the root item
	GtkWidget *item=gtk_tree_item_new_with_label("root");
	gtk_tree_append(GTK_TREE(m_Tree),item);
	gtk_widget_show(item);

	gtk_signal_connect (GTK_OBJECT (item), "button_release_event",GTK_SIGNAL_FUNC (button_release), NULL);

	m_Root=gtk_tree_new();
	gtk_tree_item_set_subtree(GTK_TREE_ITEM(item),m_Root);
	gtk_tree_item_expand(GTK_TREE_ITEM(item));


	// connect signal
	gtk_signal_connect (GTK_OBJECT(m_Root), "select_child",
		                GTK_SIGNAL_FUNC(cb_select_child), m_Tree);

	// ok were done
	return;
}

void CTree::Clear()
{
	gtk_widget_destroy(m_Tree);
	Init(m_pMainWnd);
}

void CTree::AddPath(const char *buff)
{
	Str path(buff);

	// convert '\' to '/'
	for (int c=0 ; c<path.GetLength() ; c++)
	{
		if (path.GetAt(c)=='\\')
			path.SetAt(c,'/');
	}

	if (path[0]=='/')
		path=path.Mid(1);

	// lets tokenize it first
	if (path.GetAt(path.GetLength()-1)=='/')
	{
		// its just a folder path, no file
		return;
	}

	GtkWidget *node=m_Root;

	long pos=path.Find('/');
	while (pos!=-1)
	{
		Str folder=path.Left(pos);
		path=path.Mid(pos+1);

		// add a folder, or retrieve its node if it already exists
		node=AddFolder(folder.GetBuffer(),node);

		pos=path.Find('/');
	}

	// add the filename to the final nested node
	AddFile(path.GetBuffer(),node);
}


void CTree::AddFile(const char *buff, GtkWidget *node)
{
	Str file(buff);

	if (node==NULL)
		node=m_Root;

	GtkWidget *item=gtk_tree_item_new_with_label(buff);
	gtk_tree_append(GTK_TREE(node),item);
	gtk_widget_show(item);

	gtk_widget_set_name(item,file);

	//	gtk_signal_connect (GTK_OBJECT(item), "select_child",
	//                      GTK_SIGNAL_FUNC(cb_select_child), item);

	gtk_signal_connect (GTK_OBJECT (item), "button_release_event",GTK_SIGNAL_FUNC (button_release), NULL);


	// a file is the last item in the tree so no tree is needed
}

GtkWidget* CTree::AddFolder(const char *buff,GtkWidget *node)
{
	Str folder(buff);

	if (node==NULL)
		node=m_Root;

	// search the node's siblings for the folder
	GList *children = gtk_container_children (GTK_CONTAINER (node));
    while (children)
	{
		GtkTreeItem *tree_item=(GTK_TREE_ITEM (children->data));

		// check item for equality with the folder
		// and weather it has a tree attached

		if (GTK_TREE_ITEM_SUBTREE(tree_item))
		{
			GtkItem *item=GTK_ITEM(tree_item);

			GList *item_children=gtk_container_children(GTK_CONTAINER(item));

			GtkLabel *label=GTK_LABEL(item_children->data);

			if (strcmp(label->label,buff)==0)
			{
				g_list_free(item_children);
				g_list_free(children);

				// return tree attached to the item
				return GTK_TREE_ITEM_SUBTREE(tree_item);
			}

			g_list_free(item_children);
		}

		children = g_list_remove_link (children, children);
	}

	g_list_free(children);

	// ok, not found so we add it
	GtkWidget *item=gtk_tree_item_new_with_label(buff);
	gtk_tree_append(GTK_TREE(node),item);
	gtk_widget_show(item);

	GtkWidget *tree=gtk_tree_new();
	gtk_tree_item_set_subtree(GTK_TREE_ITEM(item),tree);
	gtk_widget_set_name(tree,buff);

	gtk_signal_connect (GTK_OBJECT(tree), "select_child",
                      GTK_SIGNAL_FUNC(cb_select_child), tree);

	gtk_signal_connect (GTK_OBJECT (item), "button_release_event",GTK_SIGNAL_FUNC (button_release), NULL);


	//gtk_signal_connect (GTK_OBJECT(tree), "selection_changed",
      //                GTK_SIGNAL_FUNC(cb_selection_changed), tree, this);

	return tree;
}


std::string CTree::GetFullPath(GtkWidget *child)
{
	std::string path;
	gchar *name;

	GtkLabel *label = GTK_LABEL (GTK_BIN (child)->child);
  	gtk_label_get (label, &name);
	path=name;

	if (path.compare("root")==0)
		return "root";

	while (1)
	{
		// this gets an items parent tree
		GtkWidget *tr=(child->parent);

		if (GTK_WIDGET(tr)==m_Root)
			break;

		// and now its item
		child=GTK_TREE(tr)->tree_owner;

		// and now the items label
		label = GTK_LABEL (GTK_BIN (child)->child);
		gtk_label_get (label, &name);
		std::string lbl=name;
		path=lbl+"/"+path;
	}

	return path;
}

void CTree::RemovePath(const char *path)
{
	// not used yet
}


void CTree::SelectChild(GtkWidget *child)
{
	g_pMainWnd->UpdateStatus();
	g_pMainWnd->UpdateToolBar();
}

void cb_select_child (GtkWidget *root_tree, GtkWidget *child, GtkWidget *subtree)
{
	g_pMainWnd->m_Tree.SelectChild(child);
}


std::string CTree::GetSelected()
{
	std::string path="";

	if (!GTK_TREE(m_Tree)->selection)
		return path;

	GList *selected = (GTK_TREE(m_Tree)->selection);

	GtkWidget *item=GTK_WIDGET(selected->data);

	if (GTK_TREE_ITEM(item)->subtree)
		return path;

	path=GetFullPath(item);

	return path;
}
