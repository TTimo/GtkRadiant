// XMLPush.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main(int argc, char* argv[])
{
  FILE *f;
  xmlDocPtr doc;
  
  f = fopen("XMLDump.xml", "r");
  if (f != NULL) {
    int res, size = 1024;
    char chars[1024];
    xmlParserCtxtPtr ctxt;
    
    res = fread(chars, 1, 4, f);
    if (res > 0) {
      ctxt = xmlCreatePushParserCtxt(NULL, NULL,
        chars, res, "foooo (filename)?");
      while ((res = fread(chars, 1, size, f)) > 0) {
        xmlParseChunk(ctxt, chars, res, 0);
      }
      xmlParseChunk(ctxt, chars, 0, 1);
      doc = ctxt->myDoc;
      xmlFreeParserCtxt(ctxt);
    }
  }
  return 0;
}
