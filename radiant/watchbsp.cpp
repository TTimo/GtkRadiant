/*
   Copyright (c) 2001, Loki software, inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of Loki software nor the names of its contributors may be used
   to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// monitoring window for running BSP processes (and possibly various other stuff)

#include "stdafx.h"
#include "watchbsp.h"
#include "feedback.h"

#ifdef _WIN32
#include <winsock2.h>
#endif

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#include <sys/time.h>
#define SOCKET_ERROR -1
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

#include <assert.h>
#include <glib/gi18n.h>

// Static functions for the SAX callbacks -------------------------------------------------------

// utility for saxStartElement below
static void abortStream( message_info_t *data ){
	g_pParentWnd->GetWatchBSP()->Reset();
	// tell there has been an error
	if ( g_pParentWnd->GetWatchBSP()->HasBSPPlugin() ) {
		g_BSPFrontendTable.m_pfnEndListen( 2 );
	}
	// yeah this doesn't look good.. but it's needed so that everything will be ignored until the stream goes out
	data->ignore_depth = -1;
	data->recurse++;
}

#include "stream_version.h"

static void saxStartElement( message_info_t *data, const xmlChar *name, const xmlChar **attrs ){
	if ( data->ignore_depth == 0 ) {
		if ( data->bGeometry ) {
			// we have a handler
			data->pGeometry->saxStartElement( data, name, attrs );
		}
		else
		{
			if ( strcmp( (char *)name, "q3map_feedback" ) == 0 ) {
				// check the correct version
				// old q3map don't send a version attribute
				// the ones we support .. send Q3MAP_STREAM_VERSION
				if ( !attrs[0] || !attrs[1] || ( strcmp( (char*)attrs[0],"version" ) != 0 ) ) {
					Sys_FPrintf( SYS_ERR, "No stream version given in the feedback stream, this is an old q3map version.\n"
										  "Please turn off monitored compiling if you still wish to use this q3map executable\n" );
					abortStream( data );
					return;
				}
				else if ( strcmp( (char*)attrs[1],Q3MAP_STREAM_VERSION ) != 0 ) {
					Sys_FPrintf( SYS_ERR,
								 "This version of Radiant reads version %s debug streams, I got an incoming connection with version %s\n"
								 "Please make sure your versions of Radiant and q3map are matching.\n", Q3MAP_STREAM_VERSION, (char*)attrs[1] );
					abortStream( data );
					return;
				}
			}
			// we don't treat locally
			else if ( strcmp( (char *)name, "message" ) == 0 ) {
				data->msg_level = atoi( (char *)attrs[1] );
			}
			else if ( strcmp( (char *)name, "polyline" ) == 0 ) {
				// polyline has a particular status .. right now we only use it for leakfile ..
				data->bGeometry = true;
				data->pGeometry = &g_pointfile;
				data->pGeometry->saxStartElement( data, name, attrs );
			}
			else if ( strcmp( (char *)name, "select" ) == 0 ) {
				CSelectMsg *pSelect = new CSelectMsg();
				data->bGeometry = true;
				data->pGeometry = pSelect;
				data->pGeometry->saxStartElement( data, name, attrs );
			}
			else if ( strcmp( (char *)name, "pointmsg" ) == 0 ) {
				CPointMsg *pPoint = new CPointMsg();
				data->bGeometry = true;
				data->pGeometry = pPoint;
				data->pGeometry->saxStartElement( data, name, attrs );
			}
			else if ( strcmp( (char *)name, "windingmsg" ) == 0 ) {
				CWindingMsg *pWinding = new CWindingMsg();
				data->bGeometry = true;
				data->pGeometry = pWinding;
				data->pGeometry->saxStartElement( data, name, attrs );
			}
			else
			{
				Sys_FPrintf( SYS_WRN, "WARNING: ignoring unrecognized node in XML stream (%s)\n", name );
				// we don't recognize this node, jump over it
				// (NOTE: the ignore mechanism is a bit screwed, only works when starting an ignore at the highest level)
				data->ignore_depth = data->recurse;
			}
		}
	}
	data->recurse++;
}

static void saxEndElement( message_info_t *data, const xmlChar *name ) {
	data->recurse--;
	// we are out of an ignored chunk
	if ( data->recurse == data->ignore_depth ) {
		data->ignore_depth = 0;
		return;
	}
	if ( data->bGeometry ) {
		data->pGeometry->saxEndElement( data, name );
		// we add the object to the debug window
		if ( !data->bGeometry ) {
			g_DbgDlg.Push( data->pGeometry );
		}
	}
	if ( data->recurse == data->stop_depth ) {
#ifdef _DEBUG
		Sys_FPrintf( SYS_ERR, "ERROR: Received error msg .. shutting down..\n" );
#endif
		// tell there has been an error
		if ( g_pParentWnd->GetWatchBSP()->HasBSPPlugin() ) {
			g_BSPFrontendTable.m_pfnEndListen( 2 );
		}
		return;
	}
}

static void saxCharacters( message_info_t *data, const xmlChar *ch, int len ){
	if ( data->bGeometry ) {
		data->pGeometry->saxCharacters( data, ch, len );
	}
	else
	{
		if ( data->ignore_depth != 0 ) {
			return;
		}
		// output the message using the level
		char buf[1024];
		memcpy( buf, ch, len );
		buf[len] = '\0';
		Sys_FPrintf( data->msg_level, "%s", buf );
		// if this message has error level flag, we mark the depth to stop the compilation when we get out
		// we don't set the msg level if we don't stop on leak
		if ( data->msg_level == 3 ) {
			data->stop_depth = data->recurse - 1;
		}
	}
}

static void saxComment( void *ctx, const xmlChar *msg ){
	Sys_Printf( "XML comment: %s\n", msg );
}

static void saxWarning( void *ctx, const char *msg, ... ){
	char saxMsgBuffer[4096];
	va_list args;

	va_start( args, msg );
	vsprintf( saxMsgBuffer, msg, args );
	va_end( args );
	Sys_FPrintf( SYS_WRN, "XML warning: %s\n", saxMsgBuffer );
}

static void saxError( void *ctx, const char *msg, ... ){
	char saxMsgBuffer[4096];
	va_list args;

	va_start( args, msg );
	vsprintf( saxMsgBuffer, msg, args );
	va_end( args );
	Sys_FPrintf( SYS_ERR, "XML error: %s\n", saxMsgBuffer );
}

static void saxFatal( void *ctx, const char *msg, ... ){
	char buffer[4096];

	va_list args;

	va_start( args, msg );
	vsprintf( buffer, msg, args );
	va_end( args );
	Sys_FPrintf( SYS_ERR, "XML fatal error: %s\n", buffer );
}

static xmlSAXHandler saxParser = {
	0, /* internalSubset */
	0, /* isStandalone */
	0, /* hasInternalSubset */
	0, /* hasExternalSubset */
	0, /* resolveEntity */
	0, /* getEntity */
	0, /* entityDecl */
	0, /* notationDecl */
	0, /* attributeDecl */
	0, /* elementDecl */
	0, /* unparsedEntityDecl */
	0, /* setDocumentLocator */
	0, /* startDocument */
	0, /* endDocument */
	(startElementSAXFunc)saxStartElement, /* startElement */
	(endElementSAXFunc)saxEndElement, /* endElement */
	0, /* reference */
	(charactersSAXFunc)saxCharacters, /* characters */
	0, /* ignorableWhitespace */
	0, /* processingInstruction */
	(commentSAXFunc)saxComment, /* comment */
	(warningSAXFunc)saxWarning, /* warning */
	(errorSAXFunc)saxError, /* error */
	(fatalErrorSAXFunc)saxFatal, /* fatalError */
};

