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

struct BuildStairsRS {
	char mainTexture[256];
	char riserTexture[256];
	int direction;
	int style;
	int stairHeight;
	qboolean bUseDetail;
};

struct ResetTextureRS {
	int bResetTextureName;
	char textureName[256];
	char newTextureName[256];

	int bResetScale[2];
	float fScale[2];

	int bResetShift[2];
	float fShift[2];

	int bResetRotation;
	int rotation;
};

struct TrainThingRS {
	float fRadiusX, fRadiusY;
	float fStartAngle, fEndAngle;
	int iNumPoints;
	float fStartHeight, fEndHeight;
};

struct IntersectRS {
	int nBrushOptions;
	qboolean bUseDetail;
	qboolean bDuplicateOnly;
};

struct PolygonRS {
	qboolean bUseBorder;
	qboolean bInverse;
	qboolean bAlignTop;
	int nSides;
	int nBorderWidth;
};

struct DoorRS {
	char mainTexture[256];
	char trimTexture[256];
	qboolean bScaleMainH;
	qboolean bScaleMainV;
	qboolean bScaleTrimH;
	qboolean bScaleTrimV;
	int nOrientation;
};

struct PathPlotterRS {
	int nPoints;
	float fMultiplier;
	float fGravity;
	qboolean bNoUpdate;
	qboolean bShowExtra;
};

struct TwinWidget {
	GtkWidget* one;
	GtkWidget* two;
};

int DoMessageBox( const char* lpText, const char* lpCaption, guint32 uType );
int DoIntersectBox( IntersectRS* rs );
int DoPolygonBox( PolygonRS* rs );
int DoResetTextureBox( ResetTextureRS* rs );
int DoBuildStairsBox( BuildStairsRS* rs );
int DoDoorsBox( DoorRS* rs );
int DoPathPlotterBox( PathPlotterRS* rs );
int DoCTFColourChangeBox();
int DoTrainThingBox( TrainThingRS* rs );

//GtkWidget* GetProgressWindow(char* title, GtkProgressBar* feedback);
