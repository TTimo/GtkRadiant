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

//
// Linux stuff
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include <gtk/gtk.h>
#include <sys/stat.h>
#include "gtkmisc.h"
#include <glib/gi18n.h>
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#include <unistd.h>
#include <X11/keysym.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#endif
// for the logging part
#include <fcntl.h>
#include <sys/types.h>

QEGlobals_t g_qeglobals;
QEGlobals_GUI_t g_qeglobals_gui;

// leo: Track memory allocations for debugging
// NOTE TTimo this was never used and probably not relevant
//   there are tools to do that
#ifdef MEM_DEBUG

static GList *memblocks;

void* debug_malloc( size_t size, const char* file, int line ){
	void *buf = g_malloc( size + 8 );

	*( (const char**)buf ) = file;
	buf = (char*)buf + 4;
	*( (int*)buf ) = line;
	buf = (char*)buf + 4;

	memblocks = g_list_append( memblocks, buf );

	return buf;
}

void debug_free( void *buf, const char* file, int line ){
	const char *f;
	int l;

	if ( g_list_find( memblocks, buf ) ) {
		memblocks = g_list_remove( memblocks, buf );

		buf = (char*)buf - 4;
		l = *( (int*)buf );
		buf = (char*)buf - 4;
		f = *( (const char**)buf );

		Sys_FPrintf( SYS_DBG, "free: %s %d", file, line );
		Sys_FPrintf( SYS_DBG, " allocated: %s %d\n", f, l );

		g_free( buf );
	}
//  else
//    free (buf); // from qmalloc, will leak unless we add this same hack to cmdlib
}

#endif

vec_t Rad_rint( vec_t in ){
	if ( g_PrefsDlg.m_bNoClamp ) {
		return in;
	}
	else{
		return (float)floor( in + 0.5 );
	}
}

void WINAPI QE_CheckOpenGLForErrors( void ){
	char strMsg[1024];
	int i = qglGetError();
	if ( i != GL_NO_ERROR ) {
		if ( i == GL_OUT_OF_MEMORY ) {
			sprintf( strMsg, _( "OpenGL out of memory error %s\nDo you wish to save before exiting?" ), qgluErrorString( (GLenum)i ) );
			if ( gtk_MessageBox( g_pParentWnd->m_pWidget, strMsg, _( "Radiant Error" ), MB_YESNO ) == IDYES ) {
				Map_SaveFile( NULL, false );
			}
			_exit( 1 );
		}
		else
		{
			Sys_FPrintf( SYS_WRN, "WARNING: OpenGL Error %s\n", qgluErrorString( (GLenum)i ) );
		}
	}
}

// NOTE: don't this function, use VFS instead
char *ExpandReletivePath( char *p ){
	static char temp[1024];
	const char  *base;

	if ( !p || !p[0] ) {
		return NULL;
	}
	if ( p[0] == '/' || p[0] == '\\' ) {
		return p;
	}

	base = ValueForKey( g_qeglobals.d_project_entity, "basepath" );
	sprintf( temp, "%s/%s", base, p );
	return temp;
}

char *copystring( char *s ){
	char    *b;
	b = (char*)malloc( strlen( s ) + 1 );
	strcpy( b,s );
	return b;
}


bool DoesFileExist( const char* pBuff, long& lSize ){
	FileStream file;
	if ( file.Open( pBuff, "r" ) ) {
		lSize += file.GetLength();
		file.Close();
		return true;
	}
	return false;
}

void Map_Snapshot(){
	CString strMsg;

	// I hope the modified flag is kept correctly up to date
	if ( !modified ) {
		return;
	}

	// we need to do the following
	// 1. make sure the snapshot directory exists (create it if it doesn't)
	// 2. find out what the lastest save is based on number
	// 3. inc that and save the map
	CString strOrgPath, strOrgFile;
	ExtractPath_and_Filename( currentmap, strOrgPath, strOrgFile );
	AddSlash( strOrgPath );
	strOrgPath += "snapshots";
	bool bGo = true;
	struct stat Stat;
	if ( stat( strOrgPath, &Stat ) == -1 ) {
#ifdef _WIN32
		bGo = ( _mkdir( strOrgPath ) != -1 );
#endif

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
		bGo = ( mkdir( strOrgPath,0755 ) != -1 );
#endif
	}
	AddSlash( strOrgPath );
	if ( bGo ) {
		int nCount = 0;
		long lSize = 0;
		CString strNewPath;
		strNewPath = strOrgPath;
		strNewPath += strOrgFile;
		
		// QB - snapshots now follow the format: <mapname>.<snapnum>.<ext>
		//      **NOTE** atm snapshots must end with a .map (or .xmap) ext (this is why they were broken)
		CString strOldEXT = "map"; //default to .map
		const char* type = strrchr( strOrgFile.GetBuffer(),'.' );
		if ( type != NULL ) { strOldEXT = ++type; }; // get the ext for later.
		StripExtension(strNewPath); // then strip it from the new path
		//
		
		CString strFile;
		while ( bGo )
		{
			char buf[PATH_MAX];
			//sprintf( buf, "%s.%i", strNewPath.GetBuffer(), nCount );
			// snapshot will now end with a known ext.
			sprintf( buf, "%s.%i.%s", strNewPath.GetBuffer(), nCount, strOldEXT.GetBuffer() );
			strFile = buf;
			bGo = DoesFileExist( strFile, lSize );
			nCount++;
		}
		// strFile has the next available slot
		Map_SaveFile( strFile, false );
		// it is still a modified map (we enter this only if this is a modified map)
		Sys_SetTitle( currentmap );
		Sys_MarkMapModified();
		if ( lSize > 12 * 1024 * 1024 ) { // total size of saves > 4 mb
			Sys_Printf( "The snapshot files in %s total more than 4 megabytes. You might consider cleaning up.", strOrgPath.GetBuffer() );
		}
	}
	else
	{
		strMsg.Format( "Snapshot save failed.. unabled to create directory\n%s", strOrgPath.GetBuffer() );
		gtk_MessageBox( g_pParentWnd->m_pWidget, strMsg );
	}
	strOrgPath = "";
	strOrgFile = "";
}
/*
   ===============
   QE_CheckAutoSave

   If five minutes have passed since making a change
   and the map hasn't been saved, save it out.
   ===============
 */


void QE_CheckAutoSave( void ){
	static time_t s_start;
	time_t now;
	time( &now );

	if ( modified != 1 || !s_start ) {
		s_start = now;
		return;
	}

	if ( ( now - s_start ) > ( 60 * g_PrefsDlg.m_nAutoSave ) ) {
		if ( g_PrefsDlg.m_bAutoSave ) {
			CString strMsg;
			strMsg = g_PrefsDlg.m_bSnapShots ? "Autosaving snapshot..." : "Autosaving...";
			Sys_Printf( strMsg );
			Sys_Printf( "\n" );
			Sys_Status( strMsg,0 );

			// only snapshot if not working on a default map
			if ( g_PrefsDlg.m_bSnapShots && stricmp( currentmap, "unnamed.map" ) != 0 ) {
				Map_Snapshot();
			}
			else
			{
				Map_SaveFile( ValueForKey( g_qeglobals.d_project_entity, "autosave" ), false );
			}

			Sys_Status( "Autosaving...Saved.", 0 );
			modified = 2;
		}
		else
		{
			Sys_Printf( "Autosave skipped...\n" );
			Sys_Status( "Autosave skipped...", 0 );
		}
		s_start = now;
	}
}