// ------------------------------------------------------------------------------------------------

CWatchBSP::~CWatchBSP(){

	Reset();

	if ( m_pCmd ) {
		g_ptr_array_free( m_pCmd, true );
		m_pCmd = NULL;
	}

	if ( m_sBSPName ) {
		g_free( m_sBSPName );
		m_sBSPName = NULL;
	}

	Net_Shutdown();
}

void CWatchBSP::RunQuake() {

	// build the command line
	Str cmd;

	cmd = g_pGameDescription->mExecutablesPath.GetBuffer();
	// this is game dependant
	if ( !strcmp( ValueForKey( g_qeglobals.d_project_entity, "gamemode" ),"mp" ) ) {
		// MP
		cmd += g_pGameDescription->mMultiplayerEngine.GetBuffer();
	}
	else
	{
		// SP
		cmd += g_pGameDescription->mEngine.GetBuffer();
	}
#ifdef _WIN32
	// NOTE: we are using unix pathnames and CreateProcess doesn't like / in the program path
	// FIXME: This isn't true anymore, doesn't it?
	FindReplace( cmd, "/", "\\" );
#endif
	Str cmdline;
	if ( g_pGameDescription->idTech2 ) {
		cmdline = "+exec radiant.cfg +map ";
		cmdline += m_sBSPName;
	}
	else
	// NOTE: idTech3 specific - there used to be some logic depending on engine breed here
	{
		cmdline = "+set sv_pure 0 ";
		// TTimo: a check for vm_* but that's all fine
		//cmdline = "+set sv_pure 0 +set vm_ui 0 +set vm_cgame 0 +set vm_game 0 ";
		if ( *ValueForKey( g_qeglobals.d_project_entity, "gamename" ) != '\0' ) {
			cmdline += "+set fs_game ";
			cmdline += ValueForKey( g_qeglobals.d_project_entity, "gamename" );
			cmdline += " ";
		}
		//!\todo Read the start-map args from a config file.
		if ( g_pGameDescription->mGameFile == "wolf.game" ) {
			if ( !strcmp( ValueForKey( g_qeglobals.d_project_entity, "gamemode" ),"mp" ) ) {
				// MP
				cmdline += "+devmap ";
				cmdline += m_sBSPName;
			}
			else
			{
				// SP
				cmdline += "+set nextmap \"spdevmap ";
				cmdline += m_sBSPName;
				cmdline += "\"";
			}
		}
		else
		{
			cmdline += "+devmap ";
			cmdline += m_sBSPName;
		}
	}

	Sys_Printf( "%s %s\n", cmd.GetBuffer(), cmdline.GetBuffer() );

	// execute now
	if ( !Q_Exec( cmd.GetBuffer(), (char *)cmdline.GetBuffer(), g_pGameDescription->mEnginePath.GetBuffer(), false ) ) {
		CString msg;
		msg = "Failed to execute the following command: ";
		msg += cmd; msg += cmdline;
		Sys_Printf( msg );
		gtk_MessageBox( g_pParentWnd->m_pWidget,  msg, _( "BSP monitoring" ), MB_OK | MB_ICONERROR );
	}
}

