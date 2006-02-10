/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
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

#include "plugin.h"
#include "version.h"

/*! \file plugin.cpp
    \brief HydraToolz!

    HydraToolz by Dominic Clifton - Hydra (Hydra@Hydras-World.com)

    Overview
    ========


    Version History
    ===============

    v0.1 - 28/May/2002
      - Initial version.


    ToDo
    ====

    * Code it ? :)

*/

// =============================================================================
// Globals

_QERFuncTable_1 g_FuncTable;
_QERFileSystemTable g_FileSystemTable;
_QEREntityTable g_EntityTable;

// cast to GtkWidget*
void *g_pMainWnd;

// =============================================================================
// Ripped from TexTool.cpp

static void dialog_button_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *parent;
  int *loop, *ret;

  parent = gtk_widget_get_toplevel (widget);
  loop = (int*)gtk_object_get_data (GTK_OBJECT (parent), "loop");
  ret = (int*)gtk_object_get_data (GTK_OBJECT (parent), "ret");

  *loop = 0;
  *ret = (int)data;
}

static gint dialog_delete_callback (GtkWidget *widget, GdkEvent* event, gpointer data)
{
  int *loop;

  gtk_widget_hide (widget);
  loop = (int*)gtk_object_get_data (GTK_OBJECT (widget), "loop");
  *loop = 0;

  return TRUE;
}

int DoMessageBox (const char* lpText, const char* lpCaption, guint32 uType)
{
  GtkWidget *window, *w, *vbox, *hbox;
  int mode = (uType & MB_TYPEMASK), ret, loop = 1;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                      GTK_SIGNAL_FUNC (dialog_delete_callback), NULL);
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  gtk_window_set_title (GTK_WINDOW (window), lpCaption);
  gtk_container_border_width (GTK_CONTAINER (window), 10);
  gtk_object_set_data (GTK_OBJECT (window), "loop", &loop);
  gtk_object_set_data (GTK_OBJECT (window), "ret", &ret);
  gtk_widget_realize (window);

  vbox = gtk_vbox_new (FALSE, 10);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);

  w = gtk_label_new (lpText);
  gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
  gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
  gtk_widget_show (w);

  w = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
  gtk_widget_show (w);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
  gtk_widget_show (hbox);

  if (mode == MB_OK)
  {
    w = gtk_button_new_with_label ("Ok");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
    GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (w);
    gtk_widget_show (w);
    ret = IDOK;
  }
  else if (mode ==  MB_OKCANCEL)
  {
    w = gtk_button_new_with_label ("Ok");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDOK));
    GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (w);
    gtk_widget_show (w);

    w = gtk_button_new_with_label ("Cancel");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
    gtk_widget_show (w);
    ret = IDCANCEL;
  }
  else if (mode == MB_YESNOCANCEL)
  {
    w = gtk_button_new_with_label ("Yes");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDYES));
    GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (w);
    gtk_widget_show (w);

    w = gtk_button_new_with_label ("No");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDNO));
    gtk_widget_show (w);

    w = gtk_button_new_with_label ("Cancel");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDCANCEL));
    gtk_widget_show (w);
    ret = IDCANCEL;
  }
  else /* if (mode == MB_YESNO) */
  {
    w = gtk_button_new_with_label ("Yes");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDYES));
    GTK_WIDGET_SET_FLAGS (w, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (w);
    gtk_widget_show (w);

    w = gtk_button_new_with_label ("No");
    gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (w), "clicked",
                        GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (IDNO));
    gtk_widget_show (w);
    ret = IDNO;
  }

  gtk_widget_show (window);
  gtk_grab_add (window);

  while (loop)
    gtk_main_iteration ();

  gtk_grab_remove (window);
  gtk_widget_destroy (window);

  return ret;
}

// End of rip from TexTool.cpp

// =============================================================================
// Ripped from cmdlib.cpp

/*
====================
Extract file parts
====================
*/
void ExtractFilePath (const char *path, char *dest)
{
  const char *src;

  src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
  while (src != path && *(src-1) != '/' && *(src-1) != '\\')
    src--;

  memcpy (dest, path, src-path);
  dest[src-path] = 0;
}

void ExtractFileName (const char *path, char *dest)
{
  const char *src;

  src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
  while (src != path && *(src-1) != '/'
         && *(src-1) != '\\' )
    src--;

  while (*src)
  {
    *dest++ = *src++;
  }
  *dest = 0;
}

void ConvertDOSToUnixName( char *dst, const char *src )
{
  while ( *src )
  {
    if ( *src == '\\' )
      *dst = '/';
    else
      *dst = *src;
    dst++; src++;
  }
  *dst = 0;
}

// End of rip from cmdlib.cpp

// =============================================================================
// Actual Plugin Code