// NOTE TTimo we don't like that BuildShortPathName too much
//   the VFS provides a vfsCleanFileName which should perform the cleanup tasks
//   in the long run I'd like to completely get rid of this

// used to be disabled, but caused problems

// can't work with long win32 names until the BSP commands are not working differently
#ifdef _WIN32
int BuildShortPathName( const char* pPath, char* pBuffer, int nBufferLen ){
	char *pFile = NULL;
	int nResult = GetFullPathName( pPath, nBufferLen, pBuffer, &pFile );
	nResult = GetShortPathName( pPath, pBuffer, nBufferLen );
	if ( nResult == 0 ) {
		strcpy( pBuffer, pPath );               // Use long filename
	}
	return nResult;
}
#endif

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
int BuildShortPathName( const char* pPath, char* pBuffer, int nBufferLen ){
	// remove /../ from directories
	const char *scr = pPath; char *dst = pBuffer;
	for ( int i = 0; ( i < nBufferLen ) && ( *scr != 0 ); i++ )
	{
		if ( *scr == '/' && *( scr + 1 ) == '.' && *( scr + 2 ) == '.' ) {
			scr += 3;
			while ( dst != pBuffer && *( --dst ) != '/' )
			{
				i--;
			}
		}

		*dst = *scr;

		scr++; dst++;
	}
	*dst = 0;

	return strlen( pBuffer );
}
#endif

/*
   const char *g_pPathFixups[]=
   {
   "basepath",
   "autosave",
   };

   const int g_nPathFixupCount = sizeof(g_pPathFixups) / sizeof(const char*);

   void QE_CheckProjectEntity()
   {
   char *pFile;
   char pBuff[PATH_MAX];
   char pNewPath[PATH_MAX];
   for (int i = 0; i < g_nPathFixupCount; i++)
   {
    char *pPath = ValueForKey (g_qeglobals.d_project_entity, g_pPathFixups[i]);

    strcpy (pNewPath, pPath);
    if (pPath[0] != '\\' && pPath[0] != '/')
      if (GetFullPathName(pPath, PATH_MAX, pBuff, &pFile))
          strcpy (pNewPath, pBuff);

    BuildShortPathName (pNewPath, pBuff, PATH_MAX);

    // check it's not ending with a filename seperator
    if (pBuff[strlen(pBuff)-1] == '/' || pBuff[strlen(pBuff)-1] == '\\')
    {
      Sys_FPrintf(SYS_WRN, "WARNING: \"%s\" path in the project file has an ending file seperator, fixing.\n", g_pPathFixups[i]);
      pBuff[strlen(pBuff)-1]=0;
    }

    SetKeyValue(g_qeglobals.d_project_entity, g_pPathFixups[i], pBuff);
   }
   }
 */

void HandleXMLError( void* ctxt, const char* text, ... ){
	va_list argptr;
	static char buf[32768];

	va_start( argptr,text );
	vsprintf( buf, text, argptr );
	Sys_FPrintf( SYS_ERR, "XML %s\n", buf );
	va_end( argptr );
}

#define DTD_BUFFER_LENGTH 1024
xmlDocPtr ParseXMLStream( IDataStream *stream, bool validate = false ){
	xmlDocPtr doc = NULL;
	bool wellFormed = false, valid = false;
	int res, size = 1024;
	char chars[1024];
	xmlParserCtxtPtr ctxt;

	//if(validate)
	//  xmlDoValidityCheckingDefaultValue = 1;
	//else
	xmlDoValidityCheckingDefaultValue = 0;
	xmlSetGenericErrorFunc( NULL, HandleXMLError );

	// SPoG
	// HACK: use AppPath to resolve DTD location
	// do a buffer-safe string copy and concatenate
	int i;
	char* w;
	const char* r;
	char buf[DTD_BUFFER_LENGTH];

	w = buf;
	i = 0;
	// copy
	//assert(g_strAppPath.GetBuffer() != NULL);
	for ( r = g_strAppPath.GetBuffer(); i < DTD_BUFFER_LENGTH && *r != '\0'; i++, r++ ) w[i] = *r;
	// concatenate
	for ( r = "dtds/"; i < DTD_BUFFER_LENGTH && *r != '\0'; i++, r++ ) w[i] = *r;
	// terminate
	w[i] = '\0';

	if ( i == DTD_BUFFER_LENGTH ) {
		HandleXMLError( NULL, "ERROR: buffer overflow: DTD path length too large\n" );
		return NULL;
	}

	res = stream->Read( chars, 4 );
	if ( res > 0 ) {
		ctxt = xmlCreatePushParserCtxt( NULL, NULL, chars, res, buf );

		while ( ( res = stream->Read( chars, size ) ) > 0 )
		{
			xmlParseChunk( ctxt, chars, res, 0 );
		}
		xmlParseChunk( ctxt, chars, 0, 1 );
		doc = ctxt->myDoc;

		wellFormed = ( ctxt->wellFormed == 1 );
		valid = ( ctxt->valid == 1 );

		xmlFreeParserCtxt( ctxt );
	}

	if ( wellFormed && ( !validate || ( validate && valid ) ) ) {
		return doc;
	}

	if ( doc != NULL ) {
		xmlFreeDoc( doc );
	}

	return NULL;
}

xmlDocPtr ParseXMLFile( const char* filename, bool validate = false ){
	FileStream stream;
	if ( stream.Open( filename, "r" ) ) {
		return ParseXMLStream( &stream, validate );
	}

	Sys_FPrintf( SYS_ERR, "Failed to open file: %s\n",filename );
	return NULL;
}

