/*
   BobToolz plugin for GtkRadiant
   Copyright (C) 2001 Gordon Biggans

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

#include "StdAfx.h"
#include "CPortals.h"
#include "misc.h"

#define LINE_BUF 1000
#define MSG_PREFIX "bobToolz plugin: "

// these classes are far less of a mess than my code was,
// thanq to G.DeWan 4 the prtview source on which it was based

CBspPortal::CBspPortal(){
	memset( this, 0, sizeof( CBspPortal ) );
}

CBspPortal::~CBspPortal(){
	delete[] point;
}

void ClampFloat( float* p ){
	double i;
	double frac = modf( *p, &i );

	if ( !frac ) {
		return;
	}

	if ( fabs( *p - ceil( *p ) ) < MAX_ROUND_ERROR ) {
		*p = ceilf( *p );
	}

	if ( fabs( *p - floor( *p ) ) < MAX_ROUND_ERROR ) {
		*p = floorf( *p );
	}
}

bool CBspPortal::Build( char *def, unsigned int pointCnt, bool bInverse ){
	char *c = def;
	unsigned int n;

	point_count = pointCnt;

	if ( point_count < 3 ) {
		return FALSE;
	}

	point = new CBspPoint[point_count];

	for ( n = 0; n < point_count; n++ )
	{
		for (; *c != 0 && *c != '('; c++ ) ;

		if ( *c == 0 ) {
			return FALSE;
		}

		c++;

		int x;
		if ( bInverse ) {
			x = point_count - n - 1;
		}
		else{
			x = n;
		}

		sscanf( c, "%f %f %f", &point[x].p[0], &point[x].p[1], &point[x].p[2] );

		ClampFloat( &point[x].p[0] );
		ClampFloat( &point[x].p[1] );
		ClampFloat( &point[x].p[2] );
	}

	return TRUE;
}

CPortals::CPortals(){
	memset( this, 0, sizeof( CPortals ) );
}

CPortals::~CPortals(){
	Purge();
}

void CPortals::Purge(){
	if ( node ) {
		delete[] node;
	}
	node = NULL;
	node_count = 0;
}

void CPortals::Load(){
	char buf[LINE_BUF + 1];

	memset( buf, 0, LINE_BUF + 1 );

	Purge();

	Sys_Printf( MSG_PREFIX "Loading portal file %s.\n", fn );

	FILE *in;

	in = fopen( fn, "rt" );

	if ( in == NULL ) {
		Sys_FPrintf( SYS_ERR, "ERROR - could not open file.\n" );

		return;
	}

	if ( !fgets( buf, LINE_BUF, in ) ) {
		fclose( in );

		Sys_FPrintf( SYS_ERR, "ERROR - File ended prematurely.\n" );

		return;
	}

	if ( strncmp( "PRT1", buf, 4 ) != 0 ) {
		fclose( in );

		Sys_FPrintf( SYS_ERR, "ERROR - File header indicates wrong file type (should be \"PRT1\").\n" );

		return;
	}

	if ( !fgets( buf, LINE_BUF, in ) ) {
		fclose( in );

		Sys_FPrintf( SYS_ERR, "ERROR - File ended prematurely.\n" );

		return;
	}

	sscanf( buf, "%u", &node_count );

	if ( node_count > 0xFFFF ) {
		fclose( in );

		node_count = 0;

		Sys_FPrintf( SYS_ERR, "ERROR - Extreme number of nodes, aborting.\n" );

		return;
	}

	if ( !fgets( buf, LINE_BUF, in ) ) {
		fclose( in );

		node_count = 0;

		Sys_FPrintf( SYS_ERR, "ERROR - File ended prematurely.\n" );

		return;
	}

	unsigned int p_count;
	sscanf( buf, "%u", &p_count );

	if ( !fgets( buf, LINE_BUF, in ) ) {
		fclose( in );

		node_count = 0;

		Sys_FPrintf( SYS_ERR, "ERROR - File ended prematurely.\n" );

		return;
	}

	unsigned int p_count2;
	sscanf( buf, "%u", &p_count2 );

	node = new CBspNode[node_count];

	unsigned int i;
	for ( i = 0; i < p_count; i++ )
	{
		if ( !fgets( buf, LINE_BUF, in ) ) {
			fclose( in );

			node_count = 0;

			Sys_FPrintf( SYS_ERR, "ERROR - File ended prematurely.\n" );

			return;
		}

		unsigned int dummy, node1, node2;
		sscanf( buf, "%u %u %u", &dummy, &node1, &node2 );

		node[node1].portal_count++;
		node[node2].portal_count++;
	}

	for ( i = 0; i < p_count2; i++ )
	{
		if ( !fgets( buf, LINE_BUF, in ) ) {
			fclose( in );

			node_count = 0;

			Sys_FPrintf( SYS_ERR, "ERROR - File ended prematurely.\n" );

			return;
		}

		unsigned int dummy, node1;
		sscanf( buf, "%u %u", &dummy, &node1 );

		node[node1].portal_count++;
	}

	for ( i = 0; i < node_count; i++ )
		node[i].portal = new CBspPortal[node[i].portal_count];

	fclose( in );

	in = fopen( fn, "rt" );

	fgets( buf, LINE_BUF, in );
	fgets( buf, LINE_BUF, in );
	fgets( buf, LINE_BUF, in );
	fgets( buf, LINE_BUF, in );

	unsigned int n;
	for ( n = 0; n < p_count; n++ )
	{
		if ( !fgets( buf, LINE_BUF, in ) ) {
			fclose( in );

			Purge();

			Sys_FPrintf( SYS_ERR, "ERROR - Could not find information for portal number %d of %d.\n", n + 1, p_count );

			return;
		}

		unsigned int pCount, node1, node2;
		sscanf( buf, "%u %u %u", &pCount, &node1, &node2 );

		if ( !node[node1].AddPortal( buf, pCount, FALSE ) ) {
			fclose( in );

			Purge();

			Sys_FPrintf( SYS_ERR, "ERROR - Information for portal number %d of %d is not formatted correctly.\n", n + 1, p_count );

			return;
		}

		if ( !node[node2].AddPortal( buf, pCount, TRUE ) ) {
			fclose( in );

			Purge();

			Sys_FPrintf( SYS_ERR, "ERROR - Information for portal number %d of %d is not formatted correctly.\n", n + 1, p_count );

			return;
		}
	}

	for ( n = 0; n < p_count2; n++ )
	{
		if ( !fgets( buf, LINE_BUF, in ) ) {
			fclose( in );

			Purge();

			Sys_FPrintf( SYS_ERR, "ERROR - Could not find information for portal number %d of %d.\n", n + 1, p_count );

			return;
		}

		unsigned int pCount, node1;
		sscanf( buf, "%u %u", &pCount, &node1 );

		if ( !node[node1].AddPortal( buf, pCount, FALSE ) ) {
			fclose( in );

			Purge();

			Sys_FPrintf( SYS_ERR, "ERROR - Information for portal number %d of %d is not formatted correctly.\n", n + 1, p_count );

			return;
		}
	}

	fclose( in );
}

CBspNode::CBspNode(){
	portal = NULL;
	portal_count = 0;
	portal_next = 0;
}

CBspNode::~CBspNode(){
	if ( portal != NULL ) {
		delete[] portal;
	}
}

bool CBspNode::AddPortal( char *def, unsigned int pointCnt, bool bInverse ){
	return portal[portal_next++].Build( def, pointCnt, bInverse );
}
