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

#include "plugin.h"
#include "entity.h"
#include "light.h"

void DrawSphere( vec3_t center, float radius, int sides, int nGLState ){
	int i, j;
	float dt = (float) ( 2 * Q_PI / (float) sides );
	float dp = (float) ( Q_PI / (float) sides );
	float t, p;
	vec3_t v;

	if ( radius <= 0 ) {
		return;
	}

	g_QglTable.m_pfn_qglBegin( GL_TRIANGLES );
	for ( i = 0; i <= sides - 1; i++ ) {
		for ( j = 0; j <= sides - 2; j++ ) {
			t = i * dt;
			p = (float) ( ( j * dp ) - ( Q_PI / 2 ) );

			VectorPolar( v, radius, t, p );
			VectorAdd( v, center, v );
			g_QglTable.m_pfn_qglVertex3fv( v );

			VectorPolar( v, radius, t, p + dp );
			VectorAdd( v, center, v );
			g_QglTable.m_pfn_qglVertex3fv( v );

			VectorPolar( v, radius, t + dt, p + dp );
			VectorAdd( v, center, v );
			g_QglTable.m_pfn_qglVertex3fv( v );

			VectorPolar( v, radius, t, p );
			VectorAdd( v, center, v );
			g_QglTable.m_pfn_qglVertex3fv( v );

			VectorPolar( v, radius, t + dt, p + dp );
			VectorAdd( v, center, v );
			g_QglTable.m_pfn_qglVertex3fv( v );

			VectorPolar( v, radius, t + dt, p );
			VectorAdd( v, center, v );
			g_QglTable.m_pfn_qglVertex3fv( v );
		}
	}

	p = (float) ( ( sides - 1 ) * dp - ( Q_PI / 2 ) );
	for ( i = 0; i <= sides - 1; i++ ) {
		t = i * dt;

		VectorPolar( v, radius, t, p );
		VectorAdd( v, center, v );
		g_QglTable.m_pfn_qglVertex3fv( v );

		VectorPolar( v, radius, t + dt, p + dp );
		VectorAdd( v, center, v );
		g_QglTable.m_pfn_qglVertex3fv( v );

		VectorPolar( v, radius, t + dt, p );
		VectorAdd( v, center, v );
		g_QglTable.m_pfn_qglVertex3fv( v );
	}
	g_QglTable.m_pfn_qglEnd();
}

#define LIGHT_ATTEN_LINEAR  1
#define LIGHT_ATTEN_ANGLE       2
#define LIGHT_ATTEN_DISTANCE    4

#define LIGHT_Q3A_DEFAULT       ( LIGHT_ATTEN_ANGLE | LIGHT_ATTEN_DISTANCE )
#define LIGHT_WOLF_DEFAULT  ( LIGHT_ATTEN_LINEAR | LIGHT_ATTEN_DISTANCE )