// copy a string r to a buffer w
// replace $string as appropriate
static void ReplaceTemplates( char* w, const char* r ){
	const char *p;
	const char *__ENGINEPATH = "TEMPLATEenginepath";
	const char *__USERHOMEPATH = "TEMPLATEuserhomepath";
	const char *__TOOLSPATH = "TEMPLATEtoolspath";
	const char *__EXECPATH = "TEMPLATEexecpath";
	const char *__BASEDIR = "TEMPLATEbasedir";
	const char *__APPPATH = "TEMPLATEapppath";
	const char *__Q3MAP2 = "TEMPLATEq3map2";

	// iterate through string r
	while ( *r != '\0' )
	{
		// check for special character
		if ( *r == '$' ) {
			if ( strncmp( r + 1, __ENGINEPATH, strlen( __ENGINEPATH ) ) == 0 ) {
				r += strlen( __ENGINEPATH ) + 1;
				p = g_pGameDescription->mEnginePath.GetBuffer();
			}
			else if ( strncmp( r + 1, __USERHOMEPATH, strlen( __USERHOMEPATH ) ) == 0 ) {
				r += strlen( __USERHOMEPATH ) + 1;
				p = g_qeglobals.m_strHomeGame.GetBuffer();
			}
			else if ( strncmp( r + 1, __BASEDIR, strlen( __BASEDIR ) ) == 0 ) {
				r += strlen( __BASEDIR ) + 1;
				p = g_pGameDescription->mBaseGame;
			}
			else if ( strncmp( r + 1, __TOOLSPATH, strlen( __TOOLSPATH ) ) == 0 ) {
				r += strlen( __TOOLSPATH ) + 1;
				p = g_strGameToolsPath.GetBuffer();
			}
			else if ( strncmp( r + 1, __EXECPATH, strlen( __EXECPATH ) ) == 0 ) {
				r += strlen( __EXECPATH ) + 1;
				p = g_strExecutablesPath.GetBuffer();
			}
			else if ( strncmp( r + 1, __APPPATH, strlen( __APPPATH ) ) == 0 ) {
				r += strlen( __APPPATH ) + 1;
				p = g_strAppPath.GetBuffer();
			}
			else if ( strncmp( r + 1, __Q3MAP2, strlen( __Q3MAP2 ) ) == 0 ) {
				r += strlen( __Q3MAP2 ) + 1;
				// see https://github.com/TTimo/GtkRadiant/issues/116
#ifdef _WIN32
				if ( g_PrefsDlg.m_bx64q3map2 ) {
				  p = "x64/q3map2";
				} else
#endif
				{
				  p = "q3map2";
				}
			}
			else
			{
				r++;
				p = "$";
			}

			while ( *p != '\0' ) *w++ = *p++;
		}
		else{ *w++ = *r++; }
	}
	*w = '\0';
}

/*
Load up a project file to get the current version
 */
int QE_GetTemplateVersionForProject( const char * projectfile ) {
  xmlDocPtr doc;
  xmlNodePtr node, project;
  int ret;

  Sys_Printf( "Scanning template version in %s\n", projectfile );
  doc = ParseXMLFile( projectfile, true );
  if ( doc == NULL ) {
    Sys_FPrintf( SYS_ERR, "ERROR: XML parse failed %s\n", projectfile );
    return 0;
  }
  node = doc->children;
  while ( node != NULL && node->type != XML_DTD_NODE ) {
    node = node->next;
  }
  if ( node == NULL || strcmp( (char*)node->name, "project" ) != 0 ) {
    Sys_FPrintf( SYS_ERR, "ERROR: invalid file type %s\n", projectfile );
    xmlFree( doc );
    return 0;
  }
  while ( node->type != XML_ELEMENT_NODE ) {
    node = node->next;
  }
  // <project>
  project = node;

  for ( node = project->children; node != NULL; node = node->next ) {
    if ( node->type != XML_ELEMENT_NODE ) {
      continue;
    }
    if ( strcmp( (char*)node->properties->children->content, "template_version" ) == 0 ) {
      ret = atoi( (char*)node->properties->next->children->content );
      xmlFreeDoc( doc );
      return ret;
    }
  }
  Sys_FPrintf( SYS_WRN, "Version key not found in %s\n", projectfile );
  xmlFreeDoc( doc );
  return 0;
}

/*
   ===========
   QE_LoadProject
   NOTE: rather than bumping "version", consider bumping "template_version" (see above)
   NOTE: when QE_LoadProject is called, the prefs are updated with path to the latest project and saved on disk
   ===========
 */
bool QE_LoadProject( const char *projectfile ){
	char buf[1024];
	xmlDocPtr doc;
	xmlNodePtr node, project;

	Sys_Printf( "Loading project file: \"%s\"\n", projectfile );
	doc = ParseXMLFile( projectfile, true );

	if ( doc == NULL ) {
		return false;
	}

	node = doc->children;
	while ( node != NULL && node->type != XML_DTD_NODE ) node = node->next;
	if ( node == NULL || strcmp( (char*)node->name, "project" ) != 0 ) {
		Sys_FPrintf( SYS_ERR, "ERROR: invalid file type\n" );
		return false;
	}

	while ( node->type != XML_ELEMENT_NODE ) node = node->next;
	// <project>
	project = node;

	if ( g_qeglobals.d_project_entity != NULL ) {
		Entity_Free( g_qeglobals.d_project_entity );
	}
	g_qeglobals.d_project_entity = Entity_Alloc();

	for ( node = project->children; node != NULL; node = node->next )
	{
		if ( node->type != XML_ELEMENT_NODE ) {
			continue;
		}

		// <key>
		ReplaceTemplates( buf, (char*)node->properties->next->children->content );

		SetKeyValue( g_qeglobals.d_project_entity, (char*)node->properties->children->content, buf );
	}

	xmlFreeDoc( doc );

	// project file version checking
	// add a version checking to avoid people loading later versions of the project file and bitching
	int ver = IntForKey( g_qeglobals.d_project_entity, "version" );
	if ( ver > PROJECT_VERSION ) {
		char strMsg[1024];
		sprintf( strMsg, _( "This is a version %d project file. This build only supports <=%d project files.\n"
						 "Please choose another project file or upgrade your version of Radiant." ), ver, PROJECT_VERSION );
		gtk_MessageBox( g_pParentWnd->m_pWidget, strMsg, _( "Can't load project file" ), MB_ICONERROR | MB_OK );
		// set the project file to nothing so we are sure we'll ask next time?
		g_PrefsDlg.m_strLastProject = "";
		g_PrefsDlg.SavePrefs();
		return false;
	}

	// set here some default project settings you need
	if ( strlen( ValueForKey( g_qeglobals.d_project_entity, "brush_primit" ) ) == 0 ) {
		SetKeyValue( g_qeglobals.d_project_entity, "brush_primit", "0" );
	}

	g_qeglobals.m_bBrushPrimitMode = IntForKey( g_qeglobals.d_project_entity, "brush_primit" );

	g_qeglobals.m_strHomeMaps = g_qeglobals.m_strHomeGame;
	const char* str = ValueForKey( g_qeglobals.d_project_entity, "gamename" );
	if ( str[0] == '\0' ) {
		str = g_pGameDescription->mBaseGame.GetBuffer();
	}
	g_qeglobals.m_strHomeMaps += str;
	g_qeglobals.m_strHomeMaps += G_DIR_SEPARATOR;

	// don't forget to create the dirs
	Q_mkdir( g_qeglobals.m_strHomeGame.GetBuffer(), 0775 );
	Q_mkdir( g_qeglobals.m_strHomeMaps.GetBuffer(), 0775 );

	// usefull for the log file and debuggin fucked up configurations from users:
	// output the basic information of the .qe4 project file
	// SPoG
	// all these paths should be unix format, with a trailing slash at the end
	// if not.. to debug, check that the project file paths are set up correctly
	Sys_Printf( "basepath    : %s\n", ValueForKey( g_qeglobals.d_project_entity, "basepath" ) );
	Sys_Printf( "entitypath  : %s\n", ValueForKey( g_qeglobals.d_project_entity, "entitypath" ) );


	// check whether user_project key exists..
	// if not, save the current project under a new name
	if ( ValueForKey( g_qeglobals.d_project_entity, "user_project" )[0] == '\0' ) {
		Sys_Printf( "Loaded a template project file\n" );

		// create the user_project key
		SetKeyValue( g_qeglobals.d_project_entity, "user_project", "1" );

		if ( IntForKey( g_qeglobals.d_project_entity, "version" ) != PROJECT_VERSION ) {
			char strMsg[2048];
			sprintf( strMsg,
					_( "The template project '%s' has version %d. The editor binary is configured for version %d.\n"
					 "This indicates a problem in your setup.\n"
					 "I will keep going with this project till you fix this" ),
					 projectfile, IntForKey( g_qeglobals.d_project_entity, "version" ), PROJECT_VERSION );
			gtk_MessageBox( g_pParentWnd->m_pWidget, strMsg, _( "Can't load project file" ), MB_ICONERROR | MB_OK );
		}

		// create the writable project file path
		strcpy( buf, g_qeglobals.m_strHomeGame.GetBuffer() );
		strcat( buf, g_pGameDescription->mBaseGame.GetBuffer() );
		strcat( buf, G_DIR_SEPARATOR_S "scripts" G_DIR_SEPARATOR_S );
		// while the filename is already in use, increment the number we add to the end
		int counter = 0;
		char pUser[PATH_MAX];
		while ( 1 )
		{
			sprintf( pUser, "%suser%d." PROJECT_FILETYPE, buf, counter );
			counter++;
			if ( access( pUser, R_OK ) != 0 ) {
				// this is the one
				strcpy( buf, pUser );
				break;
			}
		}
		// saving project will cause a save prefs
		g_PrefsDlg.m_strLastProject = buf;
		g_PrefsDlg.m_nLastProjectVer = IntForKey( g_qeglobals.d_project_entity, "version" );
		QE_SaveProject( buf );
	}
	else
	{
		// update preferences::LastProject with path of this successfully-loaded project
		// save preferences
		Sys_Printf( "Setting current project in prefs to \"%s\"\n", g_PrefsDlg.m_strLastProject.GetBuffer() );
		g_PrefsDlg.m_strLastProject = projectfile;
		g_PrefsDlg.SavePrefs();
	}

	return true;
}