void CWatchBSP::Reset(){
	if ( m_pInSocket ) {
		Net_Disconnect( m_pInSocket );
		m_pInSocket = NULL;
	}
	if ( m_pListenSocket ) {
		Net_Disconnect( m_pListenSocket );
		m_pListenSocket = NULL;
	}
	if ( m_xmlInputBuffer ) {
		xmlFreeParserInputBuffer( m_xmlInputBuffer );
		m_xmlInputBuffer = NULL;
	}
	if ( m_xmlParserCtxt ) {
		xmlFreeParserCtxt( m_xmlParserCtxt );
		m_xmlParserCtxt = NULL;
	}

	m_eState = EIdle;
}

bool CWatchBSP::SetupListening(){
#ifdef _DEBUG
	if ( m_pListenSocket ) {
		Sys_FPrintf( SYS_ERR, "ERROR: m_pListenSocket != NULL in CWatchBSP::SetupListening\n" );
		return false;
	}
#endif
	Sys_Printf( "Setting up\n" );
	if ( !Net_Setup() ) {
		return false;
	}

	m_pListenSocket = Net_ListenSocket( 39000 );
	if ( m_pListenSocket == NULL ) {
		return false;
	}

	Sys_Printf( "Listening...\n" );
	return true;
}

void CWatchBSP::DoEBeginStep() {

	if ( !SetupListening() ) {
		CString msg;
		msg = _( "Failed to get a listening socket on port 39000.\nTry running with BSP monitoring disabled if you can't fix this.\n" );
		Sys_Printf( msg );
		gtk_MessageBox( g_pParentWnd->m_pWidget, msg, _( "BSP monitoring" ), MB_OK | MB_ICONERROR );
		Reset();
		return;
	}

	// re-initialise the debug window
	if ( m_iCurrentStep == 0 ) {
		g_DbgDlg.Init();
	}

	// set the timer for timeouts and step cancellation
	g_timer_reset( m_pTimer );
	g_timer_start( m_pTimer );

	if ( !m_bBSPPlugin ) {
		Sys_Printf( "=== running BSP command ===\n%s\n", g_ptr_array_index( m_pCmd, m_iCurrentStep ) );

		if ( !Q_Exec( NULL, (char *) g_ptr_array_index( m_pCmd, m_iCurrentStep ), NULL, true ) ) {
			CString msg;
			msg = _( "Failed to execute the following command: " );
			msg += (char *) g_ptr_array_index( m_pCmd, m_iCurrentStep );
			msg += _( "\nCheck that the file exists and that you don't run out of system resources.\n" );
			Sys_Printf( msg );
			gtk_MessageBox( g_pParentWnd->m_pWidget,  msg, _( "BSP monitoring" ), MB_OK | MB_ICONERROR );
			Reset();
			return;
		}

	}

	m_eState = EBeginStep;
}