// get the wad name from the shader name (or an actual wadname) and add to a list of wad names making
// sure we don't add duplicates.

GSList *AddToWadList(GSList *wadlist, const char *shadername, const char *wad)
{
  char tmpstr[QER_MAX_NAMELEN];
  char *wadname;
  if (!shadername && !wad) return wadlist;

  if (shadername)
  {
    if (strcmp(shadername,"color") == 0)
      return wadlist;
    ExtractFilePath(shadername,tmpstr);
    // Sys_Printf("checking: %s\n",shadername);

    int l = strlen(tmpstr) - 1;

    if (tmpstr[l] == '/' || tmpstr[l] == '\\')
      tmpstr[l] = 0;
    else
    {
      Sys_Printf("WARNING: Unknown wad file for shader %s\n",shadername);
      return wadlist;
    }

    ExtractFileName(tmpstr,tmpstr);

    wadname = (char *)malloc(strlen(tmpstr) + 5);
    sprintf(wadname,"%s.wad",tmpstr);
  }
  else
  {
    wadname=strdup(wad);
  }

  for (GSList *l = wadlist; l != NULL ; l = l->next)
  {
    if (!stricmp((char *)l->data,wadname))
    {
      free( wadname );
      return wadlist;
    }
  }

  Sys_Printf("Adding Wad File to WAD list: %s (reason: ",wadname);
  if (shadername)
    Sys_Printf("see shader \"%s\")\n", shadername);
  else
    Sys_Printf("already in WAD key. )\n");
  return ( g_slist_append (wadlist, wadname ) );
}

void UpdateWadKeyPair( void )
{
  int i,nb;

  char wads[2048]; // change to CString usage ?
  wads[0] = 0;
  char *p1,*p2;
  entity_t *pEntity;
  epair_t *pEpair;
  GSList *wadlist = NULL;
  face_t  *f;
  brush_t *b;
  char cleanwadname[QER_MAX_NAMELEN];
  const char *actualwad;


  pEntity = (entity_t *)g_FuncTable.m_pfnGetEntityHandle(0); // get the worldspawn ent

  Sys_Printf("Searching for in-use wad files...\n");
  for(pEpair = pEntity->epairs; pEpair != NULL; pEpair = pEpair->next)
  {
    if (stricmp(pEpair->key,"wad") == 0)
    {
      strcpy(wads,pEpair->value);
      ConvertDOSToUnixName(wads,wads);

      // ok, we got the list of ; delimited wads, now split it into a GSList that contains
      // just the wad names themselves.

      p1 = wads;

      do
      {
        p2 = strchr(p1,';');
        if (p2)
          *p2 = 0; // swap the ; with a null terminator

        if (strchr(p1,'/') || strchr(p1,'\\'))
        {
          ExtractFileName(p1,cleanwadname);
          wadlist = AddToWadList (wadlist, NULL, cleanwadname);
        }
        else
        {
          wadlist = AddToWadList (wadlist, NULL, p1);
        }
        if (p2)
          p1 = p2+1; // point back to the remainder of the string
        else
          p1 = NULL; // make it so we exit the loop.

      } while (p1);

      // ok, now we have a list of wads in GSList.
      // now we need to add any new wadfiles (with their paths) to this list
      // so scan all brushes and see what wads are in use
      // FIXME: scan brushes only in the region ?

      break; // we don't need to process any more key/pairs.
    }
  }

  nb = g_FuncTable.m_pfnAllocateActiveBrushHandles();
  for( i = 0; i < nb; i++ )
	{
    b = (brush_t *)g_FuncTable.m_pfnGetActiveBrushHandle(i);
    if (b->patchBrush) // patches in halflife ?
    {
      wadlist = AddToWadList(wadlist, b->pPatch->pShader->getName(),NULL);
    } else
    {
      for (f=b->brush_faces ; f ; f=f->next)
      {
        wadlist = AddToWadList(wadlist, f->pShader->getName(),NULL);
      }
    }
  }
  g_FuncTable.m_pfnReleaseActiveBrushHandles();

  nb = g_FuncTable.m_pfnAllocateSelectedBrushHandles();
  for( i = 0; i < nb; i++ )
	{
    b = (brush_t *)g_FuncTable.m_pfnGetSelectedBrushHandle(i);
    if (b->patchBrush) // patches in halflife ?
    {
      wadlist = AddToWadList(wadlist, b->pPatch->pShader->getName(),NULL);
    } else
    {
      for (f=b->brush_faces ; f ; f=f->next)
      {
        wadlist = AddToWadList(wadlist, f->pShader->getName(),NULL);
      }
    }
  }
  g_FuncTable.m_pfnReleaseSelectedBrushHandles();

  // Now we have a complete list of wadnames (without paths) so we just have to turn this
  // back to a ; delimited list.

  wads[0] = 0;
  while (wadlist)
  {
    if (stricmp((char *)wadlist->data,"common-hydra.wad") == 0)
    {
      Sys_Printf("Skipping radiant-supplied wad file %s\n",(char *)wadlist->data);
    }
    else
    {
      if (wads[0])
        strcat(wads,";");

      actualwad = vfsGetFullPath((char *)wadlist->data);

      if (actualwad)
      {
        strcat(wads, actualwad);
      }
      else
      {
        Sys_Printf("WARNING: could not locate wad file %s\n",(char *)wadlist->data);
        strcat(wads, (char *)wadlist->data);
      }
    }

    free (wadlist->data);
    wadlist = g_slist_remove (wadlist, wadlist->data);
  }

  // store the wad list back in the worldspawn.
  if (wads[0])
  {
    //free(pEpair->value);
    //pEpair->value = strdup(wads);
    SetKeyValue(pEntity, "wad", wads);
  }

}