/*
   ===========
   QE_SaveProject
   TTimo: whenever QE_SaveProject is called, prefs are updated and saved with the path to the project
   ===========
 */
qboolean QE_SaveProject( const char* filename ){
	Sys_Printf( "Save project file '%s'\n", filename );

	xmlNodePtr node;
	xmlDocPtr doc = xmlNewDoc( (xmlChar *)"1.0" );
	// create DTD node
	xmlCreateIntSubset( doc, (xmlChar *)"project", NULL, (xmlChar *)"project.dtd" );
	// create project node
	doc->children->next = xmlNewDocNode( doc, NULL, (xmlChar *)"project", NULL );

	for ( epair_t* epair = g_qeglobals.d_project_entity->epairs; epair != NULL; epair = epair->next )
	{
		node = xmlNewChild( doc->children->next, NULL, (xmlChar *)"key", NULL );
		xmlSetProp( node, (xmlChar*)"name", (xmlChar*)epair->key );
		xmlSetProp( node, (xmlChar*)"value", (xmlChar*)epair->value );
	}

	CreateDirectoryPath( filename );
	if ( xmlSaveFormatFile( filename, doc, 1 ) != -1 ) {
		xmlFreeDoc( doc );
		Sys_Printf( "Setting current project in prefs to \"%s\"\n", filename );
		g_PrefsDlg.m_strLastProject = filename;
		g_PrefsDlg.SavePrefs();
		return TRUE;
	}
	else
	{
		xmlFreeDoc( doc );
		Sys_FPrintf( SYS_ERR, "failed to save project file: \"%s\"\n", filename );
		return FALSE;
	}
}



/*
   ===========
   QE_KeyDown
   ===========
 */
#define SPEED_MOVE  32
#define SPEED_TURN  22.5


/*
   ===============
   ConnectEntities

   Sets target / targetname on the two entities selected
   from the first selected to the secon
   ===============
 */
void ConnectEntities( void ){
	entity_t    *e1, *e2;
	const char      *target;
	char        *newtarg = NULL;

	if ( g_qeglobals.d_select_count != 2 ) {
		Sys_Status( "Must have two brushes selected", 0 );
		Sys_Beep();
		return;
	}

	e1 = g_qeglobals.d_select_order[0]->owner;
	e2 = g_qeglobals.d_select_order[1]->owner;

	if ( e1 == world_entity || e2 == world_entity ) {
		Sys_Status( "Can't connect to the world", 0 );
		Sys_Beep();
		return;
	}

	if ( e1 == e2 ) {
		Sys_Status( "Brushes are from same entity", 0 );
		Sys_Beep();
		return;
	}

	target = ValueForKey( e1, "target" );
	if ( target && target[0] ) {
		newtarg = g_strdup( target );
	}
	else
	{
		target = ValueForKey( e2, "targetname" );
		if ( target && target[0] ) {
			newtarg = g_strdup( target );
		}
		else{
			Entity_Connect( e1, e2 );
		}
	}

	if ( newtarg != NULL ) {
		SetKeyValue( e1, "target", newtarg );
		SetKeyValue( e2, "targetname", newtarg );
		g_free( newtarg );
	}

	Sys_UpdateWindows( W_XY | W_CAMERA );

	Select_Deselect();
	Select_Brush( g_qeglobals.d_select_order[1] );
}

qboolean QE_SingleBrush( bool bQuiet ){
	if ( ( selected_brushes.next == &selected_brushes )
		 || ( selected_brushes.next->next != &selected_brushes ) ) {
		if ( !bQuiet ) {
			Sys_FPrintf( SYS_ERR, "ERROR: you must have a single brush selected\n" );
		}
		return false;
	}
	if ( selected_brushes.next->owner->eclass->fixedsize ) {
		if ( !bQuiet ) {
			Sys_FPrintf( SYS_ERR, "ERROR: you cannot manipulate fixed size entities\n" );
		}
		return false;
	}

	return true;
}

