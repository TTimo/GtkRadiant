// MainWnd.cpp: implementation of the CMainWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mainwnd.h"
#include "memfile.h"
#include "md3.h"
#include "wild.h"
#include "renamedlg.h"
#ifdef __linux__
#include <dirent.h>
#include <stdlib.h>
#endif

extern CMainWnd *g_pMainWnd;
bool g_bIgnoreCommands=FALSE;

GtkWidget* create_menu_in_menu (GtkWidget *menu, gchar *label, GtkAccelGroup *menu_accel,GtkAccelGroup **submenu_accel);
GtkWidget* create_menu_item (GtkWidget *menu, gchar *label, GtkAccelGroup *menu_accel,GtkSignalFunc func, int id);
GtkWidget* menu_separator (GtkWidget *menu);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static gint mainwnd_delete (GtkWidget *widget, gpointer data)
{
return g_pMainWnd->OnClose();
}

static void HandleCommand (GtkWidget *widget, gpointer data)
{
	int id = GPOINTER_TO_INT (data);

	if (g_bIgnoreCommands)
	return;

	switch (id)
	{
		case ID_FILE_NEW: g_pMainWnd->OnFileNew (); break;
		case ID_FILE_OPEN: g_pMainWnd->OnFileOpen (); break;
		case ID_FILE_SAVE: g_pMainWnd->OnFileSave (); break;

		case ID_WIZARD: g_pMainWnd->OnWizard (); break;

		case ID_VIEW: g_pMainWnd->OnView (); break;
		case ID_EXTRACT: g_pMainWnd->OnExtract (); break;

		case ID_ADD: g_pMainWnd->OnAdd (); break;
		case ID_RENAME: g_pMainWnd->OnRename (); break;
		case ID_DELETE: g_pMainWnd->OnDelete (); break;
	}
}

CMainWnd::CMainWnd()
{
	m_pMainWnd=NULL;
	m_mnuDrop=NULL;
}

CMainWnd::~CMainWnd()
{
	if (m_pMainWnd)
		delete m_pMainWnd;
}

bool CMainWnd::OnClose()
{
	if (pak.IsModified())
	{
		//int ret=DoMessageBox("Pk3 has not been saved\n\n    Really quit?","Pk3Man",MB_YESNO);
		int ret=g_FuncTable.m_pfnMessageBox(m_pMainWnd,"Pk3 has not been saved\n\n    Really quit?","Pk3Man",MB_YESNO, NULL);
		if (ret!=IDYES)
			return 1;
	}

	FreePakList();
	DeleteTempFiles();
	return 0;
}

void CMainWnd::OnDestroy()
{
	// NOT CALLED !!!
}

BOOL CMainWnd::Create(GtkWidget *parent)
{
	GtkWidget *vbox,*window;
	GdkPixmap *pixmap=NULL;
	GdkBitmap *mask=NULL;

	char * icon_xpm[] = { "32 32 6 1", ".	c none", "/	c #ff0ff", "0	c #dddddd", "1	c #e6a0e6", "2	c #333333", "3	c #9d189d", "................................", "................................", "............////////............", "............////////............", "........////////////////........", "........////////////////........", "......////////////////////......", "......////////////////////......", "....//////0000////////0001//....", "....//////0000////////0000//....", "....////00000000////00000001....", "....////00000000////00000001....", "....////00002222////00002223....", "....////00002222////00002223....", "..//////00002222////00002223//..", "..//////00002222////00002222//..", "..////////0000////////0000////..", "..////////0000////////0000////..", "..////////////////////////////..", "..////////////////////////////..", "..////////////////////////////..", "..////////////////////////////..", "..////////////////////////////..", "..////////////////////////////..", "..////////////////////////////..", "..////////////////////////////..", "..////..//////....//////..////..", "..////..//////....//////..////..", "..//......////....////......//..", "..//......////....////......//..", "................................", "................................"};

	// get bitmap paths
	GetPaths();

	// create the main widget
	m_pMainWnd = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC (mainwnd_delete), this);
	//gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroy), this);

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_title (GTK_WINDOW (m_pMainWnd), "Pk3Man - ");
		gtk_window_set_default_size (GTK_WINDOW (m_pMainWnd), 300, 300);
		gtk_widget_show (m_pMainWnd);

	// load and set the icon

	pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask, NULL, icon_xpm);
	gdk_window_set_icon (window->window, NULL, pixmap, mask);

	// setup the vertical layout box
	vbox=gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);

	if (!CreateToolbar (window, vbox))
		return FALSE;

	if (!CreateTreeView (window, vbox))
		return FALSE;

	if (!CreateStatusBar (window,vbox))
		return FALSE;

	UpdateStatus();
	UpdateToolBar();

	LoadExclusions();
	InitPakList();

	return TRUE;
}

BOOL CMainWnd::CreateTreeView(GtkWidget *window,GtkWidget *vbox)
{
	// create a scrollable widget to house the tree
	m_ScrolledWin = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_ScrolledWin),
								GTK_POLICY_AUTOMATIC,
								GTK_POLICY_AUTOMATIC);

	//gtk_widget_set_usize (m_ScrolledWin, 400, 400);
	gtk_box_pack_start (GTK_BOX (vbox), m_ScrolledWin, TRUE, TRUE, 0);
	gtk_widget_show (m_ScrolledWin);

	// attach and initialise the tree
	m_Tree.Init(m_ScrolledWin);

	return TRUE;
}

