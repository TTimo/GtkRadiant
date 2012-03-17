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

#define DLG_PLANE_XY0                           100
#define DLG_PLANE_XY1                           101
#define DLG_PLANE_YZ0                           102
#define DLG_PLANE_XZ0                           103
#define DLG_PLANE_YZ1                           104
#define DLG_PLANE_XZ1                           105
#define DLG_WAVE_01                             106
#define DLG_WAVE_02                             107
#define DLG_WAVE_03                             108
#define DLG_WAVE_04                             109
#define DLG_WAVE_05                             110
#define DLG_WAVE_06                             111
#define DLG_LAMBDA                              112
#define DLG_LAMBDA_TEXT                     113
#define DLG_AMP                                     114
#define DLG_AMP_TEXT                            115
#define DLG_ROUGH                                   116
#define DLG_ROUGH_TEXT                      117
#define DLG_LINEARBORDER                    118
#define DLG_FILE                                    119
#define DLG_FILE_BROWSE                     120
#define DLG_PREVIEW                             121
#define DLG_GO                                      122
#define DLG_ABOUT                                   123
#define DLG_NH_TEXT                             124
#define DLG_NH                                      125
#define DLG_NH_SPIN                             126
#define DLG_NV_TEXT                             127
#define DLG_NV                                      128
#define DLG_NV_SPIN                             129
#define DLG_HMIN_TEXT                           130
#define DLG_HMIN                                    131
#define DLG_HMAX_TEXT                           132
#define DLG_HMAX                                    133
#define DLG_VMIN_TEXT                           134
#define DLG_VMIN                                    135
#define DLG_VMAX_TEXT                           136
#define DLG_VMAX                                    137
#define DLG_Z00_TEXT                            138
#define DLG_Z00                                     139
#define DLG_Z01_TEXT                            140
#define DLG_Z01                                     141
#define DLG_Z10_TEXT                            142
#define DLG_Z10                                     143
#define DLG_Z11_TEXT                            144
#define DLG_Z11                                     145
#define DLG_TEXTURE                             146
#define DLG_SKYBOX                              147
#define DLG_AUTOOVERWRITE                   148
#define DLG_DETAIL                              149
#define DLG_ARGHRAD2                            150
#define DLG_ARGHRAD2_SPIN                   151
#define DLG_APPEND                              152
#define DLG_REFRESH                             153
#define DLG_TEXOFFSETX                      154
#define DLG_TEXOFFSETY                      155
#define DLG_TEXSCALEX                           156
#define DLG_TEXSCALEY                           157
#define DLG_FIXPOINTS                           158
#define DLG_TEXTURE_BROWSE              159
#define DLG_AZIMUTH                             162
#define DLG_AZIMUTH_SPIN                    163
#define DLG_ELEVATION                           164
#define DLG_ELEVATION_SPIN              165
#define DLG_RANDOMSEED                      166
#define DLG_RANDOMSEED_SPIN             167
#define DLG_BITMAP                              168
#define DLG_SAVE                                    169
#define DLG_OPEN                                    170
#define DLG_TAB                                     171
#define DLG_TEXTURE2                            172
#define DLG_TEXTURE2_BROWSE             173
#define DLG_LADDER                              174
#define DLG_ARGHRAD2_TEXT                   175
#define DLG_FILE_TEXT                           176
#define DLG_DECIMATE                            177
#define DLG_DECIMATE_TEXT                   178
#define DLG_HIDEBACKFACES                   179
#define DLG_DEFAULTS                            180
#define DLG_ABOUT_APP                           200
#define DLG_ABOUT_ICON                      201
#define DLG_BMP_FILE                            202
#define DLG_BMP_FILE_BROWSE             203
#define DLG_BMP_BLACK                           204
#define DLG_BMP_WHITE                           205
#define DLG_BMP_TEXT1                           206
#define DLG_BMP_TEXT2                           207
#define DLG_BMP_TEXT3                           208
#define DLG_BMP_NOTE                            209
#define DLG_BMP_RELOAD                      210
#define DLG_ABOUT_URL                           211
#define DLG_ABOUT_BOARD                     212
#define DLG_FIX_FREE                            300
#define DLG_FIX_FREEALL                     301
#define DLG_FIX_VALUE_TEXT              302
#define DLG_FIX_VALUE                           303
#define DLG_FIX_VALUE_SPIN              304
#define DLG_FIX_DONE                            305
#define DLG_FIX_RANGE_TEXT              306
#define DLG_FIX_RANGE                           307
#define DLG_FIX_NOTE                            308
#define DLG_FIX_RATE_TEXT                   309
#define DLG_FIX_RATE                            310
#define DLG_USE_PATCHES                     311
#define DLG_DECIMATE_LABEL              312
#define DLG_HINT                                    350
#define DLG_GAME_00                             400
#define DLG_GAME_01                             401
#define DLG_GAME_02                             402
#define DLG_GAME_03                             403
#define DLG_GAME_04                             404
#define DLG_GAME_05                             405
#define DLG_GAME_06                             406
#define DLG_GAME_07                             407
#define DLG_GAME_08                             408
#define DLG_GAME_09                             409
#define DLG_TEX_USEPAK                      420
#define DLG_TEX_PAK_TEXT                    421
#define DLG_TEX_PAKFILE                     422
#define DLG_TEX_PAK_BROWSE              423
#define DLG_TEX_LIST1                           424
#define DLG_TEX_LIST2                           425
#define DLG_TEX_LIST3                           426
#define DLG_TEXTURE3                            427
#define DLG_TEXTURE3_BROWSE             428
#define DLG_TEX_SLANT_TEXT              429
#define DLG_TEX_SLANT                           430
#define DLG_TEX_SLANT_SPIN            431
#define DLG_EXCEL_FUNC                      500
#define DLG_EXCEL_FUNC_TEXT             501
#define DLG_PREVIEW_ANTIALIASING    502 // ^Fishman - Antializing for the preview window.
#define DLG_SNAP_TO_GRID                    503 // Hydra : snap to grid