void QE_InitVFS( void ){
	// VFS initialization -----------------------
	// we will call vfsInitDirectory, giving the directories to look in (for files in pk3's and for standalone files)
	// we need to call in order, the mod ones first, then the base ones .. they will be searched in this order
	// *nix systems have a dual filesystem in ~/.q3a, which is searched first .. so we need to add that too
	Str directory,prefabs;

	Str basePakPath = g_strAppPath.GetBuffer();
	basePakPath += "base";
	vfsInitDirectory( basePakPath.GetBuffer() );

	// TTimo: let's leave this to HL mode for now
	if ( g_pGameDescription->mGameFile == "hl.game" ) {
		// Hydra: we search the "gametools" path first so that we can provide editor
		// specific pk3's wads and misc files for use by the editor.
		// the relevant map compiler tools will NOT use this directory, so this helps
		// to ensure that editor files are not used/required in release versions of maps
		// it also helps keep your editor files all in once place, with the editor modules,
		// plugins, scripts and config files.
		// it also helps when testing maps, as you'll know your files won't/can't be used
		// by the game engine itself.

		// <gametools>
		directory = g_pGameDescription->mGameToolsPath;
		vfsInitDirectory( directory.GetBuffer() );
	}

	// NOTE TTimo about the mymkdir calls .. this is a bit dirty, but a safe thing on *nix

	// if we have a mod dir
	if ( *ValueForKey( g_qeglobals.d_project_entity, "gamename" ) != '\0' ) {

		// ~/.<gameprefix>/<fs_game>
		if ( g_qeglobals.m_strHomeGame.GetLength() ) {
			directory = g_qeglobals.m_strHomeGame.GetBuffer();
			Q_mkdir( directory.GetBuffer(), 0775 );
			directory += ValueForKey( g_qeglobals.d_project_entity, "gamename" );
			Q_mkdir( directory.GetBuffer(), 0775 );
			vfsInitDirectory( directory.GetBuffer() );
			AddSlash( directory );
			prefabs = directory;
			// also create the maps dir, it will be used as prompt for load/save
			directory += "maps";
			Q_mkdir( directory, 0775 );
			// and the prefabs dir
			prefabs += "prefabs";
			Q_mkdir( prefabs, 0775 );
		}

		// <fs_basepath>/<fs_game>
		directory = g_pGameDescription->mEnginePath;
		directory += ValueForKey( g_qeglobals.d_project_entity, "gamename" );
		Q_mkdir( directory.GetBuffer(), 0775 );
		vfsInitDirectory( directory.GetBuffer() );
		AddSlash( directory );
		prefabs = directory;
		// also create the maps dir, it will be used as prompt for load/save
		directory += "maps";
		Q_mkdir( directory.GetBuffer(), 0775 );
		// and the prefabs dir
		prefabs += "prefabs";
		Q_mkdir( prefabs, 0775 );
	}

	// ~/.<gameprefix>/<fs_main>
	if ( g_qeglobals.m_strHomeGame.GetLength() ) {
		directory = g_qeglobals.m_strHomeGame.GetBuffer();
		directory += g_pGameDescription->mBaseGame;
		vfsInitDirectory( directory.GetBuffer() );
	}

	// <fs_basepath>/<fs_main>
	directory = g_pGameDescription->mEnginePath;
	directory += g_pGameDescription->mBaseGame;
	vfsInitDirectory( directory.GetBuffer() );
}

void QE_Init( void ){
	/*
	** initialize variables
	*/
	g_qeglobals.d_gridsize = 8;
	g_qeglobals.d_showgrid = true;

	QE_InitVFS();

	Eclass_Init();
	FillClassList();    // list in entity window
	Map_Init();

	GSList *texdirs = NULL;
	FillTextureList( &texdirs );
	FillTextureMenu( texdirs );
	ClearGSList( texdirs );
	FillBSPMenu();

	/*
	** other stuff
	*/
	Z_Init();
}

void WINAPI QE_ConvertDOSToUnixName( char *dst, const char *src ){
	while ( *src )
	{
		if ( *src == '\\' ) {
			*dst = '/';
		}
		else{
			*dst = *src;
		}
		dst++; src++;
	}
	*dst = 0;
}

int g_numbrushes, g_numentities;

void QE_CountBrushesAndUpdateStatusBar( void ){
	static int s_lastbrushcount, s_lastentitycount;
	static qboolean s_didonce;

	//entity_t   *e;
	brush_t    *b, *next;

	g_numbrushes = 0;
	g_numentities = 0;

	if ( active_brushes.next != NULL ) {
		for ( b = active_brushes.next ; b != NULL && b != &active_brushes ; b = next )
		{
			next = b->next;
			if ( b->brush_faces ) {
				if ( !b->owner->eclass->fixedsize ) {
					g_numbrushes++;
				}
				else{
					g_numentities++;
				}
			}
		}
	}
/*
    if ( entities.next != NULL )
    {
        for ( e = entities.next ; e != &entities && g_numentities != MAX_MAP_ENTITIES ; e = e->next)
        {
            g_numentities++;
        }
    }
 */
	if ( ( ( g_numbrushes != s_lastbrushcount ) || ( g_numentities != s_lastentitycount ) ) || ( !s_didonce ) ) {
		Sys_UpdateStatusBar();

		s_lastbrushcount = g_numbrushes;
		s_lastentitycount = g_numentities;
		s_didonce = true;
	}
}

char com_token[1024];
qboolean com_eof;

/*
   ================
   I_FloatTime
   ================
 */
double I_FloatTime( void ){
	time_t t;

	time( &t );

	return t;
#if 0
// more precise, less portable
	struct timeval tp;
	struct timezone tzp;
	static int secbase;

	gettimeofday( &tp, &tzp );

	if ( !secbase ) {
		secbase = tp.tv_sec;
		return tp.tv_usec / 1000000.0;
	}

	return ( tp.tv_sec - secbase ) + tp.tv_usec / 1000000.0;
#endif
}


/*
   ==============
   COM_Parse

   Parse a token out of a string
   ==============
 */
char *COM_Parse( char *data ){
	int c;
	int len;

	len = 0;
	com_token[0] = 0;

	if ( !data ) {
		return NULL;
	}

// skip whitespace
skipwhite:
	while ( ( c = *data ) <= ' ' )
	{
		if ( c == 0 ) {
			com_eof = true;
			return NULL;            // end of file;
		}
		data++;
	}

// skip // comments
	if ( c == '/' && data[1] == '/' ) {
		while ( *data && *data != '\n' )
			data++;
		goto skipwhite;
	}


// handle quoted strings specially
	if ( c == '\"' ) {
		data++;
		do
		{
			c = *data++;
			if ( c == '\"' ) {
				com_token[len] = 0;
				return data;
			}
			com_token[len] = c;
			len++;
		} while ( 1 );
	}

// parse single characters
	if ( c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ':' ) {
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return data + 1;
	}

// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;
		if ( c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ':' ) {
			break;
		}
	} while ( c > 32 );

	com_token[len] = 0;
	return data;
}

char* Get_COM_Token(){
	return com_token;
}

/*
   =============================================================================

                        MISC FUNCTIONS

   =============================================================================
 */


int argc;
char    *argv[MAX_NUM_ARGVS];

/*
   ============
   ParseCommandLine
   ============
 */
void ParseCommandLine( char *lpCmdLine ){
	argc = 1;
	argv[0] = const_cast<char*>("programname");

	while ( *lpCmdLine && ( argc < MAX_NUM_ARGVS ) )
	{
		while ( *lpCmdLine && ( ( *lpCmdLine <= 32 ) || ( *lpCmdLine > 126 ) ) )
			lpCmdLine++;

		if ( *lpCmdLine ) {
			argv[argc] = lpCmdLine;
			argc++;

			while ( *lpCmdLine && ( ( *lpCmdLine > 32 ) && ( *lpCmdLine <= 126 ) ) )
				lpCmdLine++;

			if ( *lpCmdLine ) {
				*lpCmdLine = 0;
				lpCmdLine++;
			}

		}
	}
}