void CWatchBSP::RoutineProcessing(){
	// used for select()
#ifdef _WIN32
	TIMEVAL tout = { 0, 0 };
#endif
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	timeval tout;
	tout.tv_sec = 0;
	tout.tv_usec = 0;
#endif

	switch ( m_eState )
	{
	case EBeginStep:
		// timeout: if we don't get an incoming connection fast enough, go back to idle
		if ( g_timer_elapsed( m_pTimer, NULL ) > g_PrefsDlg.m_iTimeout ) {
			gtk_MessageBox( g_pParentWnd->m_pWidget, _( "The connection timed out, assuming the BSP process failed\nMake sure you are using a networked version of Q3Map?\nOtherwise you need to disable BSP Monitoring in prefs." ), _( "BSP process monitoring" ), MB_OK );
			Reset();
			if ( m_bBSPPlugin ) {
				// status == 1 : didn't get the connection
				g_BSPFrontendTable.m_pfnEndListen( 1 );
			}
			break;
		}
#ifdef _DEBUG
		// some debug checks
		if ( !m_pListenSocket ) {
			Sys_FPrintf( SYS_ERR, "ERROR: m_pListenSocket == NULL in CWatchBSP::RoutineProcessing EBeginStep state\n" );
			Reset();
			break;
		}
#endif
		// we are not connected yet, accept any incoming connection
		m_pInSocket = Net_Accept( m_pListenSocket );
		if ( m_pInSocket ) {
			Sys_Printf( "Connected.\n" );
			// prepare the message info struct for diving in
			memset( &m_message_info, 0, sizeof( message_info_s ) );
			// a dumb flag to make sure we init the push parser context when first getting a msg
			m_eState = EWatching;
		}
		break;

	case EWatching:
#ifdef _DEBUG
		// some debug checks
		if ( !m_pInSocket ) {
			Sys_FPrintf( SYS_ERR, "ERROR: m_pInSocket == NULL in CWatchBSP::RoutineProcessing EWatching state\n" );
			Reset();
			break;
		}
#endif
		// select() will identify if the socket needs an update
		// if the socket is identified that means there's either a message or the connection has been closed/reset/terminated
		fd_set readfds;
		int ret;
		FD_ZERO( &readfds );
		FD_SET( ( (unsigned int)m_pInSocket->socket ), &readfds );
		// from select man page:
		// n is the highest-numbered descriptor in any of the three sets, plus 1
		// (no use on windows)
		ret = select( m_pInSocket->socket + 1, &readfds, NULL, NULL, &tout );
		if ( ret == SOCKET_ERROR ) {
			Sys_FPrintf( SYS_WRN, "WARNING: SOCKET_ERROR in CWatchBSP::RoutineProcessing\n" );
			Sys_Printf( "Terminating the connection.\n" );
			Reset();
			break;
		}
		if ( ret == 1 ) {
			// the socket has been identified, there's something (message or disconnection)
			// see if there's anything in input
			ret = Net_Receive( m_pInSocket, &msg );
			if ( ret > 0 ) {
				//        unsigned int size = msg.size; //++timo just a check
				g_strlcpy( m_xmlBuf, NMSG_ReadString( &msg ), sizeof( m_xmlBuf) );
				if ( m_xmlParserCtxt == NULL ) {
					m_xmlParserCtxt = xmlCreatePushParserCtxt( &saxParser, &m_message_info, m_xmlBuf, strlen( m_xmlBuf ), NULL );
					if ( m_xmlParserCtxt == NULL ) {
						Sys_FPrintf( SYS_ERR, "Failed to create the XML parser (incoming stream began with: %s)\n", m_xmlBuf );
						Reset();
						break;
					}
				}
				else
				{
					xmlParseChunk( m_xmlParserCtxt, m_xmlBuf, strlen( m_xmlBuf ), 0 );
				}
			}
			else
			{
				// error or connection closed/reset
				// NOTE: if we get an error down the XML stream we don't reach here
				Net_Disconnect( m_pInSocket );
				m_pInSocket = NULL;
				Sys_Printf( "Connection closed.\n" );
				if ( m_bBSPPlugin ) {
					// let the BSP plugin know that the job is done
					g_BSPFrontendTable.m_pfnEndListen( 0 );
				}

				Reset();

				// move to next step or finish
				m_iCurrentStep++;
				if ( m_iCurrentStep < m_pCmd->len ) {
					DoEBeginStep();
					break;
				}

				// launch the engine .. OMG
				if ( g_PrefsDlg.m_bRunQuake ) {
					// do we enter sleep mode before?
					if ( g_PrefsDlg.m_bDoSleep ) {
						Sys_Printf( "Going into sleep mode..\n" );
						g_pParentWnd->OnSleep();
					}
					Sys_Printf( "Running engine...\n" );
					RunQuake();
				}
			}
		}
		break;
	default:
		break;
	}
}