BOOL CMainWnd::CreateToolbar(GtkWidget *window,GtkWidget *vbox)
{
	GtkWidget *handle_box, *toolbar, *w;

	handle_box = gtk_handle_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox), handle_box, FALSE, FALSE, 0);
	gtk_widget_show (handle_box);

	m_Toolbar = toolbar = gtk_toolbar_new ();

	//  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), user_rc.toolbar_style);
	gtk_container_add (GTK_CONTAINER (handle_box), toolbar);
	gtk_container_border_width (GTK_CONTAINER (toolbar), 2);
	gtk_widget_show (toolbar);

	// new
	w = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
								"New",
								"Start a new Pk3",
								"",
								CPixMap::new_pixmap(window, "pk3man_tbnew.bmp"),
								//CPixMap::pixmap_from_char(window,xpm_new),
								GTK_SIGNAL_FUNC (HandleCommand),
								GINT_TO_POINTER (ID_FILE_NEW));

	gtk_object_set_data (GTK_OBJECT (window), "tb_file_new", w);

	// open
	w = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
								"Open",
								"Open an existing Pk3",
								"",
								CPixMap::new_pixmap(window, "pk3man_tbopen.bmp"),
								GTK_SIGNAL_FUNC (HandleCommand),
								GINT_TO_POINTER (ID_FILE_OPEN));

	gtk_object_set_data (GTK_OBJECT (window), "tb_file_open", w);

	// save
	w = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
								"Save",
								"Save the current Pk3",
								"",
								CPixMap::new_pixmap (window, "pk3man_tbsave.bmp"),
								GTK_SIGNAL_FUNC (HandleCommand),
								GINT_TO_POINTER (ID_FILE_SAVE));

	gtk_object_set_data (GTK_OBJECT (window), "tb_file_save", w);

	// space
	gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

	// wizard
	w = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
								"Wizard",
								"Build Pk3 from a Map",
								"",
								CPixMap::new_pixmap (window, "pk3man_tbwizard.bmp"),
								GTK_SIGNAL_FUNC (HandleCommand),
								GINT_TO_POINTER (ID_WIZARD));

	gtk_object_set_data (GTK_OBJECT (window), "tb_wizard", w);

	// space
	gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

	// view
	w = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
								"View",
								"View an entry",
								"",
								CPixMap::new_pixmap (window, "pk3man_tbview.bmp"),
								GTK_SIGNAL_FUNC (HandleCommand),
								GINT_TO_POINTER (ID_VIEW));

	gtk_object_set_data (GTK_OBJECT (window), "tb_view", w);

	// extract
	w = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
								"Extract",
								"Extract an entry",
								"",
								CPixMap::new_pixmap (window, "pk3man_tbextract.bmp"),
								GTK_SIGNAL_FUNC (HandleCommand),
								GINT_TO_POINTER (ID_EXTRACT));

	gtk_object_set_data (GTK_OBJECT (window), "tb_extract", w);

	// space
	gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

	// Add
	w = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
								"Wizard",
								"Add a file",
								"",
								CPixMap::new_pixmap (window, "pk3man_tbadd.bmp"),
								GTK_SIGNAL_FUNC (HandleCommand),
								GINT_TO_POINTER (ID_ADD));

	gtk_object_set_data (GTK_OBJECT (window), "tb_add", w);

	// rename
	w = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
								"Rename",
								"Rename an entry",
								"",
								CPixMap::new_pixmap (window, "pk3man_tbrename.bmp"),
								GTK_SIGNAL_FUNC (HandleCommand),
								GINT_TO_POINTER (ID_RENAME));

	gtk_object_set_data (GTK_OBJECT (window), "tb_rename", w);


	// rename
	w = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
								"Delete",
								"Delete an entry",
								"",
								CPixMap::new_pixmap (window, "pk3man_tbdelete.bmp"),
								GTK_SIGNAL_FUNC (HandleCommand),
								GINT_TO_POINTER (ID_DELETE));

	gtk_object_set_data (GTK_OBJECT (window), "tb_delete", w);

	return TRUE;
}

void CMainWnd::UpdateToolBar()
{
	std::string path=m_Tree.GetSelected();
	bool file_selected=TRUE;

	if (path=="")
		file_selected=FALSE;

	if (file_selected)
	{
		// enable toolbar buttons

		GtkWidget *item;

		item  = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (m_pMainWnd), "tb_view"));
		//gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (item), FALSE);
		gtk_widget_set_sensitive (item, TRUE);

		item  = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (m_pMainWnd), "tb_extract"));
		//gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (item), FALSE);
		gtk_widget_set_sensitive (item, TRUE);

		item  = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (m_pMainWnd), "tb_delete"));
		//gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (item), FALSE);
		gtk_widget_set_sensitive (item, TRUE);

		item  = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (m_pMainWnd), "tb_rename"));
		//gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (item), FALSE);
		gtk_widget_set_sensitive (item, TRUE);

	}
	else
	{
		// disable toolbar buttons

		GtkWidget *item;

		item  = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (m_pMainWnd), "tb_view"));
		//gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (item), TRUE);
		gtk_widget_set_sensitive (item, TRUE);

		item  = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (m_pMainWnd), "tb_extract"));
		//gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (item), TRUE);
		gtk_widget_set_sensitive (item, TRUE);

		item  = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (m_pMainWnd), "tb_delete"));
		//gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (item), TRUE);
		gtk_widget_set_sensitive (item, TRUE);

		item  = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (m_pMainWnd), "tb_rename"));
		//gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (item), TRUE);
		gtk_widget_set_sensitive (item, TRUE);

	}

}