/*
   =================
   CheckParm

   Checks for the given parameter in the program's command line arguments
   Returns the argument number (1 to argc-1) or 0 if not present
   =================
 */
int CheckParm( const char *check ){
	int i;

	for ( i = 1; i < argc; i++ )
	{
		if ( stricmp( check, argv[i] ) ) {
			return i;
		}
	}

	return 0;
}




/*
   ==============
   ParseNum / ParseHex
   ==============
 */
int ParseHex( const char *hex ){
	const char    *str;
	int num;

	num = 0;
	str = hex;

	while ( *str )
	{
		num <<= 4;
		if ( *str >= '0' && *str <= '9' ) {
			num += *str - '0';
		}
		else if ( *str >= 'a' && *str <= 'f' ) {
			num += 10 + *str - 'a';
		}
		else if ( *str >= 'A' && *str <= 'F' ) {
			num += 10 + *str - 'A';
		}
		else{
			Error( "Bad hex number: %s",hex );
		}
		str++;
	}

	return num;
}


int ParseNum( const char *str ){
	if ( str[0] == '$' ) {
		return ParseHex( str + 1 );
	}
	if ( str[0] == '0' && str[1] == 'x' ) {
		return ParseHex( str + 2 );
	}
	return atol( str );
}

// BSP frontend plugin
// global flag for BSP frontend plugin is g_qeglobals.bBSPFrontendPlugin
_QERPlugBSPFrontendTable g_BSPFrontendTable;

// =============================================================================
// Sys_ functions

bool Sys_AltDown(){
#ifdef _WIN32
	return ( GetKeyState( VK_MENU ) & 0x8000 ) != 0;
#endif

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	char keys[32];
	int x;

	XQueryKeymap( gdk_x11_get_default_xdisplay(), keys );

	x = XKeysymToKeycode( gdk_x11_get_default_xdisplay(), XK_Alt_L );
	if ( keys[x / 8] & ( 1 << ( x % 8 ) ) ) {
		return true;
	}

	x = XKeysymToKeycode( gdk_x11_get_default_xdisplay(), XK_Alt_R );
	if ( keys[x / 8] & ( 1 << ( x % 8 ) ) ) {
		return true;
	}

	// For Apple, let users use their Command keys since Alt + X11 is hosed
	x = XKeysymToKeycode( gdk_x11_get_default_xdisplay(), XK_Meta_L );
	if ( keys[x / 8] & ( 1 << ( x % 8 ) ) ) {
		return true;
	}

	x = XKeysymToKeycode( gdk_x11_get_default_xdisplay(), XK_Meta_R );
	if ( keys[x / 8] & ( 1 << ( x % 8 ) ) ) {
		return true;
	}

	return false;
#endif
}

bool Sys_ShiftDown(){
#ifdef _WIN32
	return ( GetKeyState( VK_SHIFT ) & 0x8000 ) != 0;
#endif

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	char keys[32];
	int x;

	XQueryKeymap( gdk_x11_get_default_xdisplay(), keys );

	x = XKeysymToKeycode( gdk_x11_get_default_xdisplay(), XK_Shift_L );
	if ( keys[x / 8] & ( 1 << ( x % 8 ) ) ) {
		return true;
	}

	x = XKeysymToKeycode( gdk_x11_get_default_xdisplay(), XK_Shift_R );
	if ( keys[x / 8] & ( 1 << ( x % 8 ) ) ) {
		return true;
	}

	return false;
#endif
}

void Sys_MarkMapModified( void ){
	char title[PATH_MAX];

	if ( modified != 1 ) {
		modified = true; // mark the map as changed
		sprintf( title, "%s *", currentmap );

		QE_ConvertDOSToUnixName( title, title );
		Sys_SetTitle( title );
	}
}

void Sys_SetTitle( const char *text ){
	gtk_window_set_title( GTK_WINDOW( g_qeglobals_gui.d_main_window ), text );
}

bool g_bWaitCursor = false;

void WINAPI Sys_BeginWait( void ){
	GdkWindow *window;
	GdkDisplay *display;
	GdkCursor *cursor;

	window = gtk_widget_get_window( g_pParentWnd->m_pWidget );
	display = gdk_window_get_display( window );
	cursor = gdk_cursor_new_for_display( display, GDK_WATCH );
	gdk_window_set_cursor( window, cursor );
#if GTK_CHECK_VERSION( 3, 0, 0 )
	g_object_unref( cursor );
#else
	gdk_cursor_unref( cursor );
#endif

	g_bWaitCursor = true;
}

void WINAPI Sys_EndWait( void ){
	GdkWindow *window;
	GdkDisplay *display;
	GdkCursor *cursor;

	window = gtk_widget_get_window( g_pParentWnd->m_pWidget );
	display = gdk_window_get_display( window );
	cursor = gdk_cursor_new_for_display( display, GDK_LEFT_PTR );

	gdk_window_set_cursor( window, cursor );
#if GTK_CHECK_VERSION( 3, 0, 0 )
	g_object_unref( cursor );
#else
	gdk_cursor_unref( cursor );
#endif

	g_bWaitCursor = false;
}

void Sys_GetCursorPos( int *x, int *y ){
	gdk_display_get_pointer( gdk_display_get_default(), 0, x, y, 0 );
}

void Sys_SetCursorPos( int x, int y ){
	GdkDisplay *display = gdk_display_get_default();
	gdk_display_warp_pointer( display, gdk_display_get_default_screen( display ), x, y );
}

void Sys_Beep( void ){
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	gdk_beep();
#else
	MessageBeep( MB_ICONASTERISK );
#endif
}

double Sys_DoubleTime( void ){
	return clock() / 1000.0;
}

/*
   ===============================================================

   STATUS WINDOW

   ===============================================================
 */

void Sys_UpdateStatusBar( void ){
	extern int g_numbrushes, g_numentities;

	char numbrushbuffer[100] = "";

	sprintf( numbrushbuffer, "Brushes: %d Entities: %d", g_numbrushes, g_numentities );
	g_pParentWnd->SetStatusText( 2, numbrushbuffer );
	//Sys_Status( numbrushbuffer, 2 );
}

void Sys_Status( const char *psz, int part ){
	g_pParentWnd->SetStatusText( part, psz );
}

// =============================================================================
// MRU

#define MRU_MAX 4
static GtkWidget *MRU_items[MRU_MAX];
static int MRU_used;
typedef char MRU_filename_t[PATH_MAX];
MRU_filename_t MRU_filenames[MRU_MAX];

static char* MRU_GetText( int index ){
	return MRU_filenames[index];
}

void buffer_write_escaped_mnemonic( char* buffer, const char* string ){
	while ( *string != '\0' )
	{
		if ( *string == '_' ) {
			*buffer++ = '_';
		}

		*buffer++ = *string++;
	}
	*buffer = '\0';
}

static void MRU_SetText( int index, const char *filename ){
	strcpy( MRU_filenames[index], filename );

	char mnemonic[PATH_MAX * 2 + 4];
	mnemonic[0] = '_';
	sprintf( mnemonic + 1, "%d", index + 1 );
	mnemonic[2] = '-';
	mnemonic[3] = ' ';
	buffer_write_escaped_mnemonic( mnemonic + 4, filename );
	gtk_menu_item_set_label( GTK_MENU_ITEM( MRU_items[index] ), mnemonic );
}