void CWatchBSP::DoMonitoringLoop( GPtrArray *pCmd, char *sBSPName ){
	guint i;

	if ( m_eState != EIdle ) {
		Sys_Printf( "WatchBSP got a monitoring request while not idling...\n" );
		// prompt the user, should we cancel the current process and go ahead?
		if ( gtk_MessageBox( g_pParentWnd->m_pWidget,  _( "I am already monitoring a BSP process.\nDo you want me to override and start a new compilation?" ),
							 _( "BSP process monitoring" ), MB_YESNO ) == IDNO ) {
			return;
		}
	}

	Reset();

	if ( m_pCmd ) {
		g_ptr_array_free( m_pCmd, true );
		m_pCmd = NULL;
	}

	if ( m_sBSPName ) {
		g_free( m_sBSPName );
		m_sBSPName = NULL;
	}

        // glib 2.30
//	m_pCmd = g_ptr_array_new_full( pCmd->len, g_free );

        m_pCmd = g_ptr_array_sized_new( pCmd->len );
        g_ptr_array_set_free_func( m_pCmd, g_free );

	for ( i = 0; i < pCmd->len; i++ ) {
		g_ptr_array_add( m_pCmd, g_strdup( (char *) pCmd->pdata[i] ) );
	}

	m_iCurrentStep = 0;

	m_sBSPName = g_strdup(sBSPName);

	DoEBeginStep();
}

void CWatchBSP::ExternalListen(){
	m_bBSPPlugin = true;
	DoEBeginStep();
}

// the part of the watchbsp interface we export to plugins
// NOTE: in the long run, the whole watchbsp.cpp interface needs to go out and be handled at the BSP plugin level
// for now we provide something really basic and limited, the essential is to have something that works fine and fast (for 1.1 final)
void WINAPI QERApp_Listen(){
	// open the listening socket
	g_pParentWnd->GetWatchBSP()->ExternalListen();
}