// =============================================================================
// PLUGIN INTERFACE STUFF

// plugin name
const char *PLUGIN_NAME = "Q3 Texture Tools";

// commands in the menu
const char *PLUGIN_COMMANDS = "About...;Create/Update WAD keypair";

const char *PLUGIN_ABOUT = "HydraToolz for GTKRadiant\n\n"
                           "By Hydra!";

extern "C" void* WINAPI QERPlug_GetFuncTable ()
{
  return &g_FuncTable;
}

const char* QERPlug_Init (void* hApp, void *pWidget)
{
  GtkWidget* pMainWidget = static_cast<GtkWidget*>(pWidget);

  g_pMainWnd = pMainWidget;
  memset(&g_FuncTable, 0, sizeof(_QERFuncTable_1));
  g_FuncTable.m_nSize = sizeof(_QERFuncTable_1);
  return "HydraToolz for GTKRadiant"; // do we need this ? hmmm
}

const char* QERPlug_GetName()
{
  return (char*)PLUGIN_NAME;
}

const char* QERPlug_GetCommandList()
{
  return PLUGIN_COMMANDS;
}

extern "C" void QERPlug_Dispatch(const char* p, vec3_t vMin, vec3_t vMax, bool bSingleBrush)
{
	if(!strcmp(p, "Create/Update WAD keypair"))
		UpdateWadKeyPair();
	else if(!strcmp(p, "About..."))
		g_FuncTable.m_pfnMessageBox((GtkWidget*)NULL, PLUGIN_ABOUT, "About", eMB_OK);
}

// =============================================================================
// SYNAPSE

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientHydraToolz g_SynapseClient;

extern "C" CSynapseClient* SYNAPSE_DLL_EXPORT Synapse_EnumerateInterfaces (const char *version, CSynapseServer *pServer)
{
  if (strcmp(version, SYNAPSE_VERSION))
  {
    Syn_Printf("ERROR: synapse API version mismatch: should be '" SYNAPSE_VERSION "', got '%s'\n", version);
    return NULL;
  }
  g_pSynapseServer = pServer;
  g_pSynapseServer->IncRef();
  Set_Syn_Printf(g_pSynapseServer->Get_Syn_Printf());

  g_SynapseClient.AddAPI(PLUGIN_MAJOR, "HydraToolz", sizeof(_QERPluginTable));
  g_SynapseClient.AddAPI(RADIANT_MAJOR, NULL, sizeof(g_FuncTable), SYN_REQUIRE, &g_FuncTable);
  g_SynapseClient.AddAPI(VFS_MAJOR, "wad", sizeof(g_FileSystemTable), SYN_REQUIRE, &g_FileSystemTable);
  g_SynapseClient.AddAPI(ENTITY_MAJOR, NULL, sizeof(g_EntityTable), SYN_REQUIRE, &g_EntityTable);
  return &g_SynapseClient;
}

bool CSynapseClientHydraToolz::RequestAPI(APIDescriptor_t *pAPI)
{
  if (!strcmp(pAPI->major_name, PLUGIN_MAJOR))
  {
    _QERPluginTable *pTable = static_cast<_QERPluginTable*>(pAPI->mpTable);
    pTable->m_pfnQERPlug_Init = QERPlug_Init;
    pTable->m_pfnQERPlug_GetName = QERPlug_GetName;
    pTable->m_pfnQERPlug_GetCommandList = QERPlug_GetCommandList;
    pTable->m_pfnQERPlug_Dispatch = QERPlug_Dispatch;
    return true;
  }

  Syn_Printf("ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo());
  return false;
}

const char* CSynapseClientHydraToolz::GetInfo()
{
  return "HydraToolz plugin built " __DATE__ " " RADIANT_VERSION;
}