void MRU_Load(){
	int i = g_PrefsDlg.m_nMRUCount;

	if ( i > 4 ) {
		i = 4; //FIXME: make this a define

	}
	for (; i > 0; i-- )
		MRU_AddFile( g_PrefsDlg.m_strMRUFiles[i - 1].GetBuffer() );
}

void MRU_Save(){
	g_PrefsDlg.m_nMRUCount = MRU_used;

	for ( int i = 0; i < MRU_used; i++ )
		g_PrefsDlg.m_strMRUFiles[i] = MRU_GetText( i );
}

void MRU_AddWidget( GtkWidget *widget, int pos ){
	if ( pos < MRU_MAX ) {
		MRU_items[pos] = widget;
	}
}

void MRU_AddFile( const char *str ){
	int i;
	char* text;

	// check if file is already in our list
	for ( i = 0; i < MRU_used; i++ )
	{
		text = MRU_GetText( i );

		if ( strcmp( text, str ) == 0 ) {
			// reorder menu
			for (; i > 0; i-- )
				MRU_SetText( i, MRU_GetText( i - 1 ) );

			MRU_SetText( 0, str );

			return;
		}
	}

	if ( MRU_used < MRU_MAX ) {
		MRU_used++;
	}

	// move items down
	for ( i = MRU_used - 1; i > 0; i-- )
		MRU_SetText( i, MRU_GetText( i - 1 ) );

	MRU_SetText( 0, str );
	gtk_widget_set_sensitive( MRU_items[0], TRUE );
	gtk_widget_show( MRU_items[MRU_used - 1] );
}

void MRU_Activate( int index ){
	char *text = MRU_GetText( index );

	if ( access( text, R_OK ) == 0 ) {
		text = strdup( text );
		MRU_AddFile( text );
		Map_LoadFile( text );
		free( text );
	}
	else
	{
		MRU_used--;

		for ( int i = index; i < MRU_used; i++ )
			MRU_SetText( i, MRU_GetText( i + 1 ) );

		if ( MRU_used == 0 ) {
			gtk_menu_item_set_label( GTK_MENU_ITEM( MRU_items[0] ), _( "Recent Files" ) );
			gtk_widget_set_sensitive( MRU_items[0], FALSE );
		}
		else
		{
			gtk_widget_hide( MRU_items[MRU_used] );
		}
	}
}

/*
   ======================================================================

   FILE DIALOGS

   ======================================================================
 */

qboolean ConfirmModified() {
	if( !modified )
		return TRUE;

	int saveChoice = gtk_MessageBoxNew( g_pParentWnd->m_pWidget, 
						"The current map has changed since it was last saved.\n"
						"Would you like to save before continuing?", "Radiant", 
						MB_YESNOCANCEL | MB_ICONQUESTION );

	switch( saveChoice ) {
	case IDYES: {
		g_pParentWnd->OnFileSave();
		break;
	}
	case IDNO: {
		return TRUE;
	}
	case IDCANCEL:
	default: {
		return FALSE;
	}
	}

	return TRUE;
}

void ProjectDialog(){
	const char *filename;
	char buffer[NAME_MAX];

	/*
	 * Obtain the system directory name and
	 * store it in buffer.
	 */

	strcpy( buffer, g_qeglobals.m_strHomeGame.GetBuffer() );
	strcat( buffer, g_pGameDescription->mBaseGame.GetBuffer() );
	strcat( buffer, "/scripts/" );

	// Display the Open dialog box
	filename = file_dialog( NULL, TRUE, _( "Open File" ), buffer, "project" );

	if ( filename == NULL ) {
		return; // canceled

	}
	// Open the file.
	// NOTE: QE_LoadProject takes care of saving prefs with new path to the project file
	if ( !QE_LoadProject( filename ) ) {
		Sys_Printf( "Failed to load project from file: %s\n", filename );
	}
	else{
		// FIXME TTimo QE_Init is probably broken if you don't call it during startup right now ..
		QE_Init();
	}
}

/*
   =======================================================

   Menu modifications

   =======================================================
 */

/*
   ==================
   FillBSPMenu

   ==================
 */
char *bsp_commands[256];

void FillBSPMenu(){
	GtkWidget *item, *menu; // menu points to a GtkMenu (not an item)
	epair_t *ep;
	GList *children, *lst;
	int i;

	menu = GTK_WIDGET( g_object_get_data( G_OBJECT( g_qeglobals_gui.d_main_window ), "menu_bsp" ) );

	children = gtk_container_get_children( GTK_CONTAINER( menu ) );
	if( children ) {
		for ( lst = children; lst != NULL; lst = g_list_next( lst ) )
		{
			gtk_container_remove( GTK_CONTAINER( menu ), GTK_WIDGET( lst->data ) );
		}
		g_list_free( children );
	}

	if ( g_PrefsDlg.m_bDetachableMenus ) {
		item = gtk_tearoff_menu_item_new();
		gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );
		gtk_widget_set_sensitive( item, TRUE );
		gtk_widget_show( item );
	}

	if ( g_qeglobals.bBSPFrontendPlugin ) {
		CString str = g_BSPFrontendTable.m_pfnGetBSPMenu();
		char cTemp[1024];
		strcpy( cTemp, str );
		char* token = strtok( cTemp, ",;" );
		if ( token && *token == ' ' ) {
			while ( *token == ' ' )
				token++;
		}
		i = 0;

		// first token is menu name
		children = gtk_container_get_children( GTK_CONTAINER( menu ) );
		if( children ) {
			if( g_list_first( children ) ) {
				gtk_label_set_text( GTK_LABEL( g_list_first( children )->data ), token );
			}
			g_list_free( children );
		}
		token = strtok( NULL, ",;" );
		while ( token != NULL )
		{
			g_BSPFrontendCommands = g_slist_append( g_BSPFrontendCommands, g_strdup( token ) );
			item = gtk_menu_item_new_with_label( token );
			gtk_widget_show( item );
			gtk_container_add( GTK_CONTAINER( menu ), item );
			g_signal_connect( G_OBJECT( item ), "activate",
								G_CALLBACK( HandleCommand ), GINT_TO_POINTER( CMD_BSPCOMMAND + i ) );
			token = strtok( NULL, ",;" );
			i++;
		}
	}
	else
	{
		i = 0;
		for ( ep = g_qeglobals.d_project_entity->epairs; ep; ep = ep->next )
		{
			if ( strncmp( ep->key, "bsp_", 4 ) == 0 ) {
				bsp_commands[i] = ep->key;
				item = gtk_menu_item_new_with_label( ep->key + 4 );
				gtk_widget_show( item );
				gtk_container_add( GTK_CONTAINER( menu ), item );
				g_signal_connect( G_OBJECT( item ), "activate",
									G_CALLBACK( HandleCommand ), GINT_TO_POINTER( CMD_BSPCOMMAND + i ) );
				i++;
			}
		}
	}
}

