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

#include "watchbsp.h"

#include <algorithm>
#include <gtk/gtkmain.h>

#include "cmdlib.h"
#include "convert.h"
#include "string/string.h"
#include "stream/stringstream.h"

#include "gtkutil/messagebox.h"
#include "xmlstuff.h"
#include "console.h"
#include "preferences.h"
#include "points.h"
#include "feedback.h"
#include "mainframe.h"
#include "sockets.h"

void message_flush(message_info_t* self)
{
  Sys_Print(self->msg_level, self->m_buffer, self->m_length);
  self->m_length = 0;
}

void message_print(message_info_t* self, const char* characters, std::size_t length)
{
  const char* end = characters + length;
  while(characters != end)
  {
    std::size_t space = message_info_t::bufsize - 1 - self->m_length;
    if(space == 0)
    {
      message_flush(self);
    }
    else
    {
      std::size_t size = std::min(space, std::size_t(end - characters));
      memcpy(self->m_buffer + self->m_length, characters, size);
      self->m_length += size;
      characters += size;
    }
  }
}


#include <glib/gtimer.h>
#include <glib/garray.h>
#include "xmlstuff.h"

class CWatchBSP
{
private:
  // a flag we have set to true when using an external BSP plugin
  // the resulting code with that is a bit dirty, cleaner solution would be to seperate the succession of commands from the listening loop
  // (in two seperate classes probably)
  bool m_bBSPPlugin;

  // EIdle: we are not listening
  //   DoMonitoringLoop will change state to EBeginStep
  // EBeginStep: the socket is up for listening, we are expecting incoming connection
  //   incoming connection will change state to EWatching
  // EWatching: we have a connection, monitor it
  //   connection closed will see if we start a new step (EBeginStep) or launch Quake3 and end (EIdle)
  enum EWatchBSPState { EIdle, EBeginStep, EWatching } m_eState;
  socket_t *m_pListenSocket;
  socket_t *m_pInSocket;
  netmessage_t msg;
  GPtrArray *m_pCmd;
  // used to timeout EBeginStep
  GTimer    *m_pTimer;
  std::size_t m_iCurrentStep;
  // name of the map so we can run the engine
  char    *m_sBSPName;
  // buffer we use in push mode to receive data directly from the network
  xmlParserInputBufferPtr m_xmlInputBuffer;
  xmlParserInputPtr m_xmlInput;
  xmlParserCtxtPtr m_xmlParserCtxt;
  // call this to switch the set listening mode
  bool SetupListening();
  // start a new EBeginStep
  void DoEBeginStep();
  // the xml and sax parser state
  char m_xmlBuf[MAX_NETMESSAGE];
  bool m_bNeedCtxtInit;
  message_info_t m_message_info;

public:
  CWatchBSP()
  {
    m_pCmd = 0;
    m_bBSPPlugin = false;
    m_pListenSocket = NULL;
    m_pInSocket = NULL;
    m_eState = EIdle;
    m_pTimer = g_timer_new();
    m_sBSPName = NULL;
    m_xmlInputBuffer = NULL;
    m_bNeedCtxtInit = true;
  }
  virtual ~CWatchBSP()
  {
    EndMonitoringLoop();
    Net_Shutdown();

    g_timer_destroy(m_pTimer);
  }

  bool HasBSPPlugin() const
    { return m_bBSPPlugin; }

  // called regularly to keep listening
  void RoutineProcessing();
  // start a monitoring loop with the following steps
  void DoMonitoringLoop( GPtrArray *pCmd, const char *sBSPName );
  void EndMonitoringLoop()
  {
    Reset();
    if (m_sBSPName)
    {
      string_release(m_sBSPName, string_length(m_sBSPName));
      m_sBSPName = 0;
    }
    if(m_pCmd)
    {
      g_ptr_array_free(m_pCmd, TRUE);
      m_pCmd = 0;
    }
  }
  // close everything - may be called from the outside to abort the process
  void Reset();
  // start a listening loop for an external process, possibly a BSP plugin
  void ExternalListen();
};

