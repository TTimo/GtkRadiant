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

// =============================================================================
// global tables

#include "plugin.h"

_QERFuncTable_1 g_FuncTable;
_QERFileSystemTable g_FileSystemTable;

// =============================================================================
// SYNAPSE

#include "synapse.h"

class CSynapseClientImage : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();

CSynapseClientImage() { }
virtual ~CSynapseClientImage() { }
};

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientImage g_SynapseClient;

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif
extern "C" CSynapseClient * SYNAPSE_DLL_EXPORT Synapse_EnumerateInterfaces( const char *version, CSynapseServer *pServer ) {
#if __GNUC__ >= 4
#pragma GCC visibility pop
#endif
	if ( strcmp( version, SYNAPSE_VERSION ) ) {
		Syn_Printf( "ERROR: synapse API version mismatch: should be '" SYNAPSE_VERSION "', got '%s'\n", version );
		return NULL;
	}
	g_pSynapseServer = pServer;
	g_pSynapseServer->IncRef();
	Set_Syn_Printf( g_pSynapseServer->Get_Syn_Printf() );

	g_SynapseClient.AddAPI( IMAGE_MAJOR, "png", sizeof( _QERPlugImageTable ) );
	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ), SYN_REQUIRE, &g_FuncTable );
	g_SynapseClient.AddAPI( VFS_MAJOR, "*", sizeof( _QERFileSystemTable ), SYN_REQUIRE, &g_FileSystemTable );

	return &g_SynapseClient;
}

bool CSynapseClientImage::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, IMAGE_MAJOR ) ) {
		_QERPlugImageTable* pTable = static_cast<_QERPlugImageTable*>( pAPI->mpTable );
		if ( !strcmp( pAPI->minor_name, "png" ) ) {
			pTable->m_pfnLoadImage = &LoadImage;
			return true;
		}
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

#include "version.h"

const char* CSynapseClientImage::GetInfo(){
	return "PNG loader module built " __DATE__ " " RADIANT_VERSION;
}



// ====== PNG loader functionality ======

#include "png.h"

#ifdef __APPLE__    //tigital
#include <stdlib.h>
#endif

void user_warning_fn( png_structp png_ptr, png_const_charp warning_msg ){
	g_FuncTable.m_pfnSysPrintf( "libpng warning: %s\n", warning_msg );
}

void user_error_fn( png_structp png_ptr, png_const_charp error_msg ){
	g_FuncTable.m_pfnSysPrintf( "libpng error: %s\n", error_msg );
	longjmp( png_jmpbuf(png_ptr), 0 );
}

void user_read_data( png_structp png_ptr, png_bytep data, png_uint_32 length ){
	png_bytep *p_p_fbuffer = (png_bytep*)png_get_io_ptr( png_ptr );
	memcpy( data, *p_p_fbuffer, length );
	*p_p_fbuffer += length;
}

void LoadImage( const char *filename, unsigned char **pic, int *width, int *height ){
	png_byte** row_pointers;
	unsigned char *fbuffer = NULL;
	png_bytep p_fbuffer;

	int nLen = g_FileSystemTable.m_pfnLoadFile( (char *)filename, (void **)&fbuffer, 0 );
	if ( nLen == -1 ) {
		return;
	}

	p_fbuffer = fbuffer;

	// the reading glue
	// http://www.libpng.org/pub/png/libpng-manual.html

	png_structp png_ptr = png_create_read_struct
							  ( PNG_LIBPNG_VER_STRING, NULL,
							  user_error_fn, user_warning_fn );
	if ( !png_ptr ) {
		g_FuncTable.m_pfnSysPrintf( "libpng error: png_create_read_struct\n" );
		return;
	}

	png_infop info_ptr = png_create_info_struct( png_ptr );
	if ( !info_ptr ) {
		png_destroy_read_struct( &png_ptr,
								 NULL, NULL );
		g_FuncTable.m_pfnSysPrintf( "libpng error: png_create_info_struct (info_ptr)\n" );
		return;
	}

	png_infop end_info = png_create_info_struct( png_ptr );
	if ( !end_info ) {
		png_destroy_read_struct( &png_ptr, &info_ptr,
								 NULL );
		g_FuncTable.m_pfnSysPrintf( "libpng error: png_create_info_struct (end_info)\n" );
		return;
	}

	// configure the read function
	png_set_read_fn( png_ptr, ( void * ) & p_fbuffer, ( png_rw_ptr ) & user_read_data );

	if ( setjmp( png_jmpbuf(png_ptr) ) ) {
		png_destroy_read_struct( &png_ptr, &info_ptr,
								 &end_info );
		if ( *pic ) {
			g_free( *pic );
			free( row_pointers );
		}
		return;
	}

	png_read_info( png_ptr, info_ptr );

	int bit_depth = png_get_bit_depth( png_ptr, info_ptr );
	int color_type = png_get_color_type( png_ptr, info_ptr );

	// we want to treat all images the same way
	//   The following code transforms grayscale images of less than 8 to 8 bits,
	//   changes paletted images to RGB, and adds a full alpha channel if there is
	//   transparency information in a tRNS chunk.
	if ( color_type == PNG_COLOR_TYPE_PALETTE ) {
		png_set_palette_to_rgb( png_ptr );
	}

	if ( color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8 ) {
		png_set_expand_gray_1_2_4_to_8( png_ptr );
	}

	if ( png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) ) {
		png_set_tRNS_to_alpha( png_ptr );
	}

	if ( !( color_type & PNG_COLOR_MASK_ALPHA ) ) {
		// Set the background color to draw transparent and alpha images over.
		png_color_16 my_background, *image_background;

		if ( png_get_bKGD( png_ptr, info_ptr, &image_background ) ) {
			png_set_background( png_ptr, image_background,
								PNG_BACKGROUND_GAMMA_FILE, 1, 1.0 );
		}
		else{
			png_set_background( png_ptr, &my_background,
								PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0 );
		}

		// Add alpha byte after each RGB triplet
		png_set_filler( png_ptr, 0xff, PNG_FILLER_AFTER );
	}

	// read the sucker in one chunk
	png_read_update_info( png_ptr, info_ptr );

	color_type = png_get_color_type( png_ptr, info_ptr );
	bit_depth = png_get_bit_depth( png_ptr, info_ptr );

	*width = png_get_image_width( png_ptr, info_ptr );
	*height = png_get_image_height( png_ptr, info_ptr );

	// allocate the pixel buffer, and the row pointers
	int size = ( *width ) * ( *height ) * 4;
	// still have to use that g_malloc heresy
	*pic = (unsigned char *)g_malloc( size );
	row_pointers = (png_byte**) malloc( ( *height ) * sizeof( png_byte* ) );

	int i;
	for ( i = 0; i < ( *height ); i++ )
		row_pointers[i] = (png_byte*)( *pic ) + i * 4 * ( *width );

	// actual read
	png_read_image( png_ptr, row_pointers );

	/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
	png_read_end( png_ptr, info_ptr );

	/* free up the memory structure */
	png_destroy_read_struct( &png_ptr, &info_ptr, NULL );

	free( row_pointers );
	g_FileSystemTable.m_pfnFreeFile( fbuffer );
}
