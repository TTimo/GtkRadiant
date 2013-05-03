/*
   GenSurf plugin for GtkRadiant
   Copyright (C) 2001 David Hyde, Loki software and qeradiant.com

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "gensurf.h"

// Heretic 2 - specific routines

typedef struct palette_s
{
	guint8 r,g,b;
} palette_t;

#define MIP_VERSION     2
#define PAL_SIZE        256
#define MIPLEVELS       16

typedef struct miptex_s
{
	int version;
	char name[32];
	unsigned width[MIPLEVELS], height[MIPLEVELS];
	unsigned offsets[MIPLEVELS];        // four mip maps stored
	char animname[32];                  // next frame in animation chain
	palette_t palette[PAL_SIZE];
	int flags;
	int contents;
	int value;
} miptex_t;

//=============================================================
int GetDefSurfaceProps( char *Tex ){
	return 0; // leo: only used for Heretic 2, fix later
	/*
	   char		path[NAME_MAX];
	   char        *p;
	   int         flags;
	   miptex_t	*mt;
	   FILE	    *f;
	   int         length;
	   int         pos;

	   if(Game != HERETIC2) return 0;
	   if(!strlen(Tex)) return 0;

	   mt = NULL;
	   flags = 0;
	   if(UsePak[Game])
	   {
	      FILE         *fpak;
	      pak_header_t pakheader;
	      pak_item_t   pakitem;
	      int          i;
	      int          num;
	      int          numitems;

	      if (NULL != (fpak = fopen(pakfile[Game], "rb")))
	      {
	          sprintf(path,"textures/%s.m8",Tex);
	          strlwr(path);
	          num=fread(&pakheader,1,sizeof(pak_header_t),fpak);
	          if((size_t)num < sizeof(pak_header_t))
	          {
	              fclose(fpak);
	              return 0;
	          }
	          if(strncmp(pakheader.id,"PACK",4))
	          {
	              fclose(fpak);
	              return 0;
	          }
	          numitems = pakheader.dsize/sizeof(pak_item_t);
	          fseek(fpak,pakheader.dstart,SEEK_SET);
	          for(i=0; i<numitems; i++)
	          {
	              fread(&pakitem,1,sizeof(pak_item_t),fpak);
	              if(strstr(pakitem.name,path))
	              {
	                  fseek(fpak,pakitem.start,SEEK_SET);
	                  if((mt = (miptex_t*)malloc(sizeof(miptex_t)))==NULL)
	                  {
	                      fclose(fpak);
	                      return 0;
	                  }
	                  else
	                  {
	                      fread(mt, 1, sizeof(miptex_t), fpak);
	                      flags = mt->flags;
	                      free(mt);
	                  }
	              }
	          }
	          fclose(fpak);
	      }
	   }
	   else
	   {
	      // Assume .map will be output to gamedir/maps, then back up
	      // to the gamedir and append /textures. Ugly but it should work
	      strcpy(path,gszMapFile);
	      strlwr(path);
	      p = strstr(path,"maps");
	      if(!p) return 0;
	      p[0] = '\0';
	      strcat(path,"textures/");
	      strcat(path,Tex);
	      strcat(path,".m8");
	      f = fopen (path, "rb");
	      if (!f)
	          flags = 0;
	      else
	      {
	          pos = ftell (f);
	          fseek (f, 0, SEEK_END);
	          length = ftell (f);
	          fseek (f, pos, SEEK_SET);
	          if((mt = (miptex_t*)malloc(length+1))==NULL)
	              flags = 0;
	          else
	          {
	              ((char *)mt)[length] = 0;
	              fread(mt, 1, length, f);
	              fclose (f);
	              flags = mt->flags;
	              free(mt);
	          }
	      }
	   }
	   return flags;
	 */
}