CWatchBSP* g_pWatchBSP;

  // watch the BSP process through network connections
  // true: trigger the BSP steps one by one and monitor them through the network
  // false: create a BAT / .sh file and execute it. don't bother monitoring it.
bool g_WatchBSP_Enabled = true;
  // do we stop the compilation process if we come accross a leak?
bool g_WatchBSP_LeakStop = true;
bool g_WatchBSP_RunQuake = false;
  // store prefs setting for automatic sleep mode activation
bool g_WatchBSP_DoSleep = true;
  // timeout when beginning a step (in seconds)
  // if we don't get a connection quick enough we assume something failed and go back to idling
int g_WatchBSP_Timeout = 10;


void Build_constructPreferences(PreferencesPage& page)
{
  GtkWidget* monitorbsp = page.appendCheckBox("", "Enable Build Process Monitoring", g_WatchBSP_Enabled);
  GtkWidget* leakstop = page.appendCheckBox("", "Stop Compilation on Leak", g_WatchBSP_LeakStop);
  GtkWidget* runengine = page.appendCheckBox("", "Run Engine After Compile", g_WatchBSP_RunQuake);
  GtkWidget* sleep = page.appendCheckBox("", "Sleep When Running the Engine", g_WatchBSP_DoSleep);
  Widget_connectToggleDependency(leakstop, monitorbsp);
  Widget_connectToggleDependency(runengine, monitorbsp);
  Widget_connectToggleDependency(sleep, runengine);
}
void Build_constructPage(PreferenceGroup& group)
{
  PreferencesPage page(group.createPage("Build", "Build Preferences"));
  Build_constructPreferences(page);
}
void Build_registerPreferencesPage()
{
  PreferencesDialog_addSettingsPage(FreeCaller1<PreferenceGroup&, Build_constructPage>());
}

#include "preferencesystem.h"
#include "stringio.h"

void BuildMonitor_Construct()
{
  g_pWatchBSP = new CWatchBSP();

  g_WatchBSP_Enabled = !string_equal(g_pGameDescription->getKeyValue("no_bsp_monitor"), "1");

  GlobalPreferenceSystem().registerPreference("WatchBSP", BoolImportStringCaller(g_WatchBSP_Enabled), BoolExportStringCaller(g_WatchBSP_Enabled));
  GlobalPreferenceSystem().registerPreference("RunQuake2Run", BoolImportStringCaller(g_WatchBSP_RunQuake), BoolExportStringCaller(g_WatchBSP_RunQuake));
  GlobalPreferenceSystem().registerPreference("LeakStop", BoolImportStringCaller(g_WatchBSP_LeakStop), BoolExportStringCaller(g_WatchBSP_LeakStop));
  GlobalPreferenceSystem().registerPreference("SleepMode", BoolImportStringCaller(g_WatchBSP_DoSleep), BoolExportStringCaller(g_WatchBSP_DoSleep));

  Build_registerPreferencesPage();
}

void BuildMonitor_Destroy()
{
  delete g_pWatchBSP;
}

CWatchBSP *GetWatchBSP()
{
  return g_pWatchBSP;
}

void BuildMonitor_Run(GPtrArray* commands, const char* mapName)
{
  GetWatchBSP()->DoMonitoringLoop(commands, mapName);
}


// Static functions for the SAX callbacks -------------------------------------------------------

// utility for saxStartElement below
static void abortStream(message_info_t *data)
{
  GetWatchBSP()->EndMonitoringLoop();
  // tell there has been an error
#if 0
  if (GetWatchBSP()->HasBSPPlugin())
    g_BSPFrontendTable.m_pfnEndListen(2);
#endif
  // yeah this doesn't look good.. but it's needed so that everything will be ignored until the stream goes out
  data->ignore_depth = -1;
  data->recurse++;
}