BOOL CMainWnd::CreateStatusBar(GtkWidget *window,GtkWidget *vbox)
{
	GtkWidget *hbox, *hbox1;
	GtkWidget *frame;
	GtkWidget *label;

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_widget_set_usize (hbox, -1, 24);
	gtk_container_border_width (GTK_CONTAINER (hbox), 1);
	gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, TRUE, 2);

	frame = gtk_frame_new ((char*)NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), hbox1);
	gtk_container_border_width (GTK_CONTAINER (hbox1), 0);
	gtk_widget_show (hbox1);

	label = gtk_label_new (" Ready ... ");
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, TRUE, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (label), 3, 0);
	m_pStatusLabel[0] = label;

	for (int i = 1; i < 3; i++)
	{
		frame = gtk_frame_new ((char*)NULL);
		gtk_widget_show (frame);
		gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, TRUE, 0);
		gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

		label = gtk_label_new ("             ");
		gtk_widget_show (label);
		gtk_container_add (GTK_CONTAINER (frame), label);
		m_pStatusLabel[i] = label;
	}

	return TRUE;
}

void CMainWnd::OnFileOpen()
{
	// lets pick a file
	const char *filename;

	if (g_strModPath.GetLength()>0)
		//filename=file_dialog(TRUE,"Choose a Pk3...",g_strModPath.GetBuffer(),"*.pk3");
		filename=g_FuncTable.m_pfnFileDialog (m_pMainWnd, TRUE, "Choose a Pk3...",
											g_strModPath.GetBuffer(), "pk3man");
	else
		//filename=file_dialog(TRUE,"Choose a Pk3...",g_strBasePath.GetBuffer(),"*.pk3");
		filename=g_FuncTable.m_pfnFileDialog (m_pMainWnd, TRUE, "Choose a Pk3...",
											g_strBasePath.GetBuffer(), "pk3man");

	if (filename==NULL)
		return;

	Str fname=filename;
	fname=fname.Right(3);
	if (fname!="pk3")
	{
		//DoMessageBox("Please pick a Pk3 file","Pk3Man",MB_OK);
		g_FuncTable.m_pfnMessageBox(m_pMainWnd,"Please pick a Pk3 file","Pk3Man",MB_OK, NULL);
		return;
	}

	OnFileNew();
	pak.Open(filename);
	UpdateTree();

	// update window title
	g_FuncTable.m_pfnQE_ConvertDOSToUnixName (filename, filename);
	std::string title=filename;
	g_free (filename);
	long pos=title.find_last_of('/');
	title=title.substr(pos+1);
	title="Pk3Man - "+title;
	gtk_window_set_title (GTK_WINDOW (m_pMainWnd), title.c_str());
}

void CMainWnd::OnFileSave()
{
	const char *filename;

	if (g_strModPath.GetLength()>0)
		filename=g_FuncTable.m_pfnFileDialog (m_pMainWnd, FALSE, "Save As...",
											g_strModPath.GetBuffer(), "pk3man");
	else
		filename=g_FuncTable.m_pfnFileDialog (m_pMainWnd, FALSE, "Save As...",
										g_strBasePath.GetBuffer(), "pk3man");

	if (filename==NULL)
		return;

	g_FuncTable.m_pfnQE_ConvertDOSToUnixName (filename, filename);
	std::string path=filename;
	g_free (filename);
	long p=path.find_last_of('/');
	std::string name=path.substr(p,-1);

	std::string tpath=g_strTempPath.GetBuffer() + name;

	if (!pak.Save(tpath.c_str()))
	{
		// eek, error central
		g_FuncTable.m_pfnMessageBox(m_pMainWnd,"Unable to save the pk3 to temp folder!","Pk3Man",MB_OK, NULL);
		return;
	}

	// ok, now copy to proper place

#ifdef WIN32

	int ret=CopyFile(tpath.c_str(),path.c_str(),FALSE);
	DeleteFile(tpath.c_str());

	if (!ret)
	{
	g_FuncTable.m_pfnMessageBox(m_pMainWnd,"Unable to copy pk3 from temp folder!","Pk3Man",MB_OK, NULL);
	return;
	}

#endif

	pak.Close();
	pak.Open(path.c_str());
	m_Tree.Clear();
	UpdateTree();

	//update title
	name=name.substr(1,-1);
	std::string title="Pk3Man - "+name;
	gtk_window_set_title (GTK_WINDOW (m_pMainWnd), title.c_str());
}

bool CMainWnd::OnFileNew()
{
	if (pak.IsModified())
	{
		int ret=g_FuncTable.m_pfnMessageBox(m_pMainWnd,"This will lose the current Pk3 contents\n\n    Are u sure?","Pk3Man",MB_YESNO, NULL);
		if (ret!=IDYES)
			return FALSE;
	}

	pak.Close();
	m_Tree.Clear();
	gtk_window_set_title (GTK_WINDOW (m_pMainWnd), "Pk3Man - ");

	return TRUE;
}

void CMainWnd::LoadExclusions()
{
	exclusions.clear();

	FILE *fp;
	std::string filename=g_strAppPath.GetBuffer();
	filename+="plugins/pk3man.exclude";

	fp=fopen(filename.c_str(),"rb");
	if (!fp)
		return;

	while (!feof(fp))
	{
		std::string line=GetLine(fp);

		if (line.length()==0)
			continue;

		if (line[0]=='/' && line[1]=='/')
			continue;

		for (int n=0 ; n < (int)line.length() ; n++)
		{
			if (line[n]=='\\')
				line[n]='/';
		}

		if (line[0]=='/')
			line=line.substr(1,-1);

		exclusions.push_back(line);
	}
}

