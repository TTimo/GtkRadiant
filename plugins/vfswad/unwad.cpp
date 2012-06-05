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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "unwad.h"


wadFile_t *wadCleanup( wadFile_t *wf ){
	if ( wf ) {
		if ( wf->fin ) {
			fclose( wf->fin );
		}
		if ( wf->lpHeader ) {
			free( wf->lpHeader );
		}
		if ( wf->lpLump ) {
			free( wf->lpLump );
		}
		if ( wf->lpMip ) {
			free( wf->lpMip );
		}
		if ( wf->wadfilename ) {
			free( wf->wadfilename );
		}
		free( wf );
		wf = NULL;
	}
	return wf;
}

int wadGetCurrentFileInfo( wadFile_t *wf, char *szFileName, unsigned long fileNameBufferSize, unsigned long *filesize ){
	/* returns 0 if error, or 1 for sucess */
	// if this fails you'll need to re-position the fileposition
	// before attempting any other calls.  e.g. call wadGoToFirstFile()

	if ( fread( wf->lpLump,sizeof( WAD3_LUMP ),1,wf->fin ) != 1 ) {
		return 0;
	}
	strncpy( szFileName, wf->lpLump->name, fileNameBufferSize );
	szFileName[fileNameBufferSize - 1] = 0; // null terminate

	*filesize = wf->lpLump->size;

	return 1;
}

int wadGoToFile( wadFile_t *wf, unsigned long filenum ){
	if ( !wf ) {
		return 0;
	}

	if ( !wf->fin ) {
		return 0;
	}

	if ( filenum >= wf->lpHeader->numlumps ) {
		return 0;
	}

	if ( fseek( wf->fin,wf->lpHeader->infotableofs + ( filenum * sizeof( WAD3_LUMP ) ),SEEK_SET ) != 0 ) {
		return 0;
	}

	wf->currentfile = filenum;

	return 1;
}

int wadGoToNextFile( wadFile_t *wf ){
	return( wadGoToFile( wf, wf->currentfile + 1 ) );
}

int wadGoToFirstFile( wadFile_t *wf ){
	/* returns 0 if error, or 1 for sucess */

	if ( !wf ) {
		return 0;
	}

	if ( !wf->fin ) {
		return 0;
	}

	if ( fseek( wf->fin,wf->lpHeader->infotableofs,SEEK_SET ) != 0 ) {
		return 0;
	}

	wf->currentfile = 0;

	return 1;
}

wadFile_t *wadOpen( const char* path ){

	wadFile_t *wf = NULL;

	if ( !path ) {
		return NULL;
	}

	wf = new wadFile_s;
	memset( wf, 0, sizeof( *wf ) );

	if ( !wf ) {
		return NULL;
	}

	wf->fin = fopen( path,"rb" );
	if ( wf->fin == NULL ) {
		return wadCleanup( wf );
	}

	// get the file size
	if ( fseek( wf->fin,0,SEEK_END ) != 0 ) {
		return wadCleanup( wf );
	}

	wf->FileSize = ftell( wf->fin );

	// Make sure it's at least big enough to manipulate the header
	if ( wf->FileSize < sizeof( WAD3_HEADER ) ) {
		// WAD3 file is malformed.
		return wadCleanup( wf );
	}

	// go back to the start
	if ( fseek( wf->fin,0,SEEK_SET ) != 0 ) {
		return wadCleanup( wf );
	}

	// allocate buffers
	wf->lpHeader = (LPWAD3_HEADER) malloc( sizeof( WAD3_HEADER ) );
	wf->lpLump = (LPWAD3_LUMP) malloc( sizeof( WAD3_LUMP ) );
	wf->lpMip = (LPWAD3_MIP) malloc( sizeof( WAD3_MIP ) );

	if ( !( wf->lpHeader ) || !( wf->lpLump ) || !( wf->lpMip ) ) {
		return wadCleanup( wf );
	}

	// read the header.
	if ( fread( wf->lpHeader,sizeof( WAD3_HEADER ),1,wf->fin ) != 1 ) {
		return wadCleanup( wf );
	}

	if ( wf->lpHeader->identification != WAD2_ID && wf->lpHeader->identification != WAD3_ID ) {
		// Invalid WAD3 header id.
		return wadCleanup( wf );
	}

	// Make sure our table is really there
	if ( ( ( wf->lpHeader->numlumps * sizeof( WAD3_LUMP ) ) + wf->lpHeader->infotableofs ) > wf->FileSize ) {
		// WAD3 file is malformed.
		return wadCleanup( wf );
	}

	// Store the name of the wadfile
	if ( !( wf->wadfilename = strdup( path ) ) ) {
		return wadCleanup( wf );
	}

	return wf;
}

