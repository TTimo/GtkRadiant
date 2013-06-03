/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// classes used for describing geometry information from q3map feedback
//

#ifndef __Q3MAP_FEEDBACK__
#define __Q3MAP_FEEDBACK__

#if defined( _WIN32 )
  // required for static linking libxml on Windows
  #define LIBXML_STATIC
#endif
#include "libxml/parser.h"

// a select message with a brush/entity select information
class CSelectMsg : public ISAXHandler
{
enum { SELECT_MESSAGE, SELECT_BRUSH } ESelectState;
GString *message;
int entitynum, brushnum;
public:
CSelectMsg() { ESelectState = SELECT_MESSAGE; }
// SAX interface
void saxStartElement( message_info_t *ctx, const xmlChar *name, const xmlChar **attrs );
void saxEndElement( message_info_t *ctx, const xmlChar *name );
void saxCharacters( message_info_t *ctx, const xmlChar *ch, int len );
// for use in the dialog window
char *getName() { return message->str; }
void Highlight();
void DropHighlight() { }
};

class CPointMsg : public ISAXHandler, public IGL2DWindow
{
enum { POINT_MESSAGE, POINT_POINT } EPointState;
GString *message;
vec3_t pt;
int refCount;
public:
CPointMsg() { EPointState = POINT_MESSAGE; refCount = 0; }
// SAX interface
void saxStartElement( message_info_t *ctx, const xmlChar *name, const xmlChar **attrs );
void saxEndElement( message_info_t *ctx, const xmlChar *name );
void saxCharacters( message_info_t *ctx, const xmlChar *ch, int len );
// for use in the dialog window
char *getName() { return message->str; }
void Highlight();
void DropHighlight();

// IGL2DWindow interface --------------------------------
// Increment the number of references to this object
void IncRef() { refCount++; }
// Decrement the reference count
void DecRef() {
	refCount--; if ( refCount <= 0 ) {
		delete this;
	}
}
void Draw2D( VIEWTYPE vt );
};

class CWindingMsg : public ISAXHandler, public IGL2DWindow
{
enum { WINDING_MESSAGE, WINDING_WINDING } EPointState;
GString *message;
vec3_t wt[256];
int numpoints;
int refCount;
public:
CWindingMsg() { EPointState = WINDING_MESSAGE; refCount = 0; numpoints = 0; }
// SAX interface
void saxStartElement( message_info_t *ctx, const xmlChar *name, const xmlChar **attrs );
void saxEndElement( message_info_t *ctx, const xmlChar *name );
void saxCharacters( message_info_t *ctx, const xmlChar *ch, int len );
// for use in the dialog window
char *getName() { return message->str; }
void Highlight();
void DropHighlight();

// IGL2DWindow interface --------------------------------
// Increment the number of references to this object
void IncRef() { refCount++; }
// Decrement the reference count
void DecRef() {
	refCount--; if ( refCount <= 0 ) {
		delete this;
	}
}
void Draw2D( VIEWTYPE vt );
};

class CDbgDlg : public Dialog
{
GPtrArray *m_pFeedbackElements;
// the list widget we use in the dialog
GtkListStore* m_clist;
ISAXHandler *m_pHighlight;
public:
CDbgDlg() { m_pFeedbackElements = g_ptr_array_new(); m_pHighlight = NULL; }
virtual ~CDbgDlg() { ClearFeedbackArray(); }
// refresh items
void          Push( ISAXHandler * );
// clean the debug window, release all ISAXHanlders we have
void          Init();
ISAXHandler   *GetElement( gint row );
void          SetHighlight( gint row );
void          DropHighlight();
protected:
void          BuildDialog();
void          ClearFeedbackArray();
};

extern CDbgDlg g_DbgDlg;

#endif