#include "stream_version.h"

static void saxStartElement(message_info_t *data, const xmlChar *name, const xmlChar **attrs) 
{
#if 0
  globalOutputStream() << "<" << name;
  if(attrs != 0)
  {
    for(const xmlChar** p = attrs; *p != 0; p += 2)
    {
      globalOutputStream() << " " << p[0] << "=" << makeQuoted(p[1]);
    }
  }
  globalOutputStream() << ">\n";
#endif

  if (data->ignore_depth == 0)
  {
    if(data->pGeometry != 0)
      // we have a handler
    {
      data->pGeometry->saxStartElement (data, name, attrs);
    }
    else
    {
      if (strcmp(reinterpret_cast<const char*>(name), "q3map_feedback") == 0)
      {
        // check the correct version
        // old q3map don't send a version attribute
        // the ones we support .. send Q3MAP_STREAM_VERSION
        if (!attrs[0] || !attrs[1] || (strcmp(reinterpret_cast<const char*>(attrs[0]), "version") != 0))
        {
          message_flush(data);
          globalErrorStream() << "No stream version given in the feedback stream, this is an old q3map version.\n"
                      "Please turn off monitored compiling if you still wish to use this q3map executable\n";
          abortStream(data);
          return;
        }
        else if (strcmp(reinterpret_cast<const char*>(attrs[1]), Q3MAP_STREAM_VERSION) != 0)
        {
          message_flush(data);
          globalErrorStream() <<
            "This version of Radiant reads version " Q3MAP_STREAM_VERSION " debug streams, I got an incoming connection with version " << reinterpret_cast<const char*>(attrs[1]) << "\n"
            "Please make sure your versions of Radiant and q3map are matching.\n";
          abortStream(data);
          return;
        }
      }
      // we don't treat locally
      else if (strcmp(reinterpret_cast<const char*>(name), "message") == 0)
      {
        int msg_level = atoi(reinterpret_cast<const char*>(attrs[1]));
        if(msg_level != data->msg_level)
        {
          message_flush(data);
          data->msg_level = msg_level;
        }
      }
      else if (strcmp(reinterpret_cast<const char*>(name), "polyline") == 0) 
      // polyline has a particular status .. right now we only use it for leakfile ..
      {
        data->geometry_depth = data->recurse;
        data->pGeometry = &g_pointfile;
        data->pGeometry->saxStartElement (data, name, attrs);  
      }
      else if (strcmp(reinterpret_cast<const char*>(name), "select") == 0)
      {
        CSelectMsg *pSelect = new CSelectMsg();
        data->geometry_depth = data->recurse;
        data->pGeometry = pSelect;
        data->pGeometry->saxStartElement (data, name, attrs);
      }
      else if (strcmp(reinterpret_cast<const char*>(name), "pointmsg") == 0)
      {
        CPointMsg *pPoint = new CPointMsg();
        data->geometry_depth = data->recurse;
        data->pGeometry = pPoint;
        data->pGeometry->saxStartElement (data, name, attrs);
      }
      else if (strcmp(reinterpret_cast<const char*>(name), "windingmsg") == 0)
      {
        CWindingMsg *pWinding = new CWindingMsg();
        data->geometry_depth = data->recurse;
        data->pGeometry = pWinding;
        data->pGeometry->saxStartElement (data, name, attrs);
      }
      else
      {
        globalErrorStream() << "Warning: ignoring unrecognized node in XML stream (" << reinterpret_cast<const char*>(name) << ")\n";
        // we don't recognize this node, jump over it
        // (NOTE: the ignore mechanism is a bit screwed, only works when starting an ignore at the highest level)
        data->ignore_depth = data->recurse;
      }
    }
  }
  data->recurse++;
}