//==============================================

void AddSlash( CString& strPath ){
	if ( strPath.GetLength() > 0 ) {
		if ( !g_str_has_suffix( strPath.GetBuffer(), G_DIR_SEPARATOR_S ) ) {
			strPath += G_DIR_SEPARATOR_S;
		}
	}
}

bool ExtractPath_and_Filename( const char* pPath, CString& strPath, CString& strFilename ){
	CString strPathName;
	strPathName = pPath;
	int nSlash = strPathName.ReverseFind( '\\' );
	if ( nSlash == -1 ) {
		// TTimo: try forward slash, some are using forward
		nSlash = strPathName.ReverseFind( '/' );
	}
	if ( nSlash >= 0 ) {
		strPath = strPathName.Left( nSlash + 1 );
		strFilename = strPathName.Right( strPathName.GetLength() - nSlash - 1 );
	}
	// TTimo: try forward slash, some are using forward
	else{
		strFilename = pPath;
	}
	return true;
}

//===========================================

//++timo FIXME: no longer used .. remove!
char *TranslateString( char *buf ){
	static char buf2[32768];
	int i, l;
	char          *out;

	l = strlen( buf );
	out = buf2;
	for ( i = 0 ; i < l ; i++ )
	{
		if ( buf[i] == '\n' ) {
			*out++ = '\r';
			*out++ = '\n';
		}
		else{
			*out++ = buf[i];
		}
	}
	*out++ = 0;

	return buf2;
}

// called whenever we need to open/close/check the console log file
void Sys_LogFile( void ){
	if ( g_PrefsDlg.mGamesDialog.m_bLogConsole && !g_qeglobals.hLogFile ) {
		// settings say we should be logging and we don't have a log file .. so create it
		// open a file to log the console (if user prefs say so)
		// the file handle is g_qeglobals.hLogFile
		// the log file is erased
		Str name;
		name = g_strTempPath;
		name += "radiant.log";
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
		g_qeglobals.hLogFile = open( name.GetBuffer(), O_TRUNC | O_CREAT | O_WRONLY, S_IREAD | S_IWRITE );
#endif
#ifdef _WIN32
		g_qeglobals.hLogFile = _open( name.GetBuffer(), _O_TRUNC | _O_CREAT | _O_WRONLY, _S_IREAD | _S_IWRITE );
#endif
		if ( g_qeglobals.hLogFile ) {
			Sys_Printf( "Started logging to %s\n", name.GetBuffer() );
			time_t localtime;
			time( &localtime );
			Sys_Printf( "Today is: %s", ctime( &localtime ) );
			Sys_Printf( "This is radiant '" RADIANT_VERSION "' compiled " __DATE__ "\n" );
			Sys_Printf( RADIANT_ABOUTMSG "\n" );
		}
		else{
			gtk_MessageBox( NULL, _( "Failed to create log file, check write permissions in Radiant directory.\n" ),
							_( "Console logging" ), MB_OK );
		}
	}
	else if ( !g_PrefsDlg.mGamesDialog.m_bLogConsole && g_qeglobals.hLogFile ) {
		// settings say we should not be logging but still we have an active logfile .. close it
		time_t localtime;
		time( &localtime );
		Sys_Printf( "Closing log file at %s\n", ctime( &localtime ) );
		#ifdef _WIN32
		_close( g_qeglobals.hLogFile );
		#endif
		#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
		close( g_qeglobals.hLogFile );
		#endif
		g_qeglobals.hLogFile = 0;
	}
}

void Sys_ClearPrintf( void ){
	GtkTextBuffer* buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( g_qeglobals_gui.d_edit ) );
	gtk_text_buffer_set_text( buffer, "", -1 );
}

// used to be around 32000, that should be way enough already
#define BUFFER_SIZE 4096

extern "C" void Sys_FPrintf_VA( int level, const char *text, va_list args ) {
	char buf[BUFFER_SIZE];

	buf[0] = 0;
	vsnprintf( buf, BUFFER_SIZE, text, args );
	buf[BUFFER_SIZE - 1] = 0;
	const unsigned int length = strlen( buf );

	if ( g_qeglobals.hLogFile ) {
#ifdef _WIN32
		_write( g_qeglobals.hLogFile, buf, length );
		_commit( g_qeglobals.hLogFile );
#endif
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
		write( g_qeglobals.hLogFile, buf, length );
#endif
	}

	if ( level != SYS_NOCON ) {
		// TTimo: FIXME: killed the console to avoid GDI leak fuckup
		if ( g_qeglobals_gui.d_edit != NULL ) {
			GtkTextBuffer* buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( g_qeglobals_gui.d_edit ) );

			GtkTextIter iter;
			gtk_text_buffer_get_end_iter( buffer, &iter );

			static GtkTextMark* end = gtk_text_buffer_create_mark( buffer, "end", &iter, FALSE );

			const GdkColor yellow = { 0, 0xb0ff, 0xb0ff, 0x0000 };
			const GdkColor red = { 0, 0xffff, 0x0000, 0x0000 };
			const GdkColor black = { 0, 0x0000, 0x0000, 0x0000 };

			static GtkTextTag* error_tag = gtk_text_buffer_create_tag( buffer, "red_foreground", "foreground-gdk", &red, NULL );
			static GtkTextTag* warning_tag = gtk_text_buffer_create_tag( buffer, "yellow_foreground", "foreground-gdk", &yellow, NULL );
			static GtkTextTag* standard_tag = gtk_text_buffer_create_tag( buffer, "black_foreground", "foreground-gdk", &black, NULL );
			GtkTextTag* tag;
			switch ( level )
			{
			case SYS_WRN:
				tag = warning_tag;
				break;
			case SYS_ERR:
				tag = error_tag;
				break;
			case SYS_STD:
			case SYS_VRB:
			default:
				tag = standard_tag;
				break;
			}
			gtk_text_buffer_insert_with_tags( buffer, &iter, buf, length, tag, (char *) NULL );

			gtk_text_view_scroll_mark_onscreen( GTK_TEXT_VIEW( g_qeglobals_gui.d_edit ), end );

			// update console widget immediately if we're doing something time-consuming
			if ( !g_bScreenUpdates && gtk_widget_get_realized( g_qeglobals_gui.d_edit ) ) {
				gtk_grab_add( g_qeglobals_gui.d_edit );

				while ( gtk_events_pending() )
					gtk_main_iteration();

				gtk_grab_remove( g_qeglobals_gui.d_edit );
			}
		}
	}
}

// NOTE: this is the handler sent to synapse
// must match PFN_SYN_PRINTF_VA
extern "C" void Sys_Printf_VA( const char *text, va_list args ){
	Sys_FPrintf_VA( SYS_STD, text, args );
}

extern "C" void Sys_Printf( const char *text, ... ) {
	va_list args;

	va_start( args, text );
	Sys_FPrintf_VA( SYS_STD, text, args );
	va_end( args );
}

extern "C" void Sys_FPrintf( int level, const char *text, ... ){
	va_list args;

	va_start( args, text );
	Sys_FPrintf_VA( level, text, args );
	va_end( args );
}