bool CMainWnd::IsExcluded(const char *name)
{
	for (std::list<std::string>::iterator i=exclusions.begin() ; i!=exclusions.end() ; i++)
	{
		std::string t=*i;

		if (CWild::fpattern_match(t.c_str(),name))
			return TRUE;

		std::string nm=name;
		if (nm.find(t) != static_cast<size_t>(-1))
			return TRUE;
	}

	return FALSE;
}

void CMainWnd::OnWizard()
{
	if (!pak.IsEmpty())
	{
		int ret=g_FuncTable.m_pfnMessageBox(m_pMainWnd,"Add to current pak contents ?","Pk3Man",MB_YESNO, NULL);
		if (ret==IDNO)
		{
			if (!OnFileNew())
				return;
		}

	}

	std::string mappath="maps";
	mappath=g_strBasePath.GetBuffer()+mappath;

	gchar *filename=g_FuncTable.m_pfnFileDialog(m_pMainWnd,TRUE,"Choose a Map...",mappath.c_str(),"pk3man");

	if (filename==NULL)
			return;

	g_FuncTable.m_pfnQE_ConvertDOSToUnixName (filename, filename);
	std::string map=filename;
	long p=map.find_last_of('/');
	map=map.substr(p+1,-1);

	//update title
	std::string title="Pk3Man - "+map;
	gtk_window_set_title (GTK_WINDOW (m_pMainWnd), title.c_str());

	// add map and aas
	ProcessFile(map.c_str(),".bsp","maps");
	ProcessFile(map.c_str(),".aas","maps");

	// add levelshots
	if (!ProcessFile(map.c_str(),".jpg","levelshots"))
		ProcessFile(map.c_str(),".tga","levelshots");

	// arena file
	ProcessFile(map.c_str(),".arena","scripts");

	// initialise text list
	texlist.clear();

	// generate item list
	FILE *fp;
	fp=fopen(filename,"r");
	g_free(filename);

	if (!fp)
		return;

	while (!feof(fp))
	{
		std::string line=GetLine(fp);

		if (line.find("// brush")!=static_cast<size_t>(-1))
		{
			// its a brush
			line=GetLine(fp);
			line=GetLine(fp);

			if (line.find("patchDef")!=static_cast<size_t>(-1))
			{
				// its a patch
				line=GetLine(fp);
				line=GetLine(fp);

				// need to trim left and right
				int n=line.find_first_not_of(' ');
				if (n!=static_cast<size_t>(-1))
				line=line.substr(n);

				std::string tex="textures/"+line;
				AddToTexList(line.c_str());
			}
			else
			{
				// its a standard brush
				while (line.find_first_of('}')==static_cast<size_t>(-1))
				{
					long p=line.find_last_of(')');
					line=line.substr(p+2,-1);

					p=line.find_first_of(' ');
					line=line.substr(0,p);

					std::string tex="textures/"+line;
					AddToTexList(line.c_str());

					// next
					line=GetLine(fp);
				}
			}
		}
		else if (line.find(".md3")!=static_cast<size_t>(-1))
		{
			long p=line.find_first_of(' ');
			std::string tex=line.substr(p+2,-1);
			tex=tex.substr(0,tex.length()-2);

			AddToTexList(tex.c_str());
		}
		else if (line.find(".wav")!=static_cast<size_t>(-1))
		{
			long p=line.find_first_of(' ');
			std::string tex=line.substr(p+2,-1);
			tex=tex.substr(0,tex.length()-2);

			AddToTexList(tex.c_str());
		}

	}
	fclose(fp);

	// ok, now proccess our texlist
	// if we find a shader we add its textures onto the end of the texlist

	for ( std::list<std::string>::const_iterator i = texlist.begin() ; i != texlist.end() ; i++)
	{
		std::string tex=*i;

		if (tex=="")
			continue;

		std::string suffix=tex.substr(tex.length()-3,-1);

		// is it a model?
		if (suffix.compare("md3")==0)
		{
			ParseModel(tex.c_str());

			// dont add md3's
			continue;
		}

		// does it exist as it is
		if (ProcessFile(tex.c_str()))
			continue;

		//shader
		std::string shadername=tex;
		const char *sname=NULL;

		IShader *shader=g_ShadersTable.m_pfnShader_ForName_NoLoad(shadername.c_str());

		if (shader)
			sname=shader->getShaderFileName();

		if (strlen(sname)>0)
		{
			ParseShader(sname,tex.c_str());

			if (ProcessFile(sname))
				continue;
		}

		// ok we got this far, its a texture

		if (tex.find(".tga")!=static_cast<size_t>(-1) || tex.find(".jpg")!=static_cast<size_t>(-1))
			tex=tex.substr(0,tex.length()-4);

		// jpegs
		std::string jpeg=tex;
		if (jpeg.find("textures/")!=0)
			jpeg="textures/"+jpeg;

		jpeg+=".jpg";
		if (ProcessFile(jpeg.c_str()))
			continue;

		// tga's
		std::string tga=tex;
		if (tga.find("textures/")!=0)
			tga="textures/"+tga;

		tga+=".tga";
		if (ProcessFile(tga.c_str()))
			continue;

	}

}

