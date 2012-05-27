/*
This code is based on source provided under the terms of the Id Software 
LIMITED USE SOFTWARE LICENSE AGREEMENT, a copy of which is included with the
GtkRadiant sources (see LICENSE_ID). If you did not receive a copy of 
LICENSE_ID, please contact Id Software immediately at info@idsoftware.com.

All changes and additions to the original source which have been developed by
other contributors (see CONTRIBUTORS) are provided under the terms of the
license the contributors choose (see LICENSE), to the extent permitted by the
LICENSE_ID. If you did not receive a copy of the contributor license,
please contact the GtkRadiant maintainers at info@gtkradiant.com immediately.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// start of shared cmdlib stuff
// 

#include "cmdlib.h"

#include <string.h>
#include <stdio.h>

#include "string/string.h"
#include "os/path.h"
#include "container/array.h"

#ifdef WIN32
  #include <windows.h>
#endif
#if defined (__linux__) || defined (__APPLE__)
  #include <unistd.h>
#endif


#if defined (__linux__) || defined (__APPLE__)
bool Q_Exec(const char *cmd, char *cmdline, const char *, bool)
{
  char fullcmd[2048];
  char *pCmd;
#ifdef _DEBUG
  printf("Q_Exec damnit\n");
#endif
  switch (fork())
  {
  case -1:
    return true;
    break;
  case 0:
    // always concat the command on linux
    if (cmd)
    {
      strcpy(fullcmd, cmd);
    }
    else
      fullcmd[0] = '\0';
    if (cmdline)
    {
      strcat(fullcmd, " ");
      strcat(fullcmd, cmdline);
    }
    pCmd = fullcmd;
    while (*pCmd == ' ')
      pCmd++;
#ifdef _DEBUG
    printf("Running system...\n");
    printf("Command: %s\n", pCmd);
#endif
    system( pCmd );
#ifdef _DEBUG
    printf ("system() returned\n");
#endif
    _exit (0);
    break;
  }
  return true;
}
#endif

#ifdef WIN32
// NOTE TTimo windows is VERY nitpicky about the syntax in CreateProcess
bool Q_Exec(const char *cmd, char *cmdline, const char *execdir, bool bCreateConsole)
{
  PROCESS_INFORMATION ProcessInformation;
  STARTUPINFO startupinfo = {0};
  DWORD dwCreationFlags;
  GetStartupInfo (&startupinfo);
  if (bCreateConsole)
    dwCreationFlags = CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS;
  else
    dwCreationFlags = DETACHED_PROCESS | NORMAL_PRIORITY_CLASS;
  const char *pCmd;
  char *pCmdline;
  pCmd = cmd;
  if (pCmd)
  {
    while (*pCmd == ' ')
      pCmd++;
  }
  pCmdline = cmdline;
  if (pCmdline)
  {
    while (*pCmdline == ' ')
      pCmdline++;
  }

  if (CreateProcess(
                    pCmd,
                    pCmdline,
                    NULL,
                    NULL,
                    FALSE,
                    dwCreationFlags,
                    NULL,
                    execdir,
                    &startupinfo,
                    &ProcessInformation
                    ))
    return true;
  return false;
}
#endif