float CalculateEnvelopeForLight( entity_t * e, float fFalloffTolerance ){
	float fEnvelope = 0.f;
	int iSpawnFlags = atoi( ValueForKey( e, "spawnflags" ) );
	int iLightFlags = 0;
	float fFade = 1.f;
	float fIntensity, fPhotons;
	float fScale;
	const char *gameFile = g_FuncTable.m_pfnGetGameFile();

	// These variables are tweakable on the q3map2 console, setting to q3map2
	// default here as there is no way to find out what the user actually uses
	// right now. Maybe move them to worldspawn?
	float fPointScale = 7500.f;
	float fLinearScale = 1.f / 8000.f;
	//float fFalloffTolerance = 1.f;  // Need it as parameter

	// Arnout: HACK for per-game radii - really need to move this to a per-game module?
	if ( !strcmp( gameFile, "wolf.game" ) || !strcmp( gameFile, "et.game" ) ) {
		// Spawnflags :
		// 1: nonlinear
		// 2: angle

		// set default flags
		iLightFlags = LIGHT_WOLF_DEFAULT;

		// inverse distance squared attenuation?
		if ( iSpawnFlags & 1 ) {
			iLightFlags &= ~LIGHT_ATTEN_LINEAR;
			iLightFlags |= LIGHT_ATTEN_ANGLE;
		}
		// angle attenuate
		if ( iSpawnFlags & 2 ) {
			iLightFlags |= LIGHT_ATTEN_ANGLE;
		}
	}
	else {
		// Spawnflags :
		// 1: linear
		// 2: no angle

		// set default flags
		iLightFlags = LIGHT_Q3A_DEFAULT;

		// linear attenuation?
		if ( iSpawnFlags & 1 ) {
			iLightFlags |= LIGHT_ATTEN_LINEAR;
			iLightFlags &= ~LIGHT_ATTEN_ANGLE;
		}
		// no angle attenuate?
		if ( iSpawnFlags & 2 ) {
			iLightFlags &= ~LIGHT_ATTEN_ANGLE;
		}
	}

	// set fade key (from wolf)
	if ( iLightFlags & LIGHT_ATTEN_LINEAR ) {
		fFade = FloatForKey( e, "fade" );
		if ( fFade <= 0.f ) {
			fFade = 1.f;
		}
	}
	// set light intensity
	fIntensity = FloatForKey( e, "_light" );
	if ( fIntensity == 0.f ) {
		fIntensity = FloatForKey( e, "light" );
	}
	if ( fIntensity == 0.f ) {
		fIntensity = 300.f;
	}

	// set light scale (sof2)
	fScale = FloatForKey( e, "scale" );
	if ( fScale <= 0.f ) {
		fScale = 1.f;
	}
	fIntensity *= fScale;

	// amount of photons
	fPhotons = fIntensity * fPointScale;

	// calculate envelope

	// solve distance for non-distance lights
	if ( !( iLightFlags & LIGHT_ATTEN_DISTANCE ) ) {
		//!\todo (spog) can't access global objects in a module - globals are EVIL - solution: API for querying global settings.
		fEnvelope = 131072 /*g_MaxWorldCoord * 2.f*/;
	}
	// solve distance for linear lights
	else if ( iLightFlags & LIGHT_ATTEN_LINEAR ) {
		fEnvelope = ( ( fPhotons * fLinearScale ) - fFalloffTolerance ) / fFade;
	}
	// solve for inverse square falloff
	else{
		fEnvelope = sqrt( fPhotons / fFalloffTolerance ) /* + fRadius */ ; // Arnout radius is always 0, only for area lights

	}
	return fEnvelope;
}

float CalculateLightRadius( entity_t * e, bool outer ){
	float fEnvelope = 0.f;
	int iSpawnFlags = atoi( ValueForKey( e, "spawnflags" ) );
	float fIntensity;
	float fScale;
	const char *gameFile = g_FuncTable.m_pfnGetGameFile();

	fIntensity = FloatForKey( e, "light" );
	if ( fIntensity == 0.f ) {
		fIntensity = 300.f;
	}

	// Arnout: HACK for per-game radii - really need to move this to a per-game module
	if ( !strcmp( gameFile, "sof2.game" ) || !strcmp( gameFile, "jk2.game" ) || !strcmp( gameFile, "ja.game" ) ) {
		// Spawnflags :
		// 1: linear
		// 2: noincidence

		if ( !outer ) {
			if ( iSpawnFlags & 2 ) {
				fIntensity *= .9f;
			}
			else{
				fIntensity *= .25f;
			}
		}
		// set light scale (sof2)
		fScale = FloatForKey( e, "scale" );
		if ( fScale <= 0.f ) {
			fScale = 1.f;
		}
		fIntensity *= fScale;

		fEnvelope = fIntensity;
	}
	else {
		float fPointScale = 7500.f;

		if ( outer ) {
			fEnvelope = sqrt( fIntensity * fPointScale / 48.f );
		}
		else{
			fEnvelope = sqrt( fIntensity * fPointScale / 255.f );
		}
	}

	return fEnvelope;
}

void Light_OnIntensityChanged( entity_t* e ){
	e->fLightEnvelope1[0] = CalculateEnvelopeForLight( e, 1.f );
	e->fLightEnvelope1[1] = CalculateEnvelopeForLight( e, 48.f );
	e->fLightEnvelope1[2] = CalculateEnvelopeForLight( e, 255.f );

	e->fLightEnvelope2[0] = CalculateLightRadius( e, TRUE );
	e->fLightEnvelope2[1] = CalculateLightRadius( e, FALSE );
}

void Light_OnKeyValueChanged( entity_t *e, const char *key, const char* value ){
	if ( strcmp( key,"_color" ) == 0 ) {
		if ( sscanf( ValueForKey( e, "_color" ),"%f %f %f",
					 &e->color[0], &e->color[1], &e->color[2] ) != 3 ) {
			VectorSet( e->color, 1, 1, 1 );
		}
	}
	else if ( strcmp( key,"spawnflags" ) == 0 ||
			  strcmp( key,"fade" ) == 0 ||
			  strcmp( key,"_light" ) == 0 ||
			  strcmp( key,"light" ) == 0 ||
			  strcmp( key,"scale" ) == 0 ) {
		Light_OnIntensityChanged( e );
	}
}