void CMainWnd::ParseModel(const char *pathname)
{
	char* modelbuff;
	int len = g_FuncTable.m_pfnLoadFile(pathname, (void**)&modelbuff);

	if (len==0)
		return;

	CMemFile file(modelbuff,len);

	md3_header_t header;

	file.Read(&header,sizeof(header));

	int n, numtags=header.Tag_num*header.BoneFrame_num;

	for (n=0 ; n<numtags ; n++)
	{
		md3_tag_t tag;
		file.Read(&tag,sizeof(md3_tag_t));
	}

	for (n=0 ; n<header.BoneFrame_num ; n++)
	{
		md3_boneframe_t bone;
		file.Read(&bone,sizeof(md3_boneframe_t));
	}

	for (int m=0 ; m<header.Mesh_num ; m++)
	{
		md3_mesh_t mesh;
		file.Read(&mesh,sizeof(md3_mesh_t));

		// skins
		for (n=0 ; n<mesh.Skin_num ; n++)
		{
			char SkinName[68];
			file.Read(&SkinName,sizeof(char)*68);

			// deal with the skin here
			// we add it to the end of the list (if it aint already there)
			// so we catch it later on

			AddToTexList(SkinName);
		}

		// triangles
		for (n=0 ; n<mesh.Triangle_num ; n++)
		{
			int Triangle[3];
			file.Read(&Triangle,sizeof(int)*3);
		}

		// uv's
		for (n=0 ; n<mesh.Vertex_num ; n++)
		{
			float uv[2];
			file.Read(&uv,sizeof(float)*2);
		}

		// vertices
		int numverts=mesh.Vertex_num*mesh.MeshFrame_num;

		for (n=0 ; n<numverts ; n++)
		{
			md3_vertices_t verts;
			file.Read(&verts,sizeof(md3_vertices_t));
		}

	}

// http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=491
g_free(modelbuff);
}

void CMainWnd::AddToTexList(const char *buf)
{
	std::string tex;
	int c=0;

	while (buf[c]!=' ' && buf[c]!='\0')
	{
		tex+=buf[c];
		c++;
	}

	if (!IsInTexList(tex.c_str()))
		texlist.push_back(tex);

}

void CMainWnd::InitPaksInDir(const char *folder)
{
struct dirent *dirlist;
DIR *dir;

dir = opendir (g_strBasePath.GetBuffer());

if (dir != NULL)
{
	while ((dirlist = readdir (dir)) != NULL)
	{
		if (!strstr (dirlist->d_name, ".pk3"))
			continue;

		std::string path=g_strBasePath.GetBuffer();
		path+=dirlist->d_name;

		if (IsExcluded(dirlist->d_name))
			continue;

		CPak *p=new CPak;

		if (p->Open(path.c_str()))
			paklist.push_back(p);
		else
			delete p;
	}
	closedir (dir);
}
}

void CMainWnd::InitPakList()
{
	gtk_label_set_text (GTK_LABEL (m_pStatusLabel[0]), "Building pk3 references");

	if (g_strModPath.GetLength()>0)
		InitPaksInDir(g_strModPath.GetBuffer());

	InitPaksInDir(g_strBasePath.GetBuffer());

	gtk_label_set_text (GTK_LABEL (m_pStatusLabel[0]), "Ready ...");
}

void CMainWnd::FreePakList()
{
	std::list<CPak*>::iterator i;

	for ( i = paklist.begin() ; i != paklist.end() ; i++)
	{
		CPak *p=*i;
		delete p;
	}

	paklist.clear();
}

void CMainWnd::DeleteTempFiles()
{
	std::list<std::string>::iterator i;

	for ( i = temp_files.begin() ; i != temp_files.end() ; i++)
	{
		std::string path=*i;

		remove(path.c_str());
	}

	temp_files.clear();
}

bool CMainWnd::IsInTexList(const char *tex)
{
	for ( std::list<std::string>::const_iterator i = texlist.begin() ; i != texlist.end() ; i++)
	{
		std::string str=*i;

		if (str.compare(tex)==0)
			return TRUE;
	}

	return FALSE;
}

void CMainWnd::SkipLines(FILE *fp, int n)
{
	for (int c=0 ; c<n ; c++)
		std::string line=GetLine(fp);
}

std::string CMainWnd::GetLine(FILE *fp)
{
	std::string line;
	char c='0';

	// grab a line;
	do
	{
		fread(&c,1,1,fp);

		if (c!='\n' && c!='\t' && c!='\r')
			line+=c;

	} while (c!='\n' && !feof(fp));

	return line;

}

std::string CMainWnd::TrimString(const char *str)
{
	std::string out;

	int c=0;
	while (str[c]==' ' || str[c]=='\t')
		c++;

	int e=strlen(str);
	while (str[e]==' ' || str[e]=='\n' || str[e]=='\r')
		e--;

	out=str;
	out=out.substr(c,-1);

	return out;
}

