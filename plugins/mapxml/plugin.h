#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <stdlib.h>
#include <stdio.h>

#include "libxml/parser.h"

#include "synapse.h"
#define USE_QERTABLE_DEFINE
#include "qerplugin.h"
#include "imap.h"
#include "ishaders.h"
#define USE_ENTITYTABLE_DEFINE
#include "ientity.h"
#define USE_BRUSHTABLE_DEFINE
#include "ibrush.h"
#define USE_PATCHTABLE_DEFINE
#include "ipatch.h"

extern _QERFuncTable_1 g_FuncTable;
extern _QERShadersTable g_ShadersTable;
extern _QEREntityTable g_EntityTable;
extern _QERBrushTable g_BrushTable;
extern _QERPatchTable g_PatchTable;

#define Error g_FuncTable.m_pfnError
#define QERApp_Shader_ForName g_ShadersTable.m_pfnShader_ForName

extern void *g_pRadiantWnd;

void Map_Read (IDataStream *in, CPtrArray *map);
void Map_Write (CPtrArray *map, IDataStream *out);

extern CSynapseServer* g_pSynapseServer;

class CSynapseClientXMap : public CSynapseClient
{
public:
  // CSynapseClient API
  bool RequestAPI(APIDescriptor_t *pAPI);
  const char* GetInfo();
  const char* GetName(); ///< required for XML runtime config
  
  CSynapseClientXMap() { }
  virtual ~CSynapseClientXMap() { }
};

#endif // _PLUGIN_H_
