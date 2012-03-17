#ifndef _SURFACEPLUGIN_H
#define _SURFACEPLUGIN_H

int SI_GetSelectedFaceCountfromBrushes( void );
void SI_GetSelFacesTexdef( texdef_to_face_t *allocd_block_texdef );
void SI_SetTexdef_FaceList( texdef_to_face_t* texdef_face_list, bool b_SetUndoPoint = FALSE, bool bFit_to_Scale = FALSE );
void SI_FaceList_FitTexture( texdef_to_face_t* si_texdef_face_list, int nHeight, int nWidth );
GtkWindow* SI_GetMainWindow( void );
void SI_SetWinPos_from_Prefs( GtkWidget *win );

#endif // _SURFACEPLUGIN_H