void CMainWnd::ParseShader(const char *pathname, const char *tex)
{
	if (pathname==NULL || tex==NULL)
		return;

	char* shaderbuff;
	int len = g_FuncTable.m_pfnLoadFile(pathname, (void**)&shaderbuff);

	if (len==0)
		return;

	std::string shader=shaderbuff;

	// skip to start of shader
	long pos=shader.find(tex);

	if (pos==-1)	// shader not found????
		return;

	shader=shader.substr(pos);

	bool inStage=FALSE;
	bool inShader=FALSE;

	// step through the lines
	while(1)
	{
		pos=shader.find_first_of('\n');
		if (pos==-1)
			break;

		std::string line=shader.substr(0,pos-1);
		shader=shader.substr(pos+1,-1);

		line=TrimString(line.c_str());

		if (line=="")
			continue;

		if (line[0]=='/' && line[1]=='/')
			continue;

		if (line[0]=='}')
		{
			if (!inStage)
				break;
			else
				inStage=FALSE;

			continue;
		}

		if (line[0]=='{')
		{
			if (!inShader)
			{
				inShader=TRUE;
			}
			else if (inShader)
			{
				inStage=TRUE;
			}

			continue;
		}

		// check for map/animmap e.t.c;

		if (inStage)
		{

			if (line.substr(0,3).compare("map")==0)
			{
				std::string map=line.substr(4,-1);
				AddToTexList(map.c_str());
				continue;
			}

			if (line.substr(0,8).compare("skyparms")==0)
			{
				std::string map=line.substr(9,-1);
				AddToTexList(map.c_str());
				continue;
			}

			if (line.substr(0,8).compare("clampmap")==0)
			{
				std::string map=line.substr(9,-1);
				AddToTexList(map.c_str());
				continue;
			}

			if (line.substr(0,7).compare("animmap")==0)
			{
				std::string map=line.substr(9,-1);
				map=TrimString(map.c_str());

				// skip past numbers
				long p=map.find_first_of(' ');
				map=map.substr(p+1,-1);

				while (1)
				{
					long pos=map.find_first_of(' ');

					if (pos==-1)
					{
						AddToTexList(map.c_str());
						break;
					}

					std::string tex=map.substr(0,pos);
					AddToTexList(tex.c_str());

					map=map.substr(pos+1,-1);
				}
			}
		}
	}
// http://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=491
g_free(shaderbuff);
}

bool CMainWnd::ProcessFile(const char *nm,const char *suffix,const char *folder)
{
	std::string name=nm;
	name=name.substr(0,name.length()-4);
	name+=suffix;

	std::string path;

	if (g_strModPath.GetLength()>0)
		path=g_strModPath.GetBuffer();
	else
		path=g_strBasePath.GetBuffer();

	path+=folder;
	path+="/";
	path+=name;

	std::string rpath=folder;
	rpath+="/";
	rpath+=name;

	if (IsExcluded(rpath.c_str()))	// quit out with a FALSE so we try others
		return FALSE;

	FILE *fp;
	fp=fopen(path.c_str(),"rb");

	if (fp)
	{
		if (!pak.IsInPak(rpath.c_str()))
		{
			m_Tree.AddPath(rpath.c_str());
			pak.AddExternalFile(rpath.c_str(),path.c_str());
		}

		fclose(fp);
		return TRUE;
	}
	else
		return FindInAPk3(rpath.c_str());


	return FALSE;
}

bool CMainWnd::ProcessFile(const char *nm)
{
	std::string name=nm;

	if (IsExcluded(name.c_str()))	// quit out with a TRUE so we think it was added
		return TRUE;

	std::string path;

	if (g_strModPath.GetLength()>0)
		path=g_strModPath.GetBuffer();
	else
		path=g_strBasePath.GetBuffer();

	path+=name;

	FILE *fp;
	fp=fopen(path.c_str(),"rb");

	if (fp)
	{
		if (!pak.IsInPak(name.c_str()))
		{
			m_Tree.AddPath(name.c_str());
			pak.AddExternalFile(name.c_str(),path.c_str());
		}

		return TRUE;
	}
	else
	{
		return FindInAPk3(name.c_str());
	}


	return FALSE;
}

bool CMainWnd::FindInAPk3(const char *name)
{
	std::list<CPak*>::iterator i;

	for ( i = paklist.begin() ; i != paklist.end() ; i++)
	{
		CPak *p=*i;

		if (p->m_filename.CompareNoCase(pak.m_filename)==0)
			continue;

		std::string fname=p->m_filename.GetBuffer();
		long pos=fname.find_last_of('/');
		fname=fname.substr(pos+1,-1);

		if (IsExcluded(fname.c_str()))
			continue;

		if (p->IsInPak(name))
		{
			pak_entry pe=p->FindPE(name);
			m_Tree.AddPath(name);
			pak.AddPakFile(name,p->m_filename.GetBuffer(),pe.compressed,pe.uncompressed);
			return TRUE;
		}
	}
	return FALSE;
}

void CMainWnd::OnAdd()
{
	std::string path;
	if (g_strModPath.GetLength()>0)
		path=g_strModPath.GetBuffer();
	else
		path=g_strBasePath.GetBuffer();

	//char *filename=file_dialog(TRUE,"Choose a file...",path.c_str(),"*.*");
	gchar *filename=g_FuncTable.m_pfnFileDialog(m_pMainWnd,TRUE,"Choose a file...",path.c_str(),"pk3man");

	if (filename==NULL)
		return;

	int c=0;

	while (filename[c]!='\0')
	{
		if (filename[c]=='\\')
			filename[c]='/';

		c++;
	}

	std::string fname=filename;

	long p=path.length();
	fname=fname.substr(p,-1);

	if (pak.IsInPak((char*)fname.c_str()))
		return;

	pak.AddExternalFile((char *)fname.c_str(),(char *)filename);
	m_Tree.AddPath((char *)fname.c_str());

}