int wadOpenCurrentFileByNum( wadFile_t *wf, unsigned long filenumber ){
	/* returns 0 if error, or 1 for sucess */
	return( wadGoToFile( wf, filenumber ) );
}

void wadCloseCurrentFile( wadFile_t *wf ){
	// nothing to do really...
}

unsigned long wadReadCurrentFile( wadFile_t *wf, char *bufferptr, unsigned long size ){
	// returns 0 if error, or the amount of data read into the buffer
	if ( fread( wf->lpLump,sizeof( WAD3_LUMP ),1,wf->fin ) != 1 ) {
		return 0;
	}

	// dunno how to handle any other image types but this (yet)
	if ( wf->lpLump->type != WAD2_TYPE_MIP && wf->lpLump->type != WAD3_TYPE_MIP ) {
		return 0;
	}

	// go to first mip
	if ( fseek( wf->fin, wf->lpLump->filepos, SEEK_SET ) != 0 ) {
		return 0;
	}

	if ( fread( bufferptr,size,1,wf->fin ) == 1 ) {
		return ( size );
	}
	else{
		return 0;
	}
}

/*

   .. or we could do it the long way, and process the file as we go..


 */
/*
   unsigned long wadReadCurrentFile (wadFile_t *wf , char *bufferptr, unsigned long size)
   {
   // returns 0 if error, or the amount of data read into the buffer
   unsigned long bufferpos;
   unsigned long mipdatasize;
   WORD palettesize;

   if (fread(wf->lpLump,sizeof(WAD3_LUMP),1,wf->fin)!=1)
        return 0;

   if (wf->lpLump->type == WAD3_TYPE_MIP) // can we handle it ?
   {

    // bounds check.
    if (wf->lpLump->filepos >= wf->FileSize)
      return 0; // malformed wad3

    // go to first mip
    if (fseek(wf->fin, wf->lpLump->filepos, SEEK_SET) != 0)
        return 0;

    // and read it
    if (fread(wf->lpMip,sizeof(WAD3_MIP),1,wf->fin)!=1)
      return 0;

    // store in buffer.
    memcpy(bufferptr, wf->lpMip, sizeof(WAD3_MIP));
    bufferpos = sizeof(WAD3_MIP);

    // now read the MIP data.
    // mip data
    if (fseek(wf->fin, wf->lpLump->filepos + wf->lpMip->offsets[0], SEEK_SET) != 0)
        return 0;

    mipdatasize = GET_MIP_DATA_SIZE(wf->lpMip->width,wf->lpMip->height);

    if (fread(bufferptr+bufferpos, mipdatasize, 1, wf->fin)!=1)
      return 0;

    bufferpos += mipdatasize;

    // ok, that's the mip data itself, now grab the palette size.
    if (fread(bufferptr+bufferpos,sizeof(WORD),1,wf->fin)!=1)
      return 0;

    palettesize = *(WORD *)(bufferptr+bufferpos);

    bufferpos += sizeof(WORD);

    // grab the palette itself
    if (fread(bufferptr+bufferpos,palettesize*3,1,wf->fin)!=1)
      return 0;

    bufferpos += palettesize*3;

    // and finally the one-word padding.
    if (fread(bufferptr+bufferpos,sizeof(WORD),1,wf->fin)!=1)
      return 0;

    bufferpos += sizeof(WORD);

    return(bufferpos); // return the amount of bytes read.
   }
   return 0;
   }
 */