static void saxEndElement(message_info_t *data, const xmlChar *name) 
{
#if 0
  globalOutputStream() << "<" << name << "/>\n";
#endif

  data->recurse--;
  // we are out of an ignored chunk
  if(data->recurse == data->ignore_depth)
  {
    data->ignore_depth = 0;
    return;
  }
  if(data->pGeometry != 0)
  {
    data->pGeometry->saxEndElement (data, name);
    // we add the object to the debug window
    if(data->geometry_depth == data->recurse)
    {
      g_DbgDlg.Push(data->pGeometry);
      data->pGeometry = 0;
    }
  }
  if (data->recurse == data->stop_depth)
  {
    message_flush(data);
#ifdef _DEBUG
    globalOutputStream() << "Received error msg .. shutting down..\n";
#endif
    GetWatchBSP()->EndMonitoringLoop();
    // tell there has been an error
#if 0
    if (GetWatchBSP()->HasBSPPlugin())
      g_BSPFrontendTable.m_pfnEndListen(2);
#endif
    return;
  }
}

class MessageOutputStream : public TextOutputStream
{
  message_info_t* m_data;
public:
  MessageOutputStream(message_info_t* data) : m_data(data)
  {
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    if(m_data->pGeometry != 0)
    {
      m_data->pGeometry->saxCharacters(m_data, reinterpret_cast<const xmlChar*>(buffer), int(length));
    }
    else
    {
      if (m_data->ignore_depth == 0)
      {
        // output the message using the level
        message_print(m_data, buffer, length);
        // if this message has error level flag, we mark the depth to stop the compilation when we get out
        // we don't set the msg level if we don't stop on leak
        if (m_data->msg_level == 3)
        {
          m_data->stop_depth = m_data->recurse-1;
        }
      }
    }

    return length;
  }
};

template<typename T>
inline MessageOutputStream& operator<<(MessageOutputStream& ostream, const T& t)
{
  return ostream_write(ostream, t);
}

static void saxCharacters(message_info_t *data, const xmlChar *ch, int len)
{
  MessageOutputStream ostream(data);
  ostream << ConvertUTF8ToLocale(StringRange(reinterpret_cast<const char*>(ch), reinterpret_cast<const char*>(ch + len)));
}

static void saxComment(void *ctx, const xmlChar *msg)
{
  globalOutputStream() << "XML comment: " << reinterpret_cast<const char*>(msg) << "\n";
}

static void saxWarning(void *ctx, const char *msg, ...)
{
  char saxMsgBuffer[4096];
  va_list args;
 
  va_start(args, msg);
  vsprintf (saxMsgBuffer, msg, args);
  va_end(args);
  globalOutputStream() << "XML warning: " << saxMsgBuffer << "\n";
}

static void saxError(void *ctx, const char *msg, ...)
{
  char saxMsgBuffer[4096];
  va_list args;
 
  va_start(args, msg);
  vsprintf (saxMsgBuffer, msg, args);
  va_end(args);
  globalErrorStream() << "XML error: " << saxMsgBuffer << "\n";
}