bool Entity_IsLight( entity_t *e ){
	return e->eclass != NULL && e->eclass->nShowFlags & ECLASS_LIGHT; //strncmp(ValueforKey(e, "classname"), "light") == 0
}

static void DrawLightSphere( entity_t * e, int nGLState, int pref ){
	const char *target = ValueForKey( e, "target" );
	bool bIsSpotLight = !!target[0];
	//!\todo Write an API for modules to register preference settings, and make this preference module-specific.
	// int nPasses = pref == 1 ? 3 : 2;

	g_QglTable.m_pfn_qglPushAttrib( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	g_QglTable.m_pfn_qglDepthMask( GL_FALSE );
	g_QglTable.m_pfn_qglEnable( GL_BLEND );
	g_QglTable.m_pfn_qglBlendFunc( GL_ONE, GL_ONE );

	// Arnout: TODO: spotlight rendering
	if ( !( bIsSpotLight ) ) {
		switch ( pref )
		{
		case 1:
			g_QglTable.m_pfn_qglColor3f( e->color[0] * .05f,
										 e->color[1] * .05f,
										 e->color[2] * .05f );
			DrawSphere( e->origin, e->fLightEnvelope1[0], 16, nGLState );
			DrawSphere( e->origin, e->fLightEnvelope1[1], 16, nGLState );
			DrawSphere( e->origin, e->fLightEnvelope1[2], 16, nGLState );
			break;
		case 2:
			g_QglTable.m_pfn_qglColor3f( e->color[0] * .15f * .95f,
										 e->color[1] * .15f * .95f,
										 e->color[2] * .15f * .95f );
			DrawSphere( e->origin, e->fLightEnvelope2[0], 16, nGLState );
			DrawSphere( e->origin, e->fLightEnvelope2[1], 16, nGLState );
			break;

		}
	}

	g_QglTable.m_pfn_qglPopAttrib();
}

float F = 0.70710678f;
// North, East, South, West
vec3_t normals[8] = { { 0, F, F }, { F, 0, F }, { 0,-F, F }, {-F, 0, F },
					  { 0, F,-F }, { F, 0,-F }, { 0,-F,-F }, {-F, 0,-F } };

unsigned short indices[24] = { 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 2,
							   1, 2, 5, 1, 5, 4, 1, 4, 3, 1, 3, 2 };

void DrawLight( entity_t* e, int nGLState, int pref, int nViewType ){
//  int i;
	// top, bottom, tleft, tright, bright, bleft
	vec3_t points[6];
	vec3_t vMid, vMin, vMax;
	VectorAdd( e->origin, e->eclass->mins, vMin );
	VectorAdd( e->origin, e->eclass->maxs, vMax );
	vMid[0] = ( vMin[0] + vMax[0] ) * 0.5;
	vMid[1] = ( vMin[1] + vMax[1] ) * 0.5;
	vMid[2] = ( vMin[2] + vMax[2] ) * 0.5;

	VectorSet( points[0], vMid[0], vMid[1], vMax[2] );
	VectorSet( points[1], vMid[0], vMid[1], vMin[2] );
	VectorSet( points[2], vMin[0], vMax[1], vMid[2] );
	VectorSet( points[3], vMax[0], vMax[1], vMid[2] );
	VectorSet( points[4], vMax[0], vMin[1], vMid[2] );
	VectorSet( points[5], vMin[0], vMin[1], vMid[2] );

	if ( nGLState & DRAW_GL_LIGHTING ) { // && g_PrefsDlg.m_bGLLighting)
		g_QglTable.m_pfn_qglBegin( GL_TRIANGLES ); // NOTE: comment to use gl_triangle_fan instead
		//g_QglTable.m_pfn_qglBegin(GL_TRIANGLE_FAN);
		g_QglTable.m_pfn_qglVertex3fv( points[0] );
		g_QglTable.m_pfn_qglVertex3fv( points[2] );
		g_QglTable.m_pfn_qglNormal3fv( normals[0] );
		g_QglTable.m_pfn_qglVertex3fv( points[3] );

		g_QglTable.m_pfn_qglVertex3fv( points[0] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[3] ); //
		g_QglTable.m_pfn_qglNormal3fv( normals[1] );
		g_QglTable.m_pfn_qglVertex3fv( points[4] );

		g_QglTable.m_pfn_qglVertex3fv( points[0] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[4] ); //
		g_QglTable.m_pfn_qglNormal3fv( normals[2] );
		g_QglTable.m_pfn_qglVertex3fv( points[5] );

		g_QglTable.m_pfn_qglVertex3fv( points[0] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[5] ); //
		g_QglTable.m_pfn_qglNormal3fv( normals[3] );
		g_QglTable.m_pfn_qglVertex3fv( points[2] );

		//g_QglTable.m_pfn_qglEnd();
		//g_QglTable.m_pfn_qglBegin(GL_TRIANGLE_FAN);

		g_QglTable.m_pfn_qglVertex3fv( points[1] );
		g_QglTable.m_pfn_qglVertex3fv( points[2] );
		g_QglTable.m_pfn_qglNormal3fv( normals[7] );
		g_QglTable.m_pfn_qglVertex3fv( points[5] );

		g_QglTable.m_pfn_qglVertex3fv( points[1] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[5] ); //
		g_QglTable.m_pfn_qglNormal3fv( normals[6] );
		g_QglTable.m_pfn_qglVertex3fv( points[4] );

		g_QglTable.m_pfn_qglVertex3fv( points[1] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[4] ); //
		g_QglTable.m_pfn_qglNormal3fv( normals[5] );
		g_QglTable.m_pfn_qglVertex3fv( points[3] );

		g_QglTable.m_pfn_qglVertex3fv( points[1] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[3] ); //
		g_QglTable.m_pfn_qglNormal3fv( normals[4] );
		g_QglTable.m_pfn_qglVertex3fv( points[2] );

		g_QglTable.m_pfn_qglEnd();
	}
	else if ( nGLState & DRAW_GL_FILL ) {
		vec3_t colors[4];
		VectorScale( e->color, 0.95, colors[0] );
		VectorScale( colors[0], 0.95, colors[1] );
		VectorScale( colors[1], 0.95, colors[2] );
		VectorScale( colors[2], 0.95, colors[3] );
		g_QglTable.m_pfn_qglBegin( GL_TRIANGLES ); // NOTE: comment to use gl_triangle_fan instead
		//g_QglTable.m_pfn_qglBegin(GL_TRIANGLE_FAN);
		g_QglTable.m_pfn_qglColor3fv( colors[0] );
		g_QglTable.m_pfn_qglVertex3fv( points[0] );
		g_QglTable.m_pfn_qglVertex3fv( points[2] );
		g_QglTable.m_pfn_qglVertex3fv( points[3] );

		g_QglTable.m_pfn_qglColor3fv( colors[1] );
		g_QglTable.m_pfn_qglVertex3fv( points[0] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[3] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[4] );

		g_QglTable.m_pfn_qglColor3fv( colors[2] );
		g_QglTable.m_pfn_qglVertex3fv( points[0] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[4] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[5] );

		g_QglTable.m_pfn_qglColor3fv( colors[3] );
		g_QglTable.m_pfn_qglVertex3fv( points[0] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[5] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[2] );

		//g_QglTable.m_pfn_qglEnd();
		//g_QglTable.m_pfn_qglBegin(GL_TRIANGLE_FAN);

		g_QglTable.m_pfn_qglColor3fv( colors[0] );
		g_QglTable.m_pfn_qglVertex3fv( points[1] );
		g_QglTable.m_pfn_qglVertex3fv( points[2] );
		g_QglTable.m_pfn_qglVertex3fv( points[5] );

		g_QglTable.m_pfn_qglColor3fv( colors[1] );
		g_QglTable.m_pfn_qglVertex3fv( points[1] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[5] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[4] );

		g_QglTable.m_pfn_qglColor3fv( colors[2] );
		g_QglTable.m_pfn_qglVertex3fv( points[1] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[4] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[3] );

		g_QglTable.m_pfn_qglColor3fv( colors[3] );
		g_QglTable.m_pfn_qglVertex3fv( points[1] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[3] ); //
		g_QglTable.m_pfn_qglVertex3fv( points[2] );

		g_QglTable.m_pfn_qglEnd();
	}
	else
	{
		g_QglTable.m_pfn_qglVertexPointer( 3, GL_FLOAT, 0, points );
		g_QglTable.m_pfn_qglDrawElements( GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, indices );
	}


	// NOTE: prolly not relevant until some time..
	// check for DOOM lights
	if ( strlen( ValueForKey( e, "light_right" ) ) > 0 ) {
		vec3_t vRight, vUp, vTarget, vTemp;
		GetVectorForKey( e, "light_right", vRight );
		GetVectorForKey( e, "light_up", vUp );
		GetVectorForKey( e, "light_target", vTarget );

		g_QglTable.m_pfn_qglColor3f( 0, 1, 0 );
		g_QglTable.m_pfn_qglBegin( GL_LINE_LOOP );
		VectorAdd( vTarget, e->origin, vTemp );
		VectorAdd( vTemp, vRight, vTemp );
		VectorAdd( vTemp, vUp, vTemp );
		g_QglTable.m_pfn_qglVertex3fv( e->origin );
		g_QglTable.m_pfn_qglVertex3fv( vTemp );
		VectorAdd( vTarget, e->origin, vTemp );
		VectorAdd( vTemp, vUp, vTemp );
		VectorSubtract( vTemp, vRight, vTemp );
		g_QglTable.m_pfn_qglVertex3fv( e->origin );
		g_QglTable.m_pfn_qglVertex3fv( vTemp );
		VectorAdd( vTarget, e->origin, vTemp );
		VectorAdd( vTemp, vRight, vTemp );
		VectorSubtract( vTemp, vUp, vTemp );
		g_QglTable.m_pfn_qglVertex3fv( e->origin );
		g_QglTable.m_pfn_qglVertex3fv( vTemp );
		VectorAdd( vTarget, e->origin, vTemp );
		VectorSubtract( vTemp, vUp, vTemp );
		VectorSubtract( vTemp, vRight, vTemp );
		g_QglTable.m_pfn_qglVertex3fv( e->origin );
		g_QglTable.m_pfn_qglVertex3fv( vTemp );
		g_QglTable.m_pfn_qglEnd();

	}

	if ( nGLState & DRAW_GL_FILL ) {
		DrawLightSphere( e, nGLState, pref );
	}
	else
	{
		// Arnout: FIXME: clean this up a bit
		// now draw lighting radius stuff...
		if ( pref ) {
			bool bDrawSpotlightArc = false;
			int nPasses = pref == 1 ? 3 : 2;

			const char *target = ValueForKey( e, "target" );
			bool bIsSpotLight = !!target[0];

			/*!\todo Spotlight..
			   if (bIsSpotLight)
			   {
			   // find the origin of the target...
			   entity_t *e = FindEntity("targetname", target);

			   if (e)
			    bDrawSpotlightArc = true;
			   }
			 */

			g_QglTable.m_pfn_qglPushAttrib( GL_LINE_BIT );
			g_QglTable.m_pfn_qglLineStipple( 8, 0xAAAA );
			g_QglTable.m_pfn_qglEnable( GL_LINE_STIPPLE );

			float* envelope = ( pref == 1 ) ? e->fLightEnvelope1 : e->fLightEnvelope2;
			for ( int iPass = 0; iPass < nPasses; iPass++ )
			{
				float fRadius = envelope[iPass];

				g_QglTable.m_pfn_qglBegin( GL_LINE_LOOP );

				if ( bIsSpotLight ) {
					if ( bDrawSpotlightArc ) {
						// I give up on this, it's beyond me
					}
				}
				else
				{
					if ( fRadius > 0 ) {
						int i;
						float ds, dc;

						for ( i = 0; i <= 24; i++ )
						{
							ds = sin( ( i * 2 * Q_PI ) / 24 );
							dc = cos( ( i * 2 * Q_PI ) / 24 );

							switch ( nViewType )
							{
							case 2:
								g_QglTable.m_pfn_qglVertex3f( e->origin[0] + fRadius * dc,
															  e->origin[1] + fRadius * ds,
															  e->origin[2] );
								break;
							case 1:
								g_QglTable.m_pfn_qglVertex3f( e->origin[0] + fRadius * dc,
															  e->origin[1],
															  e->origin[2] + fRadius * ds );
								break;
							case 0:
								g_QglTable.m_pfn_qglVertex3f( e->origin[0],
															  e->origin[1] + fRadius * dc,
															  e->origin[2] + fRadius * ds );
								break;
							}
						}
					}
				}
				g_QglTable.m_pfn_qglEnd();
			}
			g_QglTable.m_pfn_qglPopAttrib();
		}
	}
}