void CMainWnd::OnDelete()
{
	std::string path=m_Tree.GetSelected();

	if (path=="")
		return;

	pak.Delete(path.c_str());

	//m_Tree.RemovePath(path.c_str());

	// dodgy hack but works for now
	m_Tree.Clear();
	UpdateTree();

}

void CMainWnd::OnRename()
{
	std::string path=m_Tree.GetSelected();

	if (path=="")
		return;

	CRenameDlg dlg;
	dlg.m_Name=path;

	if (dlg.DoModal()==IDOK)
	{
		pak.Rename(path.c_str(),dlg.m_Name.c_str());
		m_Tree.Clear();
		UpdateTree();
	}

}

void CMainWnd::UpdateTree()
{
	// clear the tree and fill it with the items in the pak

	for ( std::list<pak_entry>::const_iterator iEntry = pak.entries.begin() ; iEntry != pak.entries.end() ; iEntry++)
	{
		pak_entry pe=*iEntry;

		std::string name;
		if (pe.rename)
			name=pe.newname;
		else
			name=pe.filename;

		m_Tree.AddPath(name.c_str());
	}

}

void CMainWnd::OnExtract()
{
	std::string path=m_Tree.GetSelected();

	if (path=="")
		return;

	pak_entry pe=pak.FindPE((char*)path.c_str());

	std::string bpath;
	if (g_strModPath.GetLength()>0)
		bpath=g_strModPath.GetBuffer();
	else
		bpath=g_strBasePath.GetBuffer();

		//char *pathto=dir_dialog("Extract to ...",bpath.c_str());
	gchar *pathto=g_FuncTable.m_pfnDirDialog(m_pMainWnd,"Extract to...",bpath.c_str());

	if (pathto==NULL)
		return;

	long pos=path.find_last_of('/');
	std::string name=path.substr(pos+1);

	std::string cpath=pathto;
	cpath+=name;

	if (pe.pakname.compare(pak.m_filename.GetBuffer())==0)
	{
		// its in pak
		pak.ExtractTo((char*)path.c_str(),(char*)cpath.c_str());
		return;
	}

}

void CMainWnd::OnView()
{
	std::string path=m_Tree.GetSelected();

	if (path=="")
		return;

	pak_entry pe=pak.FindPE(path.c_str());

	long p=pe.filename.find_last_of('/');
	std::string name=pe.filename.substr(p+1,-1);
	std::string temppath=g_strTempPath.GetBuffer()+name;

	if (pe.frompak && pe.pakname.compare(pak.m_filename.GetBuffer())==0)
	{
		// its in pak
		if (!pak.ExtractTo(path.c_str(),(char *)temppath.c_str()))
			return;

		temp_files.push_back(temppath);

		// view it
	#ifdef WIN32
		HANDLE hInst=ShellExecute(NULL,"open",(LPCTSTR)temppath.c_str(),NULL,NULL,SW_SHOWNORMAL);
	#endif

		return;
	}

	if (!pe.frompak)
	{
#ifdef WIN32
		HANDLE hInst=ShellExecute(NULL,"open",(LPCTSTR)pe.pathname.c_str(),NULL,NULL,SW_SHOWNORMAL);
#endif
		return;
	}

	if (pe.frompak)
	{
		CPak p;
		if (!p.Open(pe.pakname.c_str()))
			return;

		if (!p.ExtractTo(path.c_str(),(char *)temppath.c_str()))
			return;

		temp_files.push_back(temppath);

		// view it
#ifdef WIN32
		HANDLE hInst=ShellExecute(NULL,"open",(LPCTSTR)temppath.c_str(),NULL,NULL,SW_SHOWNORMAL);
#endif
		return;
	}
}

#ifdef WIN32
void ShortToLong(CString &thePath)
{
	WIN32_FIND_DATA wfd;
	std::string spath=thePath.GetBuffer();
	std::string lpath="";
	while(1)
	{
		FindFirstFile(spath.c_str(),&wfd);
		std::string fname=wfd.cFileName;
		lpath=fname+"\\"+lpath;
		long pos=spath.find_last_of('\\');
		spath=spath.substr(0,pos);
		pos=spath.find_last_of('\\');

		if (pos==-1)
		{
			lpath=spath+"\\"+lpath;
			break;
		}
	}
	thePath=lpath.c_str();
}
#endif

void CMainWnd::GetPaths()
{
#if 0 //mattn
#ifdef WIN32

	// convert it to long format
	ShortToLong(g_strBasePath);

	// mod path
	if (strcmp(mode,"Quake III Team Arena")==0)
	{
		g_strModPath=g_strBasePath;
		long p=g_strModPath.Find("\\baseq3");
		g_strModPath=g_strModPath.Left(p);
		g_strModPath+="\\missionpack\\";

		g_FuncTable.m_pfnQE_ConvertDOSToUnixName ((char *)g_strModPath.GetBuffer(), g_strModPath.GetBuffer());
	}

	g_FuncTable.m_pfnQE_ConvertDOSToUnixName ((char *)g_strBasePath.GetBuffer(), g_strBasePath.GetBuffer());

	// apppath
	// get path to the editor
	char* pBuffer = g_strAppPath.GetBufferSetLength(_MAX_PATH + 1);
	GetModuleFileName(NULL, pBuffer, _MAX_PATH);
	pBuffer[g_strAppPath.ReverseFind('\\') + 1] = '\0';
	g_strAppPath.ReleaseBuffer();
	g_FuncTable.m_pfnQE_ConvertDOSToUnixName ((char *)g_strAppPath.GetBuffer(), g_strAppPath.GetBuffer());

	char temp_path[_MAX_PATH];
	GetTempPath(_MAX_PATH,temp_path);
	g_strTempPath=temp_path;

#endif

#ifdef __linux__
	Str tmp;
	tmp = g_strBasePath.GetBuffer();
	tmp += "/../";

	// NOTE: we build g_strAppPath with a '/' (or '\' on WIN32)
	// it's a general convention in Radiant to have the slash at the end of directories
	char real[PATH_MAX];
	realpath (tmp.GetBuffer(), real);
	if (real[strlen(real)-1] != '/')
	strcat(real, "/");
	g_strAppPath = real;

	g_strTempPath="~/.tmp";
#endif

	extern const char *PLUGIN_NAME;
	g_strBitmapsPath = g_FuncTable.m_pfnPathForPluginName(PLUGIN_NAME);
	g_strBitmapsPath += "bitmaps/";
#endif
}