static void saxFatal(void *ctx, const char *msg, ...)
{
  char buffer[4096];
  
  va_list args;
 
  va_start(args, msg);
  vsprintf (buffer, msg, args);
  va_end(args);
  globalErrorStream() << "XML fatal error: " << buffer << "\n";
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
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

// ------------------------------------------------------------------------------------------------


guint s_routine_id;
static gint watchbsp_routine(gpointer data)
{
  reinterpret_cast<CWatchBSP*>(data)->RoutineProcessing();
  return TRUE;
}

void CWatchBSP::Reset()
{
  if (m_pInSocket)
  {
    Net_Disconnect(m_pInSocket);
    m_pInSocket = NULL;
  }
  if (m_pListenSocket)
  {
    Net_Disconnect(m_pListenSocket);
    m_pListenSocket = NULL;
  }
  if (m_xmlInputBuffer)
  {
    xmlFreeParserInputBuffer (m_xmlInputBuffer);
    m_xmlInputBuffer = NULL;
  }
  m_eState = EIdle;
  if (s_routine_id)
    gtk_timeout_remove(s_routine_id);
}

bool CWatchBSP::SetupListening()
{
#ifdef _DEBUG
  if (m_pListenSocket)
  {
    globalOutputStream() << "ERROR: m_pListenSocket != NULL in CWatchBSP::SetupListening\n";
    return false;
  }
#endif
  globalOutputStream() << "Setting up\n";
	Net_Setup();
	m_pListenSocket = Net_ListenSocket(39000);
  if (m_pListenSocket == NULL)
    return false;
  globalOutputStream() << "Listening...\n";
  return true;
}

void CWatchBSP::DoEBeginStep()
{
  Reset();
  if (SetupListening() == false)
  {
    const char* msg = "Failed to get a listening socket on port 39000.\nTry running with Build monitoring disabled if you can't fix this.\n";
    globalOutputStream() << msg;
    gtk_MessageBox(GTK_WIDGET(MainFrame_getWindow()), msg, "Build monitoring", eMB_OK, eMB_ICONERROR);
    return;
  }
  // set the timer for timeouts and step cancellation
  g_timer_reset( m_pTimer );
  g_timer_start( m_pTimer );

  if (!m_bBSPPlugin)
  {
    globalOutputStream() << "=== running build command ===\n"
      << static_cast<const char*>(g_ptr_array_index( m_pCmd, m_iCurrentStep )) << "\n";
    
    if (!Q_Exec(NULL, (char *)g_ptr_array_index( m_pCmd, m_iCurrentStep ), NULL, true ))
    {
      StringOutputStream msg(256);
      msg << "Failed to execute the following command: ";
      msg << reinterpret_cast<const char*>(g_ptr_array_index(m_pCmd, m_iCurrentStep));
      msg << "\nCheck that the file exists and that you don't run out of system resources.\n";
      globalOutputStream() << msg.c_str();
      gtk_MessageBox(GTK_WIDGET(MainFrame_getWindow()), msg.c_str(), "Build monitoring", eMB_OK, eMB_ICONERROR );
      return;
    }
    // re-initialise the debug window
    if (m_iCurrentStep == 0)
      g_DbgDlg.Init();
  }
  m_eState = EBeginStep;
  s_routine_id = gtk_timeout_add(25, watchbsp_routine, this);
}


#if defined(WIN32)
#define ENGINE_ATTRIBUTE "engine_win32"
#define MP_ENGINE_ATTRIBUTE "mp_engine_win32"
#elif defined(__linux__) || defined (__FreeBSD__)
#define ENGINE_ATTRIBUTE "engine_linux"
#define MP_ENGINE_ATTRIBUTE "mp_engine_linux"
#elif defined(__APPLE__)
#define ENGINE_ATTRIBUTE "engine_macos"
#define MP_ENGINE_ATTRIBUTE "mp_engine_macos"
#else
#error "unsupported platform"
#endif

class RunEngineConfiguration
{
public:
  const char* executable;
  const char* mp_executable;
  bool do_sp_mp;

  RunEngineConfiguration() :
    executable(g_pGameDescription->getRequiredKeyValue(ENGINE_ATTRIBUTE)),
    mp_executable(g_pGameDescription->getKeyValue(MP_ENGINE_ATTRIBUTE))
  {
    do_sp_mp = !string_empty(mp_executable);
  }
};

inline void GlobalGameDescription_string_write_mapparameter(StringOutputStream& string, const char* mapname)
{
  if(g_pGameDescription->mGameType == "q2"
    || g_pGameDescription->mGameType == "heretic2")
  {
    string << ". +exec radiant.cfg +map " << mapname;
  }
  else
  {
    string << "+set sv_pure 0 ";
    // TTimo: a check for vm_* but that's all fine
    //cmdline = "+set sv_pure 0 +set vm_ui 0 +set vm_cgame 0 +set vm_game 0 ";
    const char* fs_game = gamename_get();
    if (!string_equal(fs_game, basegame_get()))
    {
      string << "+set fs_game " << fs_game << " ";
    }
    if(g_pGameDescription->mGameType == "wolf"
      || g_pGameDescription->mGameType == "et")
    {
      if (string_equal(gamemode_get(), "mp"))
      {
        // MP
        string << "+devmap " << mapname;
      }
      else
      {
        // SP                
        string << "+set nextmap \"spdevmap " << mapname << "\"";
      }
    }
    else
    {
      string << "+devmap " << mapname;
    }
  }
}


void CWatchBSP::RoutineProcessing()
{
  switch (m_eState)
  {
  case EBeginStep:
    // timeout: if we don't get an incoming connection fast enough, go back to idle
    if ( g_timer_elapsed( m_pTimer, NULL ) > g_WatchBSP_Timeout )
    {
      gtk_MessageBox(GTK_WIDGET(MainFrame_getWindow()),  "The connection timed out, assuming the build process failed\nMake sure you are using a networked version of Q3Map?\nOtherwise you need to disable BSP Monitoring in prefs.", "BSP process monitoring", eMB_OK );
      EndMonitoringLoop();
#if 0
      if (m_bBSPPlugin)
      {
        // status == 1 : didn't get the connection
        g_BSPFrontendTable.m_pfnEndListen(1);
      }
#endif
      return;
    }
#ifdef _DEBUG
    // some debug checks
    if (!m_pListenSocket)
    {
      globalErrorStream() << "ERROR: m_pListenSocket == NULL in CWatchBSP::RoutineProcessing EBeginStep state\n";
      return;
    }
#endif
    // we are not connected yet, accept any incoming connection
    m_pInSocket = Net_Accept(m_pListenSocket);
    if (m_pInSocket)
    {
      globalOutputStream() << "Connected.\n";
      // prepare the message info struct for diving in
      memset (&m_message_info, 0, sizeof(message_info_t)); 
      // a dumb flag to make sure we init the push parser context when first getting a msg
      m_bNeedCtxtInit = true;
      m_eState = EWatching;
    }
    break;
  case EWatching:
    {
#ifdef _DEBUG
    // some debug checks
    if (!m_pInSocket)
    {
      globalErrorStream() << "ERROR: m_pInSocket == NULL in CWatchBSP::RoutineProcessing EWatching state\n";
      return;
    }
#endif

    int ret = Net_Wait(m_pInSocket, 0, 0);
    if (ret == -1)
    {
      globalOutputStream() << "WARNING: SOCKET_ERROR in CWatchBSP::RoutineProcessing\n";
      globalOutputStream() << "Terminating the connection.\n";
      EndMonitoringLoop();
      return;
    }

    if (ret == 1)
    {
      // the socket has been identified, there's something (message or disconnection)
      // see if there's anything in input
      ret = Net_Receive( m_pInSocket, &msg );
      if (ret > 0)
      {
        //        unsigned int size = msg.size; //++timo just a check
        strcpy (m_xmlBuf, NMSG_ReadString (&msg));
        if (m_bNeedCtxtInit)
        {
          m_xmlParserCtxt = NULL;
          m_xmlParserCtxt = xmlCreatePushParserCtxt (&saxParser, &m_message_info, m_xmlBuf, static_cast<int>(strlen(m_xmlBuf)), NULL);

          if (m_xmlParserCtxt == NULL)
          {
            globalErrorStream() << "Failed to create the XML parser (incoming stream began with: " << m_xmlBuf << ")\n";
            EndMonitoringLoop();
          }
          m_bNeedCtxtInit = false;
        }
        else
        {
          xmlParseChunk(m_xmlParserCtxt, m_xmlBuf, static_cast<int>(strlen(m_xmlBuf)), 0);
        }
      }
      else
      {
        message_flush(&m_message_info);
        // error or connection closed/reset
        // NOTE: if we get an error down the XML stream we don't reach here
        Net_Disconnect( m_pInSocket );
        m_pInSocket = NULL;
        globalOutputStream() << "Connection closed.\n";
#if 0
        if (m_bBSPPlugin)
        {
          EndMonitoringLoop();
          // let the BSP plugin know that the job is done
          g_BSPFrontendTable.m_pfnEndListen(0);
          return;
        }
#endif
        // move to next step or finish
        m_iCurrentStep++;
        if (m_iCurrentStep < m_pCmd->len )
        {
          DoEBeginStep();
        }
        else
        {
          // launch the engine .. OMG
          if (g_WatchBSP_RunQuake)
          {
#if 0
            // do we enter sleep mode before?
            if (g_WatchBSP_DoSleep)
            {
              globalOutputStream() << "Going into sleep mode..\n";
              g_pParentWnd->OnSleep();
            }
#endif
            globalOutputStream() << "Running engine...\n";
            StringOutputStream cmd(256);
            // build the command line
            cmd << EnginePath_get();
            // this is game dependant

            RunEngineConfiguration engineConfig;
           
            if(engineConfig.do_sp_mp)
            {
              if (string_equal(gamemode_get(), "mp"))
              {
                cmd << engineConfig.mp_executable;
              }
              else
              {
                cmd << engineConfig.executable;
              }
            }
            else
            {
              cmd << engineConfig.executable;
            }

            StringOutputStream cmdline;

            GlobalGameDescription_string_write_mapparameter(cmdline, m_sBSPName);

            globalOutputStream() << cmd.c_str() << " " << cmdline.c_str() << "\n";

            // execute now
            if (!Q_Exec(cmd.c_str(), (char *)cmdline.c_str(), EnginePath_get(), false))
            {
              StringOutputStream msg;
              msg << "Failed to execute the following command: " << cmd.c_str() << cmdline.c_str();
              globalOutputStream() << msg.c_str();
              gtk_MessageBox(GTK_WIDGET(MainFrame_getWindow()),  msg.c_str(), "Build monitoring", eMB_OK, eMB_ICONERROR );
            }
          }
          EndMonitoringLoop();
        }
      }
    }
    }
    break;
  default:
    break;
  }
}

GPtrArray* str_ptr_array_clone(GPtrArray* array)
{
  GPtrArray* cloned = g_ptr_array_sized_new(array->len);
  for(guint i = 0; i < array->len; ++i)
  {
    g_ptr_array_add(cloned, g_strdup((char*)g_ptr_array_index(array, i)));
  }
  return cloned;
}

void CWatchBSP::DoMonitoringLoop( GPtrArray *pCmd, const char *sBSPName )
{
  m_sBSPName = string_clone(sBSPName);
  if (m_eState != EIdle)
  {
    globalOutputStream() << "WatchBSP got a monitoring request while not idling...\n";
    // prompt the user, should we cancel the current process and go ahead?
    if (gtk_MessageBox(GTK_WIDGET(MainFrame_getWindow()),  "I am already monitoring a Build process.\nDo you want me to override and start a new compilation?", 
      "Build process monitoring", eMB_YESNO ) == eIDYES)
    {
      // disconnect and set EIdle state
      Reset();
    }
  }
  m_pCmd = str_ptr_array_clone(pCmd);
  m_iCurrentStep = 0;
  DoEBeginStep();
}

void CWatchBSP::ExternalListen()
{
  m_bBSPPlugin = true;
  DoEBeginStep();
}

// the part of the watchbsp interface we export to plugins
// NOTE: in the long run, the whole watchbsp.cpp interface needs to go out and be handled at the BSP plugin level
// for now we provide something really basic and limited, the essential is to have something that works fine and fast (for 1.1 final)
void QERApp_Listen()
{
  // open the listening socket
  GetWatchBSP()->ExternalListen();
}
