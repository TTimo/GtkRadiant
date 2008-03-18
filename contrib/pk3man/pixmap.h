// PixMap.h: interface for the CPixMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PIXMAP_H__5B978BDF_7DD1_4BA7_91B5_CCD49AA304AC__INCLUDED_)
#define AFX_PIXMAP_H__5B978BDF_7DD1_4BA7_91B5_CCD49AA304AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

class CPixMap  
{
public:
	CPixMap();
	virtual ~CPixMap();

	static void load_pixmap (const char* filename, GdkPixmap **gdkpixmap, GdkBitmap **mask);
	static void bmp_to_pixmap (const char* filename, GdkPixmap **pixmap, GdkBitmap **mask);
	static unsigned char *load_bitmap_file (const char* filename, guint16* width, guint16* height);
	static GtkWidget *new_pixmap (GtkWidget* parent, char* filename);
	static GtkWidget* pixmap_from_char(GtkWidget *widget, gchar **xpm_data);

};

#endif // !defined(AFX_PIXMAP_H__5B978BDF_7DD1_4BA7_91B5_CCD49AA304AC__INCLUDED_)