void CMainWnd::UpdateStatus()
{
	std::string path=m_Tree.GetSelected();

	if (path=="")
	{
		gtk_label_set_text (GTK_LABEL (m_pStatusLabel[0]), "");
		gtk_label_set_text (GTK_LABEL (m_pStatusLabel[1]), "");
		gtk_label_set_text (GTK_LABEL (m_pStatusLabel[2]), "");

		return;
	}

	pak_entry pe=pak.FindPE(path.c_str());

	std::string p=path;
	long pos=p.find_last_of('/');
	p=p.substr(pos+1);
	gtk_label_set_text (GTK_LABEL (m_pStatusLabel[0]), p.c_str());

	char lbl[255];
	sprintf(lbl," %ldk ",pe.uncompressed/1000);
	gtk_label_set_text (GTK_LABEL (m_pStatusLabel[1]), lbl);

	sprintf(lbl," %ldk ",pe.compressed/1000);
	gtk_label_set_text (GTK_LABEL (m_pStatusLabel[2]), lbl);

}

void CMainWnd::HandleDrop()
{
	std::string path=m_Tree.GetSelected();
	bool file_selected=TRUE;

	if (path=="")
		file_selected=FALSE;

	//if (m_mnuDrop == NULL) // first time, load it up
	//{
		GtkAccelGroup *accel;//, *menu_in_menu_accel;
		GtkWidget *menu;//, *menu_in_menu, *item, *submenu;

		menu = m_mnuDrop = gtk_menu_new ();
		accel = gtk_accel_group_new ();
		gtk_menu_set_accel_group (GTK_MENU (menu), accel);

		//menu_in_menu = create_menu_in_menu (menu, "Select", accel, &menu_in_menu_accel);

		create_menu_item (menu, "Add file", accel,
			GTK_SIGNAL_FUNC (HandleCommand), ID_ADD);

		if (file_selected)
		{
			create_menu_item (menu, "View File", accel,
			GTK_SIGNAL_FUNC (HandleCommand), ID_VIEW);

			create_menu_item (menu, "Extract file", accel,
			GTK_SIGNAL_FUNC (HandleCommand), ID_EXTRACT);

			menu_separator (menu);

			create_menu_item (menu, "Rename File", accel,
			GTK_SIGNAL_FUNC (HandleCommand), ID_RENAME);

			create_menu_item (menu, "Delete file", accel,
			GTK_SIGNAL_FUNC (HandleCommand), ID_DELETE);
		}
	//}

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 1, GDK_CURRENT_TIME);
}


////////////////////////////////////////////
// some handy stuff ripped from GtkRadiant
////////////////////////////////////////////


GtkWidget* create_menu_in_menu (GtkWidget *menu, gchar *label, GtkAccelGroup *menu_accel,
				GtkAccelGroup **submenu_accel)
{
	GtkWidget *item, *submenu;
	guint tmp_key;

	item = gtk_menu_item_new_with_label ("");
	tmp_key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child), label);
	gtk_widget_add_accelerator (item, "activate_item", menu_accel, tmp_key, 0, (GtkAccelFlags)0);
	gtk_widget_show (item);
	gtk_container_add (GTK_CONTAINER (menu), item);

	submenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);
	*submenu_accel = gtk_accel_group_new ();
	gtk_menu_set_accel_group (GTK_MENU (submenu), *submenu_accel);

	return submenu;
}

GtkWidget* create_menu_item (GtkWidget *menu, gchar *label, GtkAccelGroup *menu_accel,
				GtkSignalFunc func, int id)
{
	GtkWidget *item;
	guint tmp_key;

#ifdef NO_UNDERSCORE
	char label_tmp[1024];
	strcpy( label_tmp, label );
	if (char* c = strchr (label_tmp, '_'))
		while (*c)
		*c++ = *(c+1);
	item = gtk_menu_item_new_with_label (label_tmp);
#else
	item = gtk_menu_item_new_with_label ("");
	tmp_key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child), label);
	gtk_widget_add_accelerator (item, "activate_item", menu_accel, tmp_key, 0, (GtkAccelFlags)0);
#endif
	gtk_widget_show (item);
	gtk_container_add (GTK_CONTAINER (menu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC (func), GINT_TO_POINTER (id));


	return item;
}

GtkWidget* menu_separator (GtkWidget *menu)
{
	GtkWidget *menu_item = gtk_menu_item_new ();
	gtk_menu_append (GTK_MENU (menu), menu_item);
	gtk_widget_set_sensitive (menu_item, FALSE);
	gtk_widget_show (menu_item);
	return menu_item;
}
