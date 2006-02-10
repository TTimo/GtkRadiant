/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

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

#if !defined(INCLUDED_IGL_H)
#define INCLUDED_IGL_H

#include <cstddef>
#include <string.h>
#include "generic/constant.h"

#if defined(WIN32)
#define QGL_DLLEXPORT __stdcall
#else
#define QGL_DLLEXPORT
#endif

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;

#if !defined(GL_VERSION_1_1)
#define GL_VERSION_1_1 1

#define GL_ZERO 0
#define GL_ONE 1
#define GL_TRUE 1
#define GL_FALSE 0
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_QUADS 0x0007
#define GL_QUAD_STRIP 0x0008
#define GL_POLYGON 0x0009
#define GL_ACCUM 0x0100
#define GL_LOAD 0x0101
#define GL_RETURN 0x0102
#define GL_MULT 0x0103
#define GL_ADD 0x0104
#define GL_NEVER 0x0200
#define GL_LESS 0x0201
#define GL_EQUAL 0x0202
#define GL_LEQUAL 0x0203
#define GL_GREATER 0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL 0x0206
#define GL_ALWAYS 0x0207
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_2_BYTES 0x1407
#define GL_3_BYTES 0x1408
#define GL_4_BYTES 0x1409
#define GL_DOUBLE 0x140A
#define GL_NONE 0
#define GL_FRONT_LEFT 0x0400
#define GL_FRONT_RIGHT 0x0401
#define GL_BACK_LEFT 0x0402
#define GL_BACK_RIGHT 0x0403
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_LEFT 0x0406
#define GL_RIGHT 0x0407
#define GL_FRONT_AND_BACK 0x0408
#define GL_AUX0 0x0409
#define GL_AUX1 0x040A
#define GL_AUX2 0x040B
#define GL_AUX3 0x040C
#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_STACK_OVERFLOW 0x0503
#define GL_STACK_UNDERFLOW 0x0504
#define GL_OUT_OF_MEMORY 0x0505
#define GL_2D 0x0600
#define GL_3D 0x0601
#define GL_3D_COLOR 0x0602
#define GL_3D_COLOR_TEXTURE 0x0603
#define GL_4D_COLOR_TEXTURE 0x0604
#define GL_PASS_THROUGH_TOKEN 0x0700
#define GL_POINT_TOKEN 0x0701
#define GL_LINE_TOKEN 0x0702
#define GL_POLYGON_TOKEN 0x0703
#define GL_BITMAP_TOKEN 0x0704
#define GL_DRAW_PIXEL_TOKEN 0x0705
#define GL_COPY_PIXEL_TOKEN 0x0706
#define GL_LINE_RESET_TOKEN 0x0707
#define GL_EXP 0x0800
#define GL_EXP2 0x0801
#define GL_CW 0x0900
#define GL_CCW 0x0901
#define GL_COEFF 0x0A00
#define GL_ORDER 0x0A01
#define GL_DOMAIN 0x0A02
#define GL_CURRENT_COLOR 0x0B00
#define GL_CURRENT_INDEX 0x0B01
#define GL_CURRENT_NORMAL 0x0B02
#define GL_CURRENT_TEXTURE_COORDS 0x0B03
#define GL_CURRENT_RASTER_COLOR 0x0B04
#define GL_CURRENT_RASTER_INDEX 0x0B05
#define GL_CURRENT_RASTER_TEXTURE_COORDS 0x0B06
#define GL_CURRENT_RASTER_POSITION 0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID 0x0B08
#define GL_CURRENT_RASTER_DISTANCE 0x0B09
#define GL_POINT_SMOOTH 0x0B10
#define GL_POINT_SIZE 0x0B11
#define GL_POINT_SIZE_RANGE 0x0B12
#define GL_POINT_SIZE_GRANULARITY 0x0B13
#define GL_LINE_SMOOTH 0x0B20
#define GL_LINE_WIDTH 0x0B21
#define GL_LINE_WIDTH_RANGE 0x0B22
#define GL_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_LINE_STIPPLE 0x0B24
#define GL_LINE_STIPPLE_PATTERN 0x0B25
#define GL_LINE_STIPPLE_REPEAT 0x0B26
#define GL_LIST_MODE 0x0B30
#define GL_MAX_LIST_NESTING 0x0B31
#define GL_LIST_BASE 0x0B32
#define GL_LIST_INDEX 0x0B33
#define GL_POLYGON_MODE 0x0B40
#define GL_POLYGON_SMOOTH 0x0B41
#define GL_POLYGON_STIPPLE 0x0B42
#define GL_EDGE_FLAG 0x0B43
#define GL_CULL_FACE 0x0B44
#define GL_CULL_FACE_MODE 0x0B45
#define GL_FRONT_FACE 0x0B46
#define GL_LIGHTING 0x0B50
#define GL_LIGHT_MODEL_LOCAL_VIEWER 0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE 0x0B52
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#define GL_SHADE_MODEL 0x0B54
#define GL_COLOR_MATERIAL_FACE 0x0B55
#define GL_COLOR_MATERIAL_PARAMETER 0x0B56
#define GL_COLOR_MATERIAL 0x0B57
#define GL_FOG 0x0B60
#define GL_FOG_INDEX 0x0B61
#define GL_FOG_DENSITY 0x0B62
#define GL_FOG_START 0x0B63
#define GL_FOG_END 0x0B64
#define GL_FOG_MODE 0x0B65
#define GL_FOG_COLOR 0x0B66
#define GL_DEPTH_RANGE 0x0B70
#define GL_DEPTH_TEST 0x0B71
#define GL_DEPTH_WRITEMASK 0x0B72
#define GL_DEPTH_CLEAR_VALUE 0x0B73
#define GL_DEPTH_FUNC 0x0B74
#define GL_ACCUM_CLEAR_VALUE 0x0B80
#define GL_STENCIL_TEST 0x0B90
#define GL_STENCIL_CLEAR_VALUE 0x0B91
#define GL_STENCIL_FUNC 0x0B92
#define GL_STENCIL_VALUE_MASK 0x0B93
#define GL_STENCIL_FAIL 0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL 0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS 0x0B96
#define GL_STENCIL_REF 0x0B97
#define GL_STENCIL_WRITEMASK 0x0B98
#define GL_MATRIX_MODE 0x0BA0
#define GL_NORMALIZE 0x0BA1
#define GL_VIEWPORT 0x0BA2
#define GL_MODELVIEW_STACK_DEPTH 0x0BA3
#define GL_PROJECTION_STACK_DEPTH 0x0BA4
#define GL_TEXTURE_STACK_DEPTH 0x0BA5
#define GL_MODELVIEW_MATRIX 0x0BA6
#define GL_PROJECTION_MATRIX 0x0BA7
#define GL_TEXTURE_MATRIX 0x0BA8
#define GL_ATTRIB_STACK_DEPTH 0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH 0x0BB1
#define GL_ALPHA_TEST 0x0BC0
#define GL_ALPHA_TEST_FUNC 0x0BC1
#define GL_ALPHA_TEST_REF 0x0BC2
#define GL_DITHER 0x0BD0
#define GL_BLEND_DST 0x0BE0
#define GL_BLEND_SRC 0x0BE1
#define GL_BLEND 0x0BE2
#define GL_LOGIC_OP_MODE 0x0BF0
#define GL_INDEX_LOGIC_OP 0x0BF1
#define GL_COLOR_LOGIC_OP 0x0BF2
#define GL_AUX_BUFFERS 0x0C00
#define GL_DRAW_BUFFER 0x0C01
#define GL_READ_BUFFER 0x0C02
#define GL_SCISSOR_BOX 0x0C10
#define GL_SCISSOR_TEST 0x0C11
#define GL_INDEX_CLEAR_VALUE 0x0C20
#define GL_INDEX_WRITEMASK 0x0C21
#define GL_COLOR_CLEAR_VALUE 0x0C22
#define GL_COLOR_WRITEMASK 0x0C23
#define GL_INDEX_MODE 0x0C30
#define GL_RGBA_MODE 0x0C31
#define GL_DOUBLEBUFFER 0x0C32
#define GL_STEREO 0x0C33
#define GL_RENDER_MODE 0x0C40
#define GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#define GL_POINT_SMOOTH_HINT 0x0C51
#define GL_LINE_SMOOTH_HINT 0x0C52
#define GL_POLYGON_SMOOTH_HINT 0x0C53
#define GL_FOG_HINT 0x0C54
#define GL_TEXTURE_GEN_S 0x0C60
#define GL_TEXTURE_GEN_T 0x0C61
#define GL_TEXTURE_GEN_R 0x0C62
#define GL_TEXTURE_GEN_Q 0x0C63
#define GL_PIXEL_MAP_I_TO_I 0x0C70
#define GL_PIXEL_MAP_S_TO_S 0x0C71
#define GL_PIXEL_MAP_I_TO_R 0x0C72
#define GL_PIXEL_MAP_I_TO_G 0x0C73
#define GL_PIXEL_MAP_I_TO_B 0x0C74
#define GL_PIXEL_MAP_I_TO_A 0x0C75
#define GL_PIXEL_MAP_R_TO_R 0x0C76
#define GL_PIXEL_MAP_G_TO_G 0x0C77
#define GL_PIXEL_MAP_B_TO_B 0x0C78
#define GL_PIXEL_MAP_A_TO_A 0x0C79
#define GL_PIXEL_MAP_I_TO_I_SIZE 0x0CB0
#define GL_PIXEL_MAP_S_TO_S_SIZE 0x0CB1
#define GL_PIXEL_MAP_I_TO_R_SIZE 0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE 0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE 0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE 0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE 0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE 0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE 0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE 0x0CB9
#define GL_UNPACK_SWAP_BYTES 0x0CF0
#define GL_UNPACK_LSB_FIRST 0x0CF1
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_PACK_SWAP_BYTES 0x0D00
#define GL_PACK_LSB_FIRST 0x0D01
#define GL_PACK_ROW_LENGTH 0x0D02
#define GL_PACK_SKIP_ROWS 0x0D03
#define GL_PACK_SKIP_PIXELS 0x0D04
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_MAP_COLOR 0x0D10
#define GL_MAP_STENCIL 0x0D11
#define GL_INDEX_SHIFT 0x0D12
#define GL_INDEX_OFFSET 0x0D13
#define GL_RED_SCALE 0x0D14
#define GL_RED_BIAS 0x0D15
#define GL_ZOOM_X 0x0D16
#define GL_ZOOM_Y 0x0D17
#define GL_GREEN_SCALE 0x0D18
#define GL_GREEN_BIAS 0x0D19
#define GL_BLUE_SCALE 0x0D1A
#define GL_BLUE_BIAS 0x0D1B
#define GL_ALPHA_SCALE 0x0D1C
#define GL_ALPHA_BIAS 0x0D1D
#define GL_DEPTH_SCALE 0x0D1E
#define GL_DEPTH_BIAS 0x0D1F
#define GL_MAX_EVAL_ORDER 0x0D30
#define GL_MAX_LIGHTS 0x0D31
#define GL_MAX_CLIP_PLANES 0x0D32
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_MAX_PIXEL_MAP_TABLE 0x0D34
#define GL_MAX_ATTRIB_STACK_DEPTH 0x0D35
#define GL_MAX_MODELVIEW_STACK_DEPTH 0x0D36
#define GL_MAX_NAME_STACK_DEPTH 0x0D37
#define GL_MAX_PROJECTION_STACK_DEPTH 0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH 0x0D39
#define GL_MAX_VIEWPORT_DIMS 0x0D3A
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH 0x0D3B
#define GL_SUBPIXEL_BITS 0x0D50
#define GL_INDEX_BITS 0x0D51
#define GL_RED_BITS 0x0D52
#define GL_GREEN_BITS 0x0D53
#define GL_BLUE_BITS 0x0D54
#define GL_ALPHA_BITS 0x0D55
#define GL_DEPTH_BITS 0x0D56
#define GL_STENCIL_BITS 0x0D57
#define GL_ACCUM_RED_BITS 0x0D58
#define GL_ACCUM_GREEN_BITS 0x0D59
#define GL_ACCUM_BLUE_BITS 0x0D5A
#define GL_ACCUM_ALPHA_BITS 0x0D5B
#define GL_NAME_STACK_DEPTH 0x0D70
#define GL_AUTO_NORMAL 0x0D80
#define GL_MAP1_COLOR_4 0x0D90
#define GL_MAP1_INDEX 0x0D91
#define GL_MAP1_NORMAL 0x0D92
#define GL_MAP1_TEXTURE_COORD_1 0x0D93
#define GL_MAP1_TEXTURE_COORD_2 0x0D94
#define GL_MAP1_TEXTURE_COORD_3 0x0D95
#define GL_MAP1_TEXTURE_COORD_4 0x0D96
#define GL_MAP1_VERTEX_3 0x0D97
#define GL_MAP1_VERTEX_4 0x0D98
#define GL_MAP2_COLOR_4 0x0DB0
#define GL_MAP2_INDEX 0x0DB1
#define GL_MAP2_NORMAL 0x0DB2
#define GL_MAP2_TEXTURE_COORD_1 0x0DB3
#define GL_MAP2_TEXTURE_COORD_2 0x0DB4
#define GL_MAP2_TEXTURE_COORD_3 0x0DB5
#define GL_MAP2_TEXTURE_COORD_4 0x0DB6
#define GL_MAP2_VERTEX_3 0x0DB7
#define GL_MAP2_VERTEX_4 0x0DB8
#define GL_MAP1_GRID_DOMAIN 0x0DD0
#define GL_MAP1_GRID_SEGMENTS 0x0DD1
#define GL_MAP2_GRID_DOMAIN 0x0DD2
#define GL_MAP2_GRID_SEGMENTS 0x0DD3
#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1
#define GL_FEEDBACK_BUFFER_POINTER 0x0DF0
#define GL_FEEDBACK_BUFFER_SIZE 0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE 0x0DF2
#define GL_SELECTION_BUFFER_POINTER 0x0DF3
#define GL_SELECTION_BUFFER_SIZE 0x0DF4
#define GL_TEXTURE_WIDTH 0x1000
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_TEXTURE_INTERNAL_FORMAT 0x1003
#define GL_TEXTURE_BORDER_COLOR 0x1004
#define GL_TEXTURE_BORDER 0x1005
#define GL_DONT_CARE 0x1100
#define GL_FASTEST 0x1101
#define GL_NICEST 0x1102
#define GL_LIGHT0 0x4000
#define GL_LIGHT1 0x4001
#define GL_LIGHT2 0x4002
#define GL_LIGHT3 0x4003
#define GL_LIGHT4 0x4004
#define GL_LIGHT5 0x4005
#define GL_LIGHT6 0x4006
#define GL_LIGHT7 0x4007
#define GL_AMBIENT 0x1200
#define GL_DIFFUSE 0x1201
#define GL_SPECULAR 0x1202
#define GL_POSITION 0x1203
#define GL_SPOT_DIRECTION 0x1204
#define GL_SPOT_EXPONENT 0x1205
#define GL_SPOT_CUTOFF 0x1206
#define GL_CONSTANT_ATTENUATION 0x1207
#define GL_LINEAR_ATTENUATION 0x1208
#define GL_QUADRATIC_ATTENUATION 0x1209
#define GL_COMPILE 0x1300
#define GL_COMPILE_AND_EXECUTE 0x1301
#define GL_CLEAR 0x1500
#define GL_AND 0x1501
#define GL_AND_REVERSE 0x1502
#define GL_COPY 0x1503
#define GL_AND_INVERTED 0x1504
#define GL_NOOP 0x1505
#define GL_XOR 0x1506
#define GL_OR 0x1507
#define GL_NOR 0x1508
#define GL_EQUIV 0x1509
#define GL_INVERT 0x150A
#define GL_OR_REVERSE 0x150B
#define GL_COPY_INVERTED 0x150C
#define GL_OR_INVERTED 0x150D
#define GL_NAND 0x150E
#define GL_SET 0x150F
#define GL_EMISSION 0x1600
#define GL_SHININESS 0x1601
#define GL_AMBIENT_AND_DIFFUSE 0x1602
#define GL_COLOR_INDEXES 0x1603
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_TEXTURE 0x1702
#define GL_COLOR 0x1800
#define GL_DEPTH 0x1801
#define GL_STENCIL 0x1802
#define GL_COLOR_INDEX 0x1900
#define GL_STENCIL_INDEX 0x1901
#define GL_DEPTH_COMPONENT 0x1902
#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_BITMAP 0x1A00
#define GL_POINT 0x1B00
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#define GL_RENDER 0x1C00
#define GL_FEEDBACK 0x1C01
#define GL_SELECT 0x1C02
#define GL_FLAT 0x1D00
#define GL_SMOOTH 0x1D01
#define GL_KEEP 0x1E00
#define GL_REPLACE 0x1E01
#define GL_INCR 0x1E02
#define GL_DECR 0x1E03
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_S 0x2000
#define GL_T 0x2001
#define GL_R 0x2002
#define GL_Q 0x2003
#define GL_MODULATE 0x2100
#define GL_DECAL 0x2101
#define GL_TEXTURE_ENV_MODE 0x2200
#define GL_TEXTURE_ENV_COLOR 0x2201
#define GL_TEXTURE_ENV 0x2300
#define GL_EYE_LINEAR 0x2400
#define GL_OBJECT_LINEAR 0x2401
#define GL_SPHERE_MAP 0x2402
#define GL_TEXTURE_GEN_MODE 0x2500
#define GL_OBJECT_PLANE 0x2501
#define GL_EYE_PLANE 0x2502
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP 0x2900
#define GL_REPEAT 0x2901
#define GL_CLIP_PLANE0 0x3000
#define GL_CLIP_PLANE1 0x3001
#define GL_CLIP_PLANE2 0x3002
#define GL_CLIP_PLANE3 0x3003
#define GL_CLIP_PLANE4 0x3004
#define GL_CLIP_PLANE5 0x3005
#define GL_CURRENT_BIT 0x00000001
#define GL_POINT_BIT 0x00000002
#define GL_LINE_BIT 0x00000004
#define GL_POLYGON_BIT 0x00000008
#define GL_POLYGON_STIPPLE_BIT 0x00000010
#define GL_PIXEL_MODE_BIT 0x00000020
#define GL_LIGHTING_BIT 0x00000040
#define GL_FOG_BIT 0x00000080
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_ACCUM_BUFFER_BIT 0x00000200
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_VIEWPORT_BIT 0x00000800
#define GL_TRANSFORM_BIT 0x00001000
#define GL_ENABLE_BIT 0x00002000
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_HINT_BIT 0x00008000
#define GL_EVAL_BIT 0x00010000
#define GL_LIST_BIT 0x00020000
#define GL_TEXTURE_BIT 0x00040000
#define GL_SCISSOR_BIT 0x00080000
#define GL_ALL_ATTRIB_BITS 0x000fffff
#define GL_CLIENT_PIXEL_STORE_BIT 0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT 0x00000002
#define GL_CLIENT_ALL_ATTRIB_BITS 0xffffffff
#define GL_POLYGON_OFFSET_UNITS 0x2A00
#define GL_POLYGON_OFFSET_POINT 0x2A01
#define GL_POLYGON_OFFSET_LINE 0x2A02
#define GL_POLYGON_OFFSET_FILL 0x8037
#define GL_POLYGON_OFFSET_FACTOR 0x8038
#define GL_ALPHA4 0x803B
#define GL_ALPHA8 0x803C
#define GL_ALPHA12 0x803D
#define GL_ALPHA16 0x803E
#define GL_LUMINANCE4 0x803F
#define GL_LUMINANCE8 0x8040
#define GL_LUMINANCE12 0x8041
#define GL_LUMINANCE16 0x8042
#define GL_LUMINANCE4_ALPHA4 0x8043
#define GL_LUMINANCE6_ALPHA2 0x8044
#define GL_LUMINANCE8_ALPHA8 0x8045
#define GL_LUMINANCE12_ALPHA4 0x8046
#define GL_LUMINANCE12_ALPHA12 0x8047
#define GL_LUMINANCE16_ALPHA16 0x8048
#define GL_INTENSITY 0x8049
#define GL_INTENSITY4 0x804A
#define GL_INTENSITY8 0x804B
#define GL_INTENSITY12 0x804C
#define GL_INTENSITY16 0x804D
#define GL_R3_G3_B2 0x2A10
#define GL_RGB4 0x804F
#define GL_RGB5 0x8050
#define GL_RGB8 0x8051
#define GL_RGB10 0x8052
#define GL_RGB12 0x8053
#define GL_RGB16 0x8054
#define GL_RGBA2 0x8055
#define GL_RGBA4 0x8056
#define GL_RGB5_A1 0x8057
#define GL_RGBA8 0x8058
#define GL_RGB10_A2 0x8059
#define GL_RGBA12 0x805A
#define GL_RGBA16 0x805B
#define GL_TEXTURE_RED_SIZE 0x805C
#define GL_TEXTURE_GREEN_SIZE 0x805D
#define GL_TEXTURE_BLUE_SIZE 0x805E
#define GL_TEXTURE_ALPHA_SIZE 0x805F
#define GL_TEXTURE_LUMINANCE_SIZE 0x8060
#define GL_TEXTURE_INTENSITY_SIZE 0x8061
#define GL_PROXY_TEXTURE_1D 0x8063
#define GL_PROXY_TEXTURE_2D 0x8064
#define GL_TEXTURE_PRIORITY 0x8066
#define GL_TEXTURE_RESIDENT 0x8067
#define GL_TEXTURE_BINDING_1D 0x8068
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_VERTEX_ARRAY 0x8074
#define GL_NORMAL_ARRAY 0x8075
#define GL_COLOR_ARRAY 0x8076
#define GL_INDEX_ARRAY 0x8077
#define GL_TEXTURE_COORD_ARRAY 0x8078
#define GL_EDGE_FLAG_ARRAY 0x8079
#define GL_VERTEX_ARRAY_SIZE 0x807A
#define GL_VERTEX_ARRAY_TYPE 0x807B
#define GL_VERTEX_ARRAY_STRIDE 0x807C
#define GL_NORMAL_ARRAY_TYPE 0x807E
#define GL_NORMAL_ARRAY_STRIDE 0x807F
#define GL_COLOR_ARRAY_SIZE 0x8081
#define GL_COLOR_ARRAY_TYPE 0x8082
#define GL_COLOR_ARRAY_STRIDE 0x8083
#define GL_INDEX_ARRAY_TYPE 0x8085
#define GL_INDEX_ARRAY_STRIDE 0x8086
#define GL_TEXTURE_COORD_ARRAY_SIZE 0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE 0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE 0x808A
#define GL_EDGE_FLAG_ARRAY_STRIDE 0x808C
#define GL_VERTEX_ARRAY_POINTER 0x808E
#define GL_NORMAL_ARRAY_POINTER 0x808F
#define GL_COLOR_ARRAY_POINTER 0x8090
#define GL_INDEX_ARRAY_POINTER 0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER 0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER 0x8093
#define GL_V2F 0x2A20
#define GL_V3F 0x2A21
#define GL_C4UB_V2F 0x2A22
#define GL_C4UB_V3F 0x2A23
#define GL_C3F_V3F 0x2A24
#define GL_N3F_V3F 0x2A25
#define GL_C4F_N3F_V3F 0x2A26
#define GL_T2F_V3F 0x2A27
#define GL_T4F_V4F 0x2A28
#define GL_T2F_C4UB_V3F 0x2A29
#define GL_T2F_C3F_V3F 0x2A2A
#define GL_T2F_N3F_V3F 0x2A2B
#define GL_T2F_C4F_N3F_V3F 0x2A2C
#define GL_T4F_C4F_N3F_V4F 0x2A2D

#define glAccum GlobalOpenGL().m_glAccum
#define glAlphaFunc GlobalOpenGL().m_glAlphaFunc
#define glAreTexturesResident GlobalOpenGL().m_glAreTexturesResident
#define glArrayElement GlobalOpenGL().m_glArrayElement
#define glBegin GlobalOpenGL().m_glBegin
#define glBindTexture GlobalOpenGL().m_glBindTexture
#define glBitmap GlobalOpenGL().m_glBitmap
#define glBlendFunc GlobalOpenGL().m_glBlendFunc
#define glCallList GlobalOpenGL().m_glCallList
#define glCallLists GlobalOpenGL().m_glCallLists
#define glClear GlobalOpenGL().m_glClear
#define glClearAccum GlobalOpenGL().m_glClearAccum
#define glClearColor GlobalOpenGL().m_glClearColor
#define glClearDepth GlobalOpenGL().m_glClearDepth
#define glClearIndex GlobalOpenGL().m_glClearIndex
#define glClearStencil GlobalOpenGL().m_glClearStencil
#define glClipPlane GlobalOpenGL().m_glClipPlane
#define glColor3b GlobalOpenGL().m_glColor3b
#define glColor3bv GlobalOpenGL().m_glColor3bv
#define glColor3d GlobalOpenGL().m_glColor3d
#define glColor3dv GlobalOpenGL().m_glColor3dv
#define glColor3f GlobalOpenGL().m_glColor3f
#define glColor3fv GlobalOpenGL().m_glColor3fv
#define glColor3i GlobalOpenGL().m_glColor3i
#define glColor3iv GlobalOpenGL().m_glColor3iv
#define glColor3s GlobalOpenGL().m_glColor3s
#define glColor3sv GlobalOpenGL().m_glColor3sv
#define glColor3ub GlobalOpenGL().m_glColor3ub
#define glColor3ubv GlobalOpenGL().m_glColor3ubv
#define glColor3ui GlobalOpenGL().m_glColor3ui
#define glColor3uiv GlobalOpenGL().m_glColor3uiv
#define glColor3us GlobalOpenGL().m_glColor3us
#define glColor3usv GlobalOpenGL().m_glColor3usv
#define glColor4b GlobalOpenGL().m_glColor4b
#define glColor4bv GlobalOpenGL().m_glColor4bv
#define glColor4d GlobalOpenGL().m_glColor4d
#define glColor4dv GlobalOpenGL().m_glColor4dv
#define glColor4f GlobalOpenGL().m_glColor4f
#define glColor4fv GlobalOpenGL().m_glColor4fv
#define glColor4i GlobalOpenGL().m_glColor4i
#define glColor4iv GlobalOpenGL().m_glColor4iv
#define glColor4s GlobalOpenGL().m_glColor4s
#define glColor4sv GlobalOpenGL().m_glColor4sv
#define glColor4ub GlobalOpenGL().m_glColor4ub
#define glColor4ubv GlobalOpenGL().m_glColor4ubv
#define glColor4ui GlobalOpenGL().m_glColor4ui
#define glColor4uiv GlobalOpenGL().m_glColor4uiv
#define glColor4us GlobalOpenGL().m_glColor4us
#define glColor4usv GlobalOpenGL().m_glColor4usv
#define glColorMask GlobalOpenGL().m_glColorMask
#define glColorMaterial GlobalOpenGL().m_glColorMaterial
#define glColorPointer GlobalOpenGL().m_glColorPointer
#define glCopyPixels GlobalOpenGL().m_glCopyPixels
#define glCopyTexImage1D GlobalOpenGL().m_glCopyTexImage1D
#define glCopyTexImage2D GlobalOpenGL().m_glCopyTexImage2D
#define glCopyTexSubImage1D GlobalOpenGL().m_glCopyTexSubImage1D
#define glCopyTexSubImage2D GlobalOpenGL().m_glCopyTexSubImage2D
#define glCullFace GlobalOpenGL().m_glCullFace
#define glDeleteLists GlobalOpenGL().m_glDeleteLists
#define glDeleteTextures GlobalOpenGL().m_glDeleteTextures
#define glDepthFunc GlobalOpenGL().m_glDepthFunc
#define glDepthMask GlobalOpenGL().m_glDepthMask
#define glDepthRange GlobalOpenGL().m_glDepthRange
#define glDisable GlobalOpenGL().m_glDisable
#define glDisableClientState GlobalOpenGL().m_glDisableClientState
#define glDrawArrays GlobalOpenGL().m_glDrawArrays
#define glDrawBuffer GlobalOpenGL().m_glDrawBuffer
#define glDrawElements GlobalOpenGL().m_glDrawElements
#define glDrawPixels GlobalOpenGL().m_glDrawPixels
#define glEdgeFlag GlobalOpenGL().m_glEdgeFlag
#define glEdgeFlagPointer GlobalOpenGL().m_glEdgeFlagPointer
#define glEdgeFlagv GlobalOpenGL().m_glEdgeFlagv
#define glEnable GlobalOpenGL().m_glEnable
#define glEnableClientState GlobalOpenGL().m_glEnableClientState
#define glEnd GlobalOpenGL().m_glEnd
#define glEndList GlobalOpenGL().m_glEndList
#define glEvalCoord1d GlobalOpenGL().m_glEvalCoord1d
#define glEvalCoord1dv GlobalOpenGL().m_glEvalCoord1dv
#define glEvalCoord1f GlobalOpenGL().m_glEvalCoord1f
#define glEvalCoord1fv GlobalOpenGL().m_glEvalCoord1fv
#define glEvalCoord2d GlobalOpenGL().m_glEvalCoord2d
#define glEvalCoord2dv GlobalOpenGL().m_glEvalCoord2dv
#define glEvalCoord2f GlobalOpenGL().m_glEvalCoord2f
#define glEvalCoord2fv GlobalOpenGL().m_glEvalCoord2fv
#define glEvalMesh1 GlobalOpenGL().m_glEvalMesh1
#define glEvalMesh2 GlobalOpenGL().m_glEvalMesh2
#define glEvalPoint1 GlobalOpenGL().m_glEvalPoint1
#define glEvalPoint2 GlobalOpenGL().m_glEvalPoint2
#define glFeedbackBuffer GlobalOpenGL().m_glFeedbackBuffer
#define glFinish GlobalOpenGL().m_glFinish
#define glFlush GlobalOpenGL().m_glFlush
#define glFogf GlobalOpenGL().m_glFogf
#define glFogfv GlobalOpenGL().m_glFogfv
#define glFogi GlobalOpenGL().m_glFogi
#define glFogiv GlobalOpenGL().m_glFogiv
#define glFrontFace GlobalOpenGL().m_glFrontFace
#define glFrustum GlobalOpenGL().m_glFrustum
#define glGenLists GlobalOpenGL().m_glGenLists
#define glGenTextures GlobalOpenGL().m_glGenTextures
#define glGetBooleanv GlobalOpenGL().m_glGetBooleanv
#define glGetClipPlane GlobalOpenGL().m_glGetClipPlane
#define glGetDoublev GlobalOpenGL().m_glGetDoublev
#define glGetError GlobalOpenGL().m_glGetError
#define glGetFloatv GlobalOpenGL().m_glGetFloatv
#define glGetIntegerv GlobalOpenGL().m_glGetIntegerv
#define glGetLightfv GlobalOpenGL().m_glGetLightfv
#define glGetLightiv GlobalOpenGL().m_glGetLightiv
#define glGetMapdv GlobalOpenGL().m_glGetMapdv
#define glGetMapfv GlobalOpenGL().m_glGetMapfv
#define glGetMapiv GlobalOpenGL().m_glGetMapiv
#define glGetMaterialfv GlobalOpenGL().m_glGetMaterialfv
#define glGetMaterialiv GlobalOpenGL().m_glGetMaterialiv
#define glGetPixelMapfv GlobalOpenGL().m_glGetPixelMapfv
#define glGetPixelMapuiv GlobalOpenGL().m_glGetPixelMapuiv
#define glGetPixelMapusv GlobalOpenGL().m_glGetPixelMapusv
#define glGetPointerv GlobalOpenGL().m_glGetPointerv
#define glGetPolygonStipple GlobalOpenGL().m_glGetPolygonStipple
#define glGetString GlobalOpenGL().m_glGetString
#define glGetTexEnvfv GlobalOpenGL().m_glGetTexEnvfv
#define glGetTexEnviv GlobalOpenGL().m_glGetTexEnviv
#define glGetTexGendv GlobalOpenGL().m_glGetTexGendv
#define glGetTexGenfv GlobalOpenGL().m_glGetTexGenfv
#define glGetTexGeniv GlobalOpenGL().m_glGetTexGeniv
#define glGetTexImage GlobalOpenGL().m_glGetTexImage
#define glGetTexLevelParameterfv GlobalOpenGL().m_glGetTexLevelParameter
#define glGetTexLevelParameteriv GlobalOpenGL().m_glGetTexLevelParameteriv
#define glGetTexParameterfv GlobalOpenGL().m_glGetTexParameterfv
#define glGetTexParameteriv GlobalOpenGL().m_glGetTexParameteriv
#define glHint GlobalOpenGL().m_glHint
#define glIndexMask GlobalOpenGL().m_glIndexMask
#define glIndexPointer GlobalOpenGL().m_glIndexPointer
#define glIndexd GlobalOpenGL().m_glIndexd
#define glIndexdv GlobalOpenGL().m_glIndexdv
#define glIndexf GlobalOpenGL().m_glIndexf
#define glIndexfv GlobalOpenGL().m_glIndexfv
#define glIndexi GlobalOpenGL().m_glIndexi
#define glIndexiv GlobalOpenGL().m_glIndexiv
#define glIndexs GlobalOpenGL().m_glIndexs
#define glIndexsv GlobalOpenGL().m_glIndexsv
#define glIndexub GlobalOpenGL().m_glIndexub
#define glIndexubv GlobalOpenGL().m_glIndexubv
#define glInitNames GlobalOpenGL().m_glInitNames
#define glInterleavedArrays GlobalOpenGL().m_glInterleavedArrays
#define glIsEnabled GlobalOpenGL().m_glIsEnabled
#define glIsList GlobalOpenGL().m_glIsList
#define glIsTexture GlobalOpenGL().m_glIsTexture
#define glLightModelf GlobalOpenGL().m_glLightModelf
#define glLightModelfv GlobalOpenGL().m_glLightModelfv
#define glLightModeli GlobalOpenGL().m_glLightModeli
#define glLightModeliv GlobalOpenGL().m_glLightModeliv
#define glLightf GlobalOpenGL().m_glLightf
#define glLightfv GlobalOpenGL().m_glLightfv
#define glLighti GlobalOpenGL().m_glLighti
#define glLightiv GlobalOpenGL().m_glLightiv
#define glLineStipple GlobalOpenGL().m_glLineStipple
#define glLineWidth GlobalOpenGL().m_glLineWidth
#define glListBase GlobalOpenGL().m_glListBase
#define glLoadIdentity GlobalOpenGL().m_glLoadIdentity
#define glLoadMatrixd GlobalOpenGL().m_glLoadMatrixd
#define glLoadMatrixf GlobalOpenGL().m_glLoadMatrixf
#define glLoadName GlobalOpenGL().m_glLoadName
#define glLogicOp GlobalOpenGL().m_glLogicOp
#define glMap1d GlobalOpenGL().m_glMap1d
#define glMap1f GlobalOpenGL().m_glMap1f
#define glMap2d GlobalOpenGL().m_glMap2d
#define glMap2f GlobalOpenGL().m_glMap2f
#define glMapGrid1d GlobalOpenGL().m_glMapGrid1d
#define glMapGrid1f GlobalOpenGL().m_glMapGrid1f
#define glMapGrid2d GlobalOpenGL().m_glMapGrid2d
#define glMapGrid2f GlobalOpenGL().m_glMapGrid2f
#define glMaterialf GlobalOpenGL().m_glMaterialf
#define glMaterialfv GlobalOpenGL().m_glMaterialfv
#define glMateriali GlobalOpenGL().m_glMateriali
#define glMaterialiv GlobalOpenGL().m_glMaterialiv
#define glMatrixMode GlobalOpenGL().m_glMatrixMode
#define glMultMatrixd GlobalOpenGL().m_glMultMatrixd
#define glMultMatrixf GlobalOpenGL().m_glMultMatrixf
#define glNewList GlobalOpenGL().m_glNewList
#define glNormal3b GlobalOpenGL().m_glNormal3b
#define glNormal3bv GlobalOpenGL().m_glNormal3bv
#define glNormal3d GlobalOpenGL().m_glNormal3d
#define glNormal3dv GlobalOpenGL().m_glNormal3dv
#define glNormal3f GlobalOpenGL().m_glNormal3f
#define glNormal3fv GlobalOpenGL().m_glNormal3fv
#define glNormal3i GlobalOpenGL().m_glNormal3i
#define glNormal3iv GlobalOpenGL().m_glNormal3iv
#define glNormal3s GlobalOpenGL().m_glNormal3s
#define glNormal3sv GlobalOpenGL().m_glNormal3sv
#define glNormalPointer GlobalOpenGL().m_glNormalPointer
#define glOrtho GlobalOpenGL().m_glOrtho
#define glPassThrough GlobalOpenGL().m_glPassThrough
#define glPixelMapfv GlobalOpenGL().m_glPixelMapfv
#define glPixelMapuiv GlobalOpenGL().m_glPixelMapuiv
#define glPixelMapusv GlobalOpenGL().m_glPixelMapusv
#define glPixelStoref GlobalOpenGL().m_glPixelStoref
#define glPixelStorei GlobalOpenGL().m_glPixelStorei
#define glPixelTransferf GlobalOpenGL().m_glPixelTransferf
#define glPixelTransferi GlobalOpenGL().m_glPixelTransferi
#define glPixelZoom GlobalOpenGL().m_glPixelZoom
#define glPointSize GlobalOpenGL().m_glPointSize
#define glPolygonMode GlobalOpenGL().m_glPolygonMode
#define glPolygonOffset GlobalOpenGL().m_glPolygonOffset
#define glPolygonStipple GlobalOpenGL().m_glPolygonStipple
#define glPopAttrib GlobalOpenGL().m_glPopAttrib
#define glPopClientAttrib GlobalOpenGL().m_glPopClientAttrib
#define glPopMatrix GlobalOpenGL().m_glPopMatrix
#define glPopName GlobalOpenGL().m_glPopName
#define glPrioritizeTextures GlobalOpenGL().m_glPrioritizeTextures
#define glPushAttrib GlobalOpenGL().m_glPushAttrib
#define glPushClientAttrib GlobalOpenGL().m_glPushClientAttrib
#define glPushMatrix GlobalOpenGL().m_glPushMatrix
#define glPushName GlobalOpenGL().m_glPushName
#define glRasterPos2d GlobalOpenGL().m_glRasterPos2d
#define glRasterPos2dv GlobalOpenGL().m_glRasterPos2dv
#define glRasterPos2f GlobalOpenGL().m_glRasterPos2f
#define glRasterPos2fv GlobalOpenGL().m_glRasterPos2fv
#define glRasterPos2i GlobalOpenGL().m_glRasterPos2i
#define glRasterPos2iv GlobalOpenGL().m_glRasterPos2iv
#define glRasterPos2s GlobalOpenGL().m_glRasterPos2s
#define glRasterPos2sv GlobalOpenGL().m_glRasterPos2sv
#define glRasterPos3d GlobalOpenGL().m_glRasterPos3d
#define glRasterPos3dv GlobalOpenGL().m_glRasterPos3dv
#define glRasterPos3f GlobalOpenGL().m_glRasterPos3f
#define glRasterPos3fv GlobalOpenGL().m_glRasterPos3fv
#define glRasterPos3i GlobalOpenGL().m_glRasterPos3i
#define glRasterPos3iv GlobalOpenGL().m_glRasterPos3iv
#define glRasterPos3s GlobalOpenGL().m_glRasterPos3s
#define glRasterPos3sv GlobalOpenGL().m_glRasterPos3sv
#define glRasterPos4d GlobalOpenGL().m_glRasterPos4d
#define glRasterPos4dv GlobalOpenGL().m_glRasterPos4dv
#define glRasterPos4f GlobalOpenGL().m_glRasterPos4f
#define glRasterPos4fv GlobalOpenGL().m_glRasterPos4fv
#define glRasterPos4i GlobalOpenGL().m_glRasterPos4i
#define glRasterPos4iv GlobalOpenGL().m_glRasterPos4iv
#define glRasterPos4s GlobalOpenGL().m_glRasterPos4s
#define glRasterPos4sv GlobalOpenGL().m_glRasterPos4sv
#define glReadBuffer GlobalOpenGL().m_glReadBuffer
#define glReadPixels GlobalOpenGL().m_glReadPixels
#define glRectd GlobalOpenGL().m_glRectd
#define glRectdv GlobalOpenGL().m_glRectdv
#define glRectf GlobalOpenGL().m_glRectf
#define glRectfv GlobalOpenGL().m_glRectfv
#define glRecti GlobalOpenGL().m_glRecti
#define glRectiv GlobalOpenGL().m_glRectiv
#define glRects GlobalOpenGL().m_glRects
#define glRectsv GlobalOpenGL().m_glRectsv
#define glRenderMode GlobalOpenGL().m_glRenderMode
#define glRotated GlobalOpenGL().m_glRotated
#define glRotatef GlobalOpenGL().m_glRotatef
#define glScaled GlobalOpenGL().m_glScaled
#define glScalef GlobalOpenGL().m_glScalef
#define glScissor GlobalOpenGL().m_glScissor
#define glSelectBuffer GlobalOpenGL().m_glSelectBuffer
#define glShadeModel GlobalOpenGL().m_glShadeModel
#define glStencilFunc GlobalOpenGL().m_glStencilFunc
#define glStencilMask GlobalOpenGL().m_glStencilMask
#define glStencilOp GlobalOpenGL().m_glStencilOp
#define glTexCoord1d GlobalOpenGL().m_glTexCoord1d
#define glTexCoord1dv GlobalOpenGL().m_glTexCoord1dv
#define glTexCoord1f GlobalOpenGL().m_glTexCoord1f
#define glTexCoord1fv GlobalOpenGL().m_glTexCoord1fv
#define glTexCoord1i GlobalOpenGL().m_glTexCoord1i
#define glTexCoord1iv GlobalOpenGL().m_glTexCoord1iv
#define glTexCoord1s GlobalOpenGL().m_glTexCoord1s
#define glTexCoord1sv GlobalOpenGL().m_glTexCoord1sv
#define glTexCoord2d GlobalOpenGL().m_glTexCoord2d
#define glTexCoord2dv GlobalOpenGL().m_glTexCoord2dv
#define glTexCoord2f GlobalOpenGL().m_glTexCoord2f
#define glTexCoord2fv GlobalOpenGL().m_glTexCoord2fv
#define glTexCoord2i GlobalOpenGL().m_glTexCoord2i
#define glTexCoord2iv GlobalOpenGL().m_glTexCoord2iv
#define glTexCoord2s GlobalOpenGL().m_glTexCoord2s
#define glTexCoord2sv GlobalOpenGL().m_glTexCoord2sv
#define glTexCoord3d GlobalOpenGL().m_glTexCoord3d
#define glTexCoord3dv GlobalOpenGL().m_glTexCoord3dv
#define glTexCoord3f GlobalOpenGL().m_glTexCoord3f
#define glTexCoord3fv GlobalOpenGL().m_glTexCoord3fv
#define glTexCoord3i GlobalOpenGL().m_glTexCoord3i
#define glTexCoord3iv GlobalOpenGL().m_glTexCoord3iv
#define glTexCoord3s GlobalOpenGL().m_glTexCoord3s
#define glTexCoord3sv GlobalOpenGL().m_glTexCoord3sv
#define glTexCoord4d GlobalOpenGL().m_glTexCoord4d
#define glTexCoord4dv GlobalOpenGL().m_glTexCoord4dv
#define glTexCoord4f GlobalOpenGL().m_glTexCoord4f
#define glTexCoord4fv GlobalOpenGL().m_glTexCoord4fv
#define glTexCoord4i GlobalOpenGL().m_glTexCoord4i
#define glTexCoord4iv GlobalOpenGL().m_glTexCoord4iv
#define glTexCoord4s GlobalOpenGL().m_glTexCoord4s
#define glTexCoord4sv GlobalOpenGL().m_glTexCoord4sv
#define glTexCoordPointer GlobalOpenGL().m_glTexCoordPointer
#define glTexEnvf GlobalOpenGL().m_glTexEnvf
#define glTexEnvfv GlobalOpenGL().m_glTexEnvfv
#define glTexEnvi GlobalOpenGL().m_glTexEnvi
#define glTexEnviv GlobalOpenGL().m_glTexEnviv
#define glTexGend GlobalOpenGL().m_glTexGend
#define glTexGendv GlobalOpenGL().m_glTexGendv
#define glTexGenf GlobalOpenGL().m_glTexGenf
#define glTexGenfv GlobalOpenGL().m_glTexGenfv
#define glTexGeni GlobalOpenGL().m_glTexGeni
#define glTexGeniv GlobalOpenGL().m_glTexGeniv
#define glTexImage1D GlobalOpenGL().m_glTexImage1D
#define glTexImage2D GlobalOpenGL().m_glTexImage2D
#define glTexParameterf GlobalOpenGL().m_glTexParameterf
#define glTexParameterfv GlobalOpenGL().m_glTexParameterfv
#define glTexParameteri GlobalOpenGL().m_glTexParameteri
#define glTexParameteriv GlobalOpenGL().m_glTexParameteriv
#define glTexSubImage1D GlobalOpenGL().m_glTexSubImage1D
#define glTexSubImage2D GlobalOpenGL().m_glTexSubImage2D
#define glTranslated GlobalOpenGL().m_glTranslated
#define glTranslatef GlobalOpenGL().m_glTranslatef
#define glVertex2d GlobalOpenGL().m_glVertex2d
#define glVertex2dv GlobalOpenGL().m_glVertex2dv
#define glVertex2f GlobalOpenGL().m_glVertex2f
#define glVertex2fv GlobalOpenGL().m_glVertex2fv
#define glVertex2i GlobalOpenGL().m_glVertex2i
#define glVertex2iv GlobalOpenGL().m_glVertex2iv
#define glVertex2s GlobalOpenGL().m_glVertex2s
#define glVertex2sv GlobalOpenGL().m_glVertex2sv
#define glVertex3d GlobalOpenGL().m_glVertex3d
#define glVertex3dv GlobalOpenGL().m_glVertex3dv
#define glVertex3f GlobalOpenGL().m_glVertex3f
#define glVertex3fv GlobalOpenGL().m_glVertex3fv
#define glVertex3i GlobalOpenGL().m_glVertex3i
#define glVertex3iv GlobalOpenGL().m_glVertex3iv
#define glVertex3s GlobalOpenGL().m_glVertex3s
#define glVertex3sv GlobalOpenGL().m_glVertex3sv
#define glVertex4d GlobalOpenGL().m_glVertex4d
#define glVertex4dv GlobalOpenGL().m_glVertex4dv
#define glVertex4f GlobalOpenGL().m_glVertex4f
#define glVertex4fv GlobalOpenGL().m_glVertex4fv
#define glVertex4i GlobalOpenGL().m_glVertex4i
#define glVertex4iv GlobalOpenGL().m_glVertex4iv
#define glVertex4s GlobalOpenGL().m_glVertex4s
#define glVertex4sv GlobalOpenGL().m_glVertex4sv
#define glVertexPointer GlobalOpenGL().m_glVertexPointer
#define glViewport GlobalOpenGL().m_glViewport

#endif


#if !defined(GL_EXT_vertex_array)
#define GL_EXT_vertex_array 1

#define GL_VERTEX_ARRAY_EXT 0x8074
#define GL_NORMAL_ARRAY_EXT 0x8075
#define GL_COLOR_ARRAY_EXT 0x8076
#define GL_INDEX_ARRAY_EXT 0x8077
#define GL_TEXTURE_COORD_ARRAY_EXT 0x8078
#define GL_EDGE_FLAG_ARRAY_EXT 0x8079
#define GL_VERTEX_ARRAY_SIZE_EXT 0x807A
#define GL_VERTEX_ARRAY_TYPE_EXT 0x807B
#define GL_VERTEX_ARRAY_STRIDE_EXT 0x807C
#define GL_VERTEX_ARRAY_COUNT_EXT 0x807D
#define GL_NORMAL_ARRAY_TYPE_EXT 0x807E
#define GL_NORMAL_ARRAY_STRIDE_EXT 0x807F
#define GL_NORMAL_ARRAY_COUNT_EXT 0x8080
#define GL_COLOR_ARRAY_SIZE_EXT 0x8081
#define GL_COLOR_ARRAY_TYPE_EXT 0x8082
#define GL_COLOR_ARRAY_STRIDE_EXT 0x8083
#define GL_COLOR_ARRAY_COUNT_EXT 0x8084
#define GL_INDEX_ARRAY_TYPE_EXT 0x8085
#define GL_INDEX_ARRAY_STRIDE_EXT 0x8086
#define GL_INDEX_ARRAY_COUNT_EXT 0x8087
#define GL_TEXTURE_COORD_ARRAY_SIZE_EXT 0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE_EXT 0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT 0x808A
#define GL_TEXTURE_COORD_ARRAY_COUNT_EXT 0x808B
#define GL_EDGE_FLAG_ARRAY_STRIDE_EXT 0x808C
#define GL_EDGE_FLAG_ARRAY_COUNT_EXT 0x808D
#define GL_VERTEX_ARRAY_POINTER_EXT 0x808E
#define GL_NORMAL_ARRAY_POINTER_EXT 0x808F
#define GL_COLOR_ARRAY_POINTER_EXT 0x8090
#define GL_INDEX_ARRAY_POINTER_EXT 0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER_EXT 0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER_EXT 0x8093
#define GL_DOUBLE_EXT GL_DOUBLE

#endif


#if !defined(GL_EXT_bgra)
#define GL_EXT_bgra 1

#define GL_BGR_EXT 0x80E0
#define GL_BGRA_EXT 0x80E1

#endif


#if !defined(GL_EXT_paletted_texture)
#define GL_EXT_paletted_texture 1

#define GL_COLOR_TABLE_FORMAT_EXT 0x80D8
#define GL_COLOR_TABLE_WIDTH_EXT 0x80D9
#define GL_COLOR_TABLE_RED_SIZE_EXT 0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE_EXT 0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE_EXT 0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE_EXT 0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE_EXT 0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE_EXT 0x80DF

#define GL_COLOR_INDEX1_EXT 0x80E2
#define GL_COLOR_INDEX2_EXT 0x80E3
#define GL_COLOR_INDEX4_EXT 0x80E4
#define GL_COLOR_INDEX8_EXT 0x80E5
#define GL_COLOR_INDEX12_EXT 0x80E6
#define GL_COLOR_INDEX16_EXT 0x80E7

#define GL_LOGIC_OP GL_INDEX_LOGIC_OP
#define GL_TEXTURE_COMPONENTS GL_TEXTURE_INTERNAL_FORMAT

#endif

#if !defined(GL_ARB_multitexture)
#define GL_ARB_multitexture 1

#define GL_TEXTURE0_ARB 0x84C0
#define GL_TEXTURE1_ARB 0x84C1
#define GL_TEXTURE2_ARB 0x84C2
#define GL_TEXTURE3_ARB 0x84C3
#define GL_TEXTURE4_ARB 0x84C4
#define GL_TEXTURE5_ARB 0x84C5
#define GL_TEXTURE6_ARB 0x84C6
#define GL_TEXTURE7_ARB 0x84C7
#define GL_TEXTURE8_ARB 0x84C8
#define GL_TEXTURE9_ARB 0x84C9
#define GL_TEXTURE10_ARB 0x84CA
#define GL_TEXTURE11_ARB 0x84CB
#define GL_TEXTURE12_ARB 0x84CC
#define GL_TEXTURE13_ARB 0x84CD
#define GL_TEXTURE14_ARB 0x84CE
#define GL_TEXTURE15_ARB 0x84CF
#define GL_TEXTURE16_ARB 0x84D0
#define GL_TEXTURE17_ARB 0x84D1
#define GL_TEXTURE18_ARB 0x84D2
#define GL_TEXTURE19_ARB 0x84D3
#define GL_TEXTURE20_ARB 0x84D4
#define GL_TEXTURE21_ARB 0x84D5
#define GL_TEXTURE22_ARB 0x84D6
#define GL_TEXTURE23_ARB 0x84D7
#define GL_TEXTURE24_ARB 0x84D8
#define GL_TEXTURE25_ARB 0x84D9
#define GL_TEXTURE26_ARB 0x84DA
#define GL_TEXTURE27_ARB 0x84DB
#define GL_TEXTURE28_ARB 0x84DC
#define GL_TEXTURE29_ARB 0x84DD
#define GL_TEXTURE30_ARB 0x84DE
#define GL_TEXTURE31_ARB 0x84DF
#define GL_ACTIVE_TEXTURE_ARB 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB 0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB 0x84E2

#define glActiveTextureARB GlobalOpenGL().m_glActiveTextureARB
#define glClientActiveTextureARB GlobalOpenGL().m_glClientActiveTextureARB
#define glMultiTexCoord1dARB GlobalOpenGL().m_glMultiTexCoord1dARB
#define glMultiTexCoord1dvARB GlobalOpenGL().m_glMultiTexCoord1dvARB
#define glMultiTexCoord1fARB GlobalOpenGL().m_glMultiTexCoord1fARB
#define glMultiTexCoord1fvARB GlobalOpenGL().m_glMultiTexCoord1fvARB
#define glMultiTexCoord1iARB GlobalOpenGL().m_glMultiTexCoord1iARB
#define glMultiTexCoord1ivARB GlobalOpenGL().m_glMultiTexCoord1ivARB
#define glMultiTexCoord1sARB GlobalOpenGL().m_glMultiTexCoord1sARB
#define glMultiTexCoord1svARB GlobalOpenGL().m_glMultiTexCoord1svARB
#define glMultiTexCoord2dARB GlobalOpenGL().m_glMultiTexCoord2dARB
#define glMultiTexCoord2dvARB GlobalOpenGL().m_glMultiTexCoord2dvARB
#define glMultiTexCoord2fARB GlobalOpenGL().m_glMultiTexCoord2fARB
#define glMultiTexCoord2fvARB GlobalOpenGL().m_glMultiTexCoord2fvARB
#define glMultiTexCoord2iARB GlobalOpenGL().m_glMultiTexCoord2iARB
#define glMultiTexCoord2ivARB GlobalOpenGL().m_glMultiTexCoord2ivARB
#define glMultiTexCoord2sARB GlobalOpenGL().m_glMultiTexCoord2sARB
#define glMultiTexCoord2svARB GlobalOpenGL().m_glMultiTexCoord2svARB
#define glMultiTexCoord3dARB GlobalOpenGL().m_glMultiTexCoord3dARB
#define glMultiTexCoord3dvARB GlobalOpenGL().m_glMultiTexCoord3dvARB
#define glMultiTexCoord3fARB GlobalOpenGL().m_glMultiTexCoord3fARB
#define glMultiTexCoord3fvARB GlobalOpenGL().m_glMultiTexCoord3fvARB
#define glMultiTexCoord3iARB GlobalOpenGL().m_glMultiTexCoord3iARB
#define glMultiTexCoord3ivARB GlobalOpenGL().m_glMultiTexCoord3ivARB
#define glMultiTexCoord3sARB GlobalOpenGL().m_glMultiTexCoord3sARB
#define glMultiTexCoord3svARB GlobalOpenGL().m_glMultiTexCoord3svARB
#define glMultiTexCoord4dARB GlobalOpenGL().m_glMultiTexCoord4dARB
#define glMultiTexCoord4dvARB GlobalOpenGL().m_glMultiTexCoord4dvARB
#define glMultiTexCoord4fARB GlobalOpenGL().m_glMultiTexCoord4fARB
#define glMultiTexCoord4fvARB GlobalOpenGL().m_glMultiTexCoord4fvARB
#define glMultiTexCoord4iARB GlobalOpenGL().m_glMultiTexCoord4iARB
#define glMultiTexCoord4ivARB GlobalOpenGL().m_glMultiTexCoord4ivARB
#define glMultiTexCoord4sARB GlobalOpenGL().m_glMultiTexCoord4sARB
#define glMultiTexCoord4svARB GlobalOpenGL().m_glMultiTexCoord4svARB

#endif


// EXT_texture_compression_s3tc
#if !defined(GL_EXT_texture_compression_s3tc)
#define GL_EXT_texture_compression_s3tc 1

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

#endif


// ARB_texture_compression
#if !defined(GL_ARB_texture_compression)
#define GL_ARB_texture_compression 1

#define GL_COMPRESSED_ALPHA_ARB 0x84E9
#define GL_COMPRESSED_LUMINANCE_ARB 0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA_ARB 0x84EB
#define GL_COMPRESSED_INTENSITY_ARB 0x84EC
#define GL_COMPRESSED_RGB_ARB 0x84ED
#define GL_COMPRESSED_RGBA_ARB 0x84EE
#define GL_TEXTURE_COMPRESSION_HINT_ARB 0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB 0x86A0
#define GL_TEXTURE_COMPRESSED_ARB 0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A3

#define glCompressedTexImage3DARB GlobalOpenGL().m_glCompressedTexImage3DARB
#define glCompressedTexImage2DARB GlobalOpenGL().m_glCompressedTexImage2DARB
#define glCompressedTexImage1DARB GlobalOpenGL().m_glCompressedTexImage1DARB
#define glCompressedTexSubImage3DARB GlobalOpenGL().m_glCompressedTexSubImage3DARB
#define glCompressedTexSubImage2DARB GlobalOpenGL().m_glCompressedTexSubImage2DARB
#define glCompressedTexSubImage1DARB GlobalOpenGL().m_glCompressedTexSubImage1DARB
#define glGetCompressedTexImageARB GlobalOpenGL().m_glGetCompressedTexImageARB

#endif


// GL 1.2

#if !defined(GL_VERSION_1_2)

#define GL_SMOOTH_POINT_SIZE_RANGE 0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY 0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE 0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_UNSIGNED_BYTE_3_3_2 0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_INT_8_8_8_8 0x8035
#define GL_UNSIGNED_INT_10_10_10_2 0x8036
#define GL_RESCALE_NORMAL 0x803A
#define GL_TEXTURE_BINDING_3D 0x806A
#define GL_PACK_SKIP_IMAGES 0x806B
#define GL_PACK_IMAGE_HEIGHT 0x806C
#define GL_UNPACK_SKIP_IMAGES 0x806D
#define GL_UNPACK_IMAGE_HEIGHT 0x806E
#define GL_TEXTURE_3D 0x806F
#define GL_PROXY_TEXTURE_3D 0x8070
#define GL_TEXTURE_DEPTH 0x8071
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1
#define GL_MAX_ELEMENTS_VERTICES 0x80E8
#define GL_MAX_ELEMENTS_INDICES 0x80E9
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE_MIN_LOD 0x813A
#define GL_TEXTURE_MAX_LOD 0x813B
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
#define GL_SINGLE_COLOR 0x81F9
#define GL_SEPARATE_SPECULAR_COLOR 0x81FA
#define GL_UNSIGNED_BYTE_2_3_3_REV 0x8362
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#define GL_ALIASED_POINT_SIZE_RANGE 0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE 0x846E

#define glCopyTexSubImage3D GlobalOpenGL().m_glCopyTexSubImage3D
#define glDrawRangeElements GlobalOpenGL().m_glDrawRangeElements
#define glTexImage3D GlobalOpenGL().m_glTexImage3D
#define glTexSubImage3D GlobalOpenGL().m_glTexSubImage3D

#endif


// GL 1.3

#if !defined(GL_VERSION_1_3)
#define GL_VERSION_1_3 1

#define GL_MULTISAMPLE 0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#define GL_SAMPLE_ALPHA_TO_ONE 0x809F
#define GL_SAMPLE_COVERAGE 0x80A0
#define GL_SAMPLE_BUFFERS 0x80A8
#define GL_SAMPLES 0x80A9
#define GL_SAMPLE_COVERAGE_VALUE 0x80AA
#define GL_SAMPLE_COVERAGE_INVERT 0x80AB
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF
#define GL_ACTIVE_TEXTURE 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE 0x84E1
#define GL_MAX_TEXTURE_UNITS 0x84E2
#define GL_TRANSPOSE_MODELVIEW_MATRIX 0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX 0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX 0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX 0x84E6
#define GL_SUBTRACT 0x84E7
#define GL_COMPRESSED_ALPHA 0x84E9
#define GL_COMPRESSED_LUMINANCE 0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA 0x84EB
#define GL_COMPRESSED_INTENSITY 0x84EC
#define GL_COMPRESSED_RGB 0x84ED
#define GL_COMPRESSED_RGBA 0x84EE
#define GL_TEXTURE_COMPRESSION_HINT 0x84EF
#define GL_NORMAL_MAP 0x8511
#define GL_REFLECTION_MAP 0x8512
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP 0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP 0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE 0x851C
#define GL_COMBINE 0x8570
#define GL_COMBINE_RGB 0x8571
#define GL_COMBINE_ALPHA 0x8572
#define GL_RGB_SCALE 0x8573
#define GL_ADD_SIGNED 0x8574
#define GL_INTERPOLATE 0x8575
#define GL_CONSTANT 0x8576
#define GL_PRIMARY_COLOR 0x8577
#define GL_PREVIOUS 0x8578
#define GL_SOURCE0_RGB 0x8580
#define GL_SOURCE1_RGB 0x8581
#define GL_SOURCE2_RGB 0x8582
#define GL_SOURCE0_ALPHA 0x8588
#define GL_SOURCE1_ALPHA 0x8589
#define GL_SOURCE2_ALPHA 0x858A
#define GL_OPERAND0_RGB 0x8590
#define GL_OPERAND1_RGB 0x8591
#define GL_OPERAND2_RGB 0x8592
#define GL_OPERAND0_ALPHA 0x8598
#define GL_OPERAND1_ALPHA 0x8599
#define GL_OPERAND2_ALPHA 0x859A
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE 0x86A0
#define GL_TEXTURE_COMPRESSED 0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS 0x86A3
#define GL_DOT3_RGB 0x86AE
#define GL_DOT3_RGBA 0x86AF
#define GL_MULTISAMPLE_BIT 0x20000000

#define glActiveTexture GlobalOpenGL().m_glActiveTexture
#define glClientActiveTexture GlobalOpenGL().m_glClientActiveTexture
#define glCompressedTexImage1D GlobalOpenGL().m_glCompressedTexImage1D
#define glCompressedTexImage2D GlobalOpenGL().m_glCompressedTexImage2D
#define glCompressedTexImage3D GlobalOpenGL().m_glCompressedTexImage3D
#define glCompressedTexSubImage1D GlobalOpenGL().m_glCompressedTexSubImage1D
#define glCompressedTexSubImage2D GlobalOpenGL().m_glCompressedTexSubImage2D
#define glCompressedTexSubImage3D GlobalOpenGL().m_glCompressedTexSubImage3D
#define glGetCompressedTexImage GlobalOpenGL().m_glGetCompressedTexImage
#define glLoadTransposeMatrixd GlobalOpenGL().m_glLoadTransposeMatrixd
#define glLoadTransposeMatrixf GlobalOpenGL().m_glLoadTransposeMatrixf
#define glMultTransposeMatrixd GlobalOpenGL().m_glMultTransposeMatrixd
#define glMultTransposeMatrixf GlobalOpenGL().m_glMultTransposeMatrixf
#define glMultiTexCoord1d GlobalOpenGL().m_glMultiTexCoord1d
#define glMultiTexCoord1dv GlobalOpenGL().m_glMultiTexCoord1dv
#define glMultiTexCoord1f GlobalOpenGL().m_glMultiTexCoord1f
#define glMultiTexCoord1fv GlobalOpenGL().m_glMultiTexCoord1fv
#define glMultiTexCoord1i GlobalOpenGL().m_glMultiTexCoord1i
#define glMultiTexCoord1iv GlobalOpenGL().m_glMultiTexCoord1iv
#define glMultiTexCoord1s GlobalOpenGL().m_glMultiTexCoord1s
#define glMultiTexCoord1sv GlobalOpenGL().m_glMultiTexCoord1sv
#define glMultiTexCoord2d GlobalOpenGL().m_glMultiTexCoord2d
#define glMultiTexCoord2dv GlobalOpenGL().m_glMultiTexCoord2dv
#define glMultiTexCoord2f GlobalOpenGL().m_glMultiTexCoord2f
#define glMultiTexCoord2fv GlobalOpenGL().m_glMultiTexCoord2fv
#define glMultiTexCoord2i GlobalOpenGL().m_glMultiTexCoord2i
#define glMultiTexCoord2iv GlobalOpenGL().m_glMultiTexCoord2iv
#define glMultiTexCoord2s GlobalOpenGL().m_glMultiTexCoord2s
#define glMultiTexCoord2sv GlobalOpenGL().m_glMultiTexCoord2sv
#define glMultiTexCoord3d GlobalOpenGL().m_glMultiTexCoord3d
#define glMultiTexCoord3dv GlobalOpenGL().m_glMultiTexCoord3dv
#define glMultiTexCoord3f GlobalOpenGL().m_glMultiTexCoord3f
#define glMultiTexCoord3fv GlobalOpenGL().m_glMultiTexCoord3fv
#define glMultiTexCoord3i GlobalOpenGL().m_glMultiTexCoord3i
#define glMultiTexCoord3iv GlobalOpenGL().m_glMultiTexCoord3iv
#define glMultiTexCoord3s GlobalOpenGL().m_glMultiTexCoord3s
#define glMultiTexCoord3sv GlobalOpenGL().m_glMultiTexCoord3sv
#define glMultiTexCoord4d GlobalOpenGL().m_glMultiTexCoord4d
#define glMultiTexCoord4dv GlobalOpenGL().m_glMultiTexCoord4dv
#define glMultiTexCoord4f GlobalOpenGL().m_glMultiTexCoord4f
#define glMultiTexCoord4fv GlobalOpenGL().m_glMultiTexCoord4fv
#define glMultiTexCoord4i GlobalOpenGL().m_glMultiTexCoord4i
#define glMultiTexCoord4iv GlobalOpenGL().m_glMultiTexCoord4iv
#define glMultiTexCoord4s GlobalOpenGL().m_glMultiTexCoord4s
#define glMultiTexCoord4sv GlobalOpenGL().m_glMultiTexCoord4sv
#define glSampleCoverage GlobalOpenGL().m_glSampleCoverage

#endif


// GL 1.4
#if !defined(GL_VERSION_1_4)
#define GL_VERSION_1_4 1

#define GL_BLEND_DST_RGB 0x80C8
#define GL_BLEND_SRC_RGB 0x80C9
#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_SRC_ALPHA 0x80CB
#define GL_POINT_SIZE_MIN 0x8126
#define GL_POINT_SIZE_MAX 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE 0x8128
#define GL_POINT_DISTANCE_ATTENUATION 0x8129
#define GL_GENERATE_MIPMAP 0x8191
#define GL_GENERATE_MIPMAP_HINT 0x8192
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_DEPTH_COMPONENT32 0x81A7
#define GL_MIRRORED_REPEAT 0x8370
#define GL_FOG_COORDINATE_SOURCE 0x8450
#define GL_FOG_COORDINATE 0x8451
#define GL_FRAGMENT_DEPTH 0x8452
#define GL_CURRENT_FOG_COORDINATE 0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE 0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER 0x8456
#define GL_FOG_COORDINATE_ARRAY 0x8457
#define GL_COLOR_SUM 0x8458
#define GL_CURRENT_SECONDARY_COLOR 0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE 0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE 0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER 0x845D
#define GL_SECONDARY_COLOR_ARRAY 0x845E
#define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#define GL_TEXTURE_FILTER_CONTROL 0x8500
#define GL_TEXTURE_LOD_BIAS 0x8501
#define GL_INCR_WRAP 0x8507
#define GL_DECR_WRAP 0x8508
#define GL_TEXTURE_DEPTH_SIZE 0x884A
#define GL_DEPTH_TEXTURE_MODE 0x884B
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_COMPARE_R_TO_TEXTURE 0x884E

#define glBlendColor GlobalOpenGL().m_glBlendColor
#define glBlendEquation GlobalOpenGL().m_glBlendEquation
#define glBlendFuncSeparate GlobalOpenGL().m_glBlendFuncSeparate
#define glFogCoordPointer GlobalOpenGL().m_glFogCoordPointer
#define glFogCoordd GlobalOpenGL().m_glFogCoordd
#define glFogCoorddv GlobalOpenGL().m_glFogCoorddv
#define glFogCoordf GlobalOpenGL().m_glFogCoordf
#define glFogCoordfv GlobalOpenGL().m_glFogCoordfv
#define glMultiDrawArrays GlobalOpenGL().m_glMultiDrawArrays
#define glMultiDrawElements GlobalOpenGL().m_glMultiDrawElements
#define glPointParameterf GlobalOpenGL().m_glPointParameterf
#define glPointParameterfv GlobalOpenGL().m_glPointParameterfv
#define glSecondaryColor3b GlobalOpenGL().m_glSecondaryColor3b
#define glSecondaryColor3bv GlobalOpenGL().m_glSecondaryColor3bv
#define glSecondaryColor3d GlobalOpenGL().m_glSecondaryColor3d
#define glSecondaryColor3dv GlobalOpenGL().m_glSecondaryColor3dv
#define glSecondaryColor3f GlobalOpenGL().m_glSecondaryColor3f
#define glSecondaryColor3fv GlobalOpenGL().m_glSecondaryColor3fv
#define glSecondaryColor3i GlobalOpenGL().m_glSecondaryColor3i
#define glSecondaryColor3iv GlobalOpenGL().m_glSecondaryColor3iv
#define glSecondaryColor3s GlobalOpenGL().m_glSecondaryColor3s
#define glSecondaryColor3sv GlobalOpenGL().m_glSecondaryColor3sv
#define glSecondaryColor3ub GlobalOpenGL().m_glSecondaryColor3ub
#define glSecondaryColor3ubv GlobalOpenGL().m_glSecondaryColor3ubv
#define glSecondaryColor3ui GlobalOpenGL().m_glSecondaryColor3ui
#define glSecondaryColor3uiv GlobalOpenGL().m_glSecondaryColor3uiv
#define glSecondaryColor3us GlobalOpenGL().m_glSecondaryColor3us
#define glSecondaryColor3usv GlobalOpenGL().m_glSecondaryColor3usv
#define glSecondaryColorPointer GlobalOpenGL().m_glSecondaryColorPointer
#define glWindowPos2d GlobalOpenGL().m_glWindowPos2d
#define glWindowPos2dv GlobalOpenGL().m_glWindowPos2dv
#define glWindowPos2f GlobalOpenGL().m_glWindowPos2f
#define glWindowPos2fv GlobalOpenGL().m_glWindowPos2fv
#define glWindowPos2i GlobalOpenGL().m_glWindowPos2i
#define glWindowPos2iv GlobalOpenGL().m_glWindowPos2iv
#define glWindowPos2s GlobalOpenGL().m_glWindowPos2s
#define glWindowPos2sv GlobalOpenGL().m_glWindowPos2sv
#define glWindowPos3d GlobalOpenGL().m_glWindowPos3d
#define glWindowPos3dv GlobalOpenGL().m_glWindowPos3dv
#define glWindowPos3f GlobalOpenGL().m_glWindowPos3f
#define glWindowPos3fv GlobalOpenGL().m_glWindowPos3fv
#define glWindowPos3i GlobalOpenGL().m_glWindowPos3i
#define glWindowPos3iv GlobalOpenGL().m_glWindowPos3iv
#define glWindowPos3s GlobalOpenGL().m_glWindowPos3s
#define glWindowPos3sv GlobalOpenGL().m_glWindowPos3sv

#endif


// GL 1.5
#if !defined(GL_VERSION_1_5)
#define GL_VERSION_1_5 1

#define GL_FOG_COORD GL_FOG_COORDINATE
#define GL_FOG_COORD_ARRAY GL_FOG_COORDINATE_ARRAY
#define GL_SRC0_RGB GL_SOURCE0_RGB
#define GL_FOG_COORD_ARRAY_POINTER GL_FOG_COORDINATE_ARRAY_POINTER
#define GL_FOG_COORD_SOURCE GL_FOG_COORDINATE_SOURCE
#define GL_FOG_COORD_ARRAY_TYPE GL_FOG_COORDINATE_ARRAY_TYPE
#define GL_SRC1_ALPHA GL_SOURCE1_ALPHA
#define GL_CURRENT_FOG_COORD GL_CURRENT_FOG_COORDINATE
#define GL_FOG_COORD_ARRAY_STRIDE GL_FOG_COORDINATE_ARRAY_STRIDE
#define GL_SRC0_ALPHA GL_SOURCE0_ALPHA
#define GL_SRC1_RGB GL_SOURCE1_RGB
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING
#define GL_SRC2_ALPHA GL_SOURCE2_ALPHA
#define GL_SRC2_RGB GL_SOURCE2_RGB
#define GL_BUFFER_SIZE 0x8764
#define GL_BUFFER_USAGE 0x8765
#define GL_QUERY_COUNTER_BITS 0x8864
#define GL_CURRENT_QUERY 0x8865
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_BUFFER_ACCESS 0x88BB
#define GL_BUFFER_MAPPED 0x88BC
#define GL_BUFFER_MAP_POINTER 0x88BD
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_SAMPLES_PASSED 0x8914

typedef std::ptrdiff_t GLsizeiptr;
typedef std::ptrdiff_t GLintptr;

#define glBeginQuery GlobalOpenGL().m_glBeginQuery
#define glBindBuffer GlobalOpenGL().m_glBindBuffer
#define glBufferData GlobalOpenGL().m_glBufferData
#define glBufferSubData GlobalOpenGL().m_glBufferSubData
#define glDeleteBuffers GlobalOpenGL().m_glDeleteBuffers
#define glDeleteQueries GlobalOpenGL().m_glDeleteQueries
#define glEndQuery GlobalOpenGL().m_glEndQuery
#define glGenBuffers GlobalOpenGL().m_glGenBuffers
#define glGenQueries GlobalOpenGL().m_glGenQueries
#define glGetBufferParameteriv GlobalOpenGL().m_glGetBufferParameteriv
#define glGetBufferPointerv GlobalOpenGL().m_glGetBufferPointerv
#define glGetBufferSubData GlobalOpenGL().m_glGetBufferSubData
#define glGetQueryObjectiv GlobalOpenGL().m_glGetQueryObjectiv
#define glGetQueryObjectuiv GlobalOpenGL().m_glGetQueryObjectuiv
#define glGetQueryiv GlobalOpenGL().m_glGetQueryiv
#define glIsBuffer GlobalOpenGL().m_glIsBuffer
#define glIsQuery GlobalOpenGL().m_glIsQuery
#define glMapBuffer GlobalOpenGL().m_glMapBuffer
#define glUnmapBuffer GlobalOpenGL().m_glUnmapBuffer

#endif


// GL_ARB_vertex_program
#if !defined(GL_ARB_vertex_program)
#define GL_ARB_vertex_program

#define GL_VERTEX_PROGRAM_ARB 0x8620
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB 0x8643
#define GL_COLOR_SUM_ARB 0x8458
#define GL_PROGRAM_FORMAT_ASCII_ARB 0x8875
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB 0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB 0x886A
#define GL_CURRENT_VERTEX_ATTRIB_ARB 0x8626
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB 0x8645
#define GL_PROGRAM_LENGTH_ARB 0x8627
#define GL_PROGRAM_FORMAT_ARB 0x8876
#define GL_PROGRAM_BINDING_ARB 0x8677
#define GL_PROGRAM_INSTRUCTIONS_ARB 0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB 0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB 0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB 0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A6
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A7
#define GL_PROGRAM_PARAMETERS_ARB 0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB 0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB 0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB 0x88AB
#define GL_PROGRAM_ATTRIBS_ARB 0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB 0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB 0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB 0x88AF
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB 0x88B0
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB 0x88B1
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B2
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B3
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB 0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB 0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB 0x88B6
#define GL_PROGRAM_STRING_ARB 0x8628
#define GL_PROGRAM_ERROR_POSITION_ARB 0x864B
#define GL_CURRENT_MATRIX_ARB 0x8641
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB 0x88B7
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB 0x8640
#define GL_MAX_VERTEX_ATTRIBS_ARB 0x8869
#define GL_MAX_PROGRAM_MATRICES_ARB 0x862F
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB 0x862E
#define GL_PROGRAM_ERROR_STRING_ARB 0x8874
#define GL_MATRIX0_ARB 0x88C0
#define GL_MATRIX1_ARB 0x88C1
#define GL_MATRIX2_ARB 0x88C2
#define GL_MATRIX3_ARB 0x88C3
#define GL_MATRIX4_ARB 0x88C4
#define GL_MATRIX5_ARB 0x88C5
#define GL_MATRIX6_ARB 0x88C6
#define GL_MATRIX7_ARB 0x88C7
#define GL_MATRIX8_ARB 0x88C8
#define GL_MATRIX9_ARB 0x88C9
#define GL_MATRIX10_ARB 0x88CA
#define GL_MATRIX11_ARB 0x88CB
#define GL_MATRIX12_ARB 0x88CC
#define GL_MATRIX13_ARB 0x88CD
#define GL_MATRIX14_ARB 0x88CE
#define GL_MATRIX15_ARB 0x88CF
#define GL_MATRIX16_ARB 0x88D0
#define GL_MATRIX17_ARB 0x88D1
#define GL_MATRIX18_ARB 0x88D2
#define GL_MATRIX19_ARB 0x88D3
#define GL_MATRIX20_ARB 0x88D4
#define GL_MATRIX21_ARB 0x88D5
#define GL_MATRIX22_ARB 0x88D6
#define GL_MATRIX23_ARB 0x88D7
#define GL_MATRIX24_ARB 0x88D8
#define GL_MATRIX25_ARB 0x88D9
#define GL_MATRIX26_ARB 0x88DA
#define GL_MATRIX27_ARB 0x88DB
#define GL_MATRIX28_ARB 0x88DC
#define GL_MATRIX29_ARB 0x88DD
#define GL_MATRIX30_ARB 0x88DE
#define GL_MATRIX31_ARB 0x88DF

#define glVertexAttrib1sARB GlobalOpenGL().m_glVertexAttrib1sARB
#define glVertexAttrib1fARB GlobalOpenGL().m_glVertexAttrib1fARB
#define glVertexAttrib1dARB GlobalOpenGL().m_glVertexAttrib1dARB
#define glVertexAttrib2sARB GlobalOpenGL().m_glVertexAttrib2sARB
#define glVertexAttrib2fARB GlobalOpenGL().m_glVertexAttrib2fARB
#define glVertexAttrib2dARB GlobalOpenGL().m_glVertexAttrib2dARB
#define glVertexAttrib3sARB GlobalOpenGL().m_glVertexAttrib3sARB
#define glVertexAttrib3fARB GlobalOpenGL().m_glVertexAttrib3fARB
#define glVertexAttrib3dARB GlobalOpenGL().m_glVertexAttrib3dARB
#define glVertexAttrib4sARB GlobalOpenGL().m_glVertexAttrib4sARB
#define glVertexAttrib4fARB GlobalOpenGL().m_glVertexAttrib4fARB
#define glVertexAttrib4dARB GlobalOpenGL().m_glVertexAttrib4dARB
#define glVertexAttrib4NubARB GlobalOpenGL().m_glVertexAttrib4NubARB
#define glVertexAttrib1svARB GlobalOpenGL().m_glVertexAttrib1svARB
#define glVertexAttrib1fvARB GlobalOpenGL().m_glVertexAttrib1fvARB
#define glVertexAttrib1dvARB GlobalOpenGL().m_glVertexAttrib1dvARB
#define glVertexAttrib2svARB GlobalOpenGL().m_glVertexAttrib2svARB
#define glVertexAttrib2fvARB GlobalOpenGL().m_glVertexAttrib2fvARB
#define glVertexAttrib2dvARB GlobalOpenGL().m_glVertexAttrib2dvARB
#define glVertexAttrib3svARB GlobalOpenGL().m_glVertexAttrib3svARB
#define glVertexAttrib3fvARB GlobalOpenGL().m_glVertexAttrib3fvARB
#define glVertexAttrib3dvARB GlobalOpenGL().m_glVertexAttrib3dvARB
#define glVertexAttrib4bvARB GlobalOpenGL().m_glVertexAttrib4bvARB
#define glVertexAttrib4svARB GlobalOpenGL().m_glVertexAttrib4svARB
#define glVertexAttrib4ivARB GlobalOpenGL().m_glVertexAttrib4ivARB
#define glVertexAttrib4ubvARB GlobalOpenGL().m_glVertexAttrib4ubvARB
#define glVertexAttrib4usvARB GlobalOpenGL().m_glVertexAttrib4usvARB
#define glVertexAttrib4uivARB GlobalOpenGL().m_glVertexAttrib4uivARB
#define glVertexAttrib4fvARB GlobalOpenGL().m_glVertexAttrib4fvARB
#define glVertexAttrib4dvARB GlobalOpenGL().m_glVertexAttrib4dvARB
#define glVertexAttrib4NbvARB GlobalOpenGL().m_glVertexAttrib4NbvARB
#define glVertexAttrib4NsvARB GlobalOpenGL().m_glVertexAttrib4NsvARB
#define glVertexAttrib4NivARB GlobalOpenGL().m_glVertexAttrib4NivARB
#define glVertexAttrib4NubvARB GlobalOpenGL().m_glVertexAttrib4NubvARB
#define glVertexAttrib4NusvARB GlobalOpenGL().m_glVertexAttrib4NusvARB
#define glVertexAttrib4NuivARB GlobalOpenGL().m_glVertexAttrib4NuivARB
#define glVertexAttribPointerARB GlobalOpenGL().m_glVertexAttribPointerARB
#define glEnableVertexAttribArrayARB GlobalOpenGL().m_glEnableVertexAttribArrayARB
#define glDisableVertexAttribArrayARB GlobalOpenGL().m_glDisableVertexAttribArrayARB
#define glProgramStringARB GlobalOpenGL().m_glProgramStringARB
#define glBindProgramARB GlobalOpenGL().m_glBindProgramARB
#define glDeleteProgramsARB GlobalOpenGL().m_glDeleteProgramsARB
#define glGenProgramsARB GlobalOpenGL().m_glGenProgramsARB
#define glProgramEnvParameter4dARB GlobalOpenGL().m_glProgramEnvParameter4dARB
#define glProgramEnvParameter4dvARB GlobalOpenGL().m_glProgramEnvParameter4dvARB
#define glProgramEnvParameter4fARB GlobalOpenGL().m_glProgramEnvParameter4fARB
#define glProgramEnvParameter4fvARB GlobalOpenGL().m_glProgramEnvParameter4fvARB
#define glProgramLocalParameter4dARB GlobalOpenGL().m_glProgramLocalParameter4dARB
#define glProgramLocalParameter4dvARB GlobalOpenGL().m_glProgramLocalParameter4dvARB
#define glProgramLocalParameter4fARB GlobalOpenGL().m_glProgramLocalParameter4fARB
#define glProgramLocalParameter4fvARB GlobalOpenGL().m_glProgramLocalParameter4fvARB
#define glGetProgramEnvParameterdvARB GlobalOpenGL().m_glGetProgramEnvParameterdvARB
#define glGetProgramEnvParameterfvARB GlobalOpenGL().m_glGetProgramEnvParameterfvARB
#define glGetProgramLocalParameterdvARB GlobalOpenGL().m_glGetProgramLocalParameterdvARB
#define glGetProgramLocalParameterfvARB GlobalOpenGL().m_glGetProgramLocalParameterfvARB
#define glGetProgramivARB GlobalOpenGL().m_glGetProgramivARB
#define glGetProgramStringARB GlobalOpenGL().m_glGetProgramStringARB
#define glGetVertexAttribdvARB GlobalOpenGL().m_glGetVertexAttribdvARB
#define glGetVertexAttribfvARB GlobalOpenGL().m_glGetVertexAttribfvARB
#define glGetVertexAttribivARB GlobalOpenGL().m_glGetVertexAttribivARB
#define glGetVertexAttribPointervARB GlobalOpenGL().m_glGetVertexAttribPointervARB
#define glIsProgramARB GlobalOpenGL().m_glIsProgramARB

#endif


// GL_ARB_fragment_program
#if !defined(GL_ARB_fragment_program)
#define GL_ARB_fragment_program 1

#define GL_FRAGMENT_PROGRAM_ARB 0x8804
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB 0x8805
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB 0x8806
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB 0x8807
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x8808
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x8809
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x880A
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB 0x880B
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB 0x880C
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB 0x880D
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x880E
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x880F
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x8810
#define GL_MAX_TEXTURE_COORDS_ARB 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB 0x8872

#endif


// GL_ARB_shader_objects
#if !defined(GL_ARB_shader_objects)
#define GL_ARB_shader_objects 1

#define GL_PROGRAM_OBJECT_ARB 0x8B40
#define GL_SHADER_OBJECT_ARB 0x8B48
#define GL_OBJECT_TYPE_ARB 0x8B4E
#define GL_OBJECT_SUBTYPE_ARB 0x8B4F
#define GL_FLOAT_VEC2_ARB 0x8B50
#define GL_FLOAT_VEC3_ARB 0x8B51
#define GL_FLOAT_VEC4_ARB 0x8B52
#define GL_INT_VEC2_ARB 0x8B53
#define GL_INT_VEC3_ARB 0x8B54
#define GL_INT_VEC4_ARB 0x8B55
#define GL_BOOL_ARB 0x8B56
#define GL_BOOL_VEC2_ARB 0x8B57
#define GL_BOOL_VEC3_ARB 0x8B58
#define GL_BOOL_VEC4_ARB 0x8B59
#define GL_FLOAT_MAT2_ARB 0x8B5A
#define GL_FLOAT_MAT3_ARB 0x8B5B
#define GL_FLOAT_MAT4_ARB 0x8B5C
#define GL_SAMPLER_1D_ARB 0x8B5D
#define GL_SAMPLER_2D_ARB 0x8B5E
#define GL_SAMPLER_3D_ARB 0x8B5F
#define GL_SAMPLER_CUBE_ARB 0x8B60
#define GL_SAMPLER_1D_SHADOW_ARB 0x8B61
#define GL_SAMPLER_2D_SHADOW_ARB 0x8B62
#define GL_SAMPLER_2D_RECT_ARB 0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW_ARB 0x8B64
#define GL_OBJECT_DELETE_STATUS_ARB 0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB 0x8B81
#define GL_OBJECT_LINK_STATUS_ARB 0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB 0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB 0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB 0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB 0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB 0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB 0x8B88

#define glDeleteObjectARB GlobalOpenGL().m_glDeleteObjectARB
#define glGetHandleARB GlobalOpenGL().m_glGetHandleARB
#define glDetachObjectARB GlobalOpenGL().m_glDetachObjectARB
#define glCreateShaderObjectARB GlobalOpenGL().m_glCreateShaderObjectARB
#define glShaderSourceARB GlobalOpenGL().m_glShaderSourceARB
#define glCompileShaderARB GlobalOpenGL().m_glCompileShaderARB
#define glCreateProgramObjectARB GlobalOpenGL().m_glCreateProgramObjectARB
#define glAttachObjectARB GlobalOpenGL().m_glAttachObjectARB
#define glLinkProgramARB GlobalOpenGL().m_glLinkProgramARB
#define glUseProgramObjectARB GlobalOpenGL().m_glUseProgramObjectARB
#define glValidateProgramARB GlobalOpenGL().m_glValidateProgramARB
#define glUniform1fARB GlobalOpenGL().m_glUniform1fARB
#define glUniform2fARB GlobalOpenGL().m_glUniform2fARB
#define glUniform3fARB GlobalOpenGL().m_glUniform3fARB
#define glUniform4fARB GlobalOpenGL().m_glUniform4fARB
#define glUniform1iARB GlobalOpenGL().m_glUniform1iARB
#define glUniform2iARB GlobalOpenGL().m_glUniform2iARB
#define glUniform3iARB GlobalOpenGL().m_glUniform3iARB
#define glUniform4iARB GlobalOpenGL().m_glUniform4iARB
#define glUniform1fvARB GlobalOpenGL().m_glUniform1fvARB
#define glUniform2fvARB GlobalOpenGL().m_glUniform2fvARB
#define glUniform3fvARB GlobalOpenGL().m_glUniform3fvARB
#define glUniform4fvARB GlobalOpenGL().m_glUniform4fvARB
#define glUniform1ivARB GlobalOpenGL().m_glUniform1ivARB
#define glUniform2ivARB GlobalOpenGL().m_glUniform2ivARB
#define glUniform3ivARB GlobalOpenGL().m_glUniform3ivARB
#define glUniform4ivARB GlobalOpenGL().m_glUniform4ivARB
#define glUniformMatrix2fvARB GlobalOpenGL().m_glUniformMatrix2fvARB
#define glUniformMatrix3fvARB GlobalOpenGL().m_glUniformMatrix3fvARB
#define glUniformMatrix4fvARB GlobalOpenGL().m_glUniformMatrix4fvARB
#define glGetObjectParameterfvARB GlobalOpenGL().m_glGetObjectParameterfvARB
#define glGetObjectParameterivARB GlobalOpenGL().m_glGetObjectParameterivARB
#define glGetInfoLogARB GlobalOpenGL().m_glGetInfoLogARB
#define glGetAttachedObjectsARB GlobalOpenGL().m_glGetAttachedObjectsARB
#define glGetUniformLocationARB GlobalOpenGL().m_glGetUniformLocationARB
#define glGetActiveUniformARB GlobalOpenGL().m_glGetActiveUniformARB
#define glGetUniformfvARB GlobalOpenGL().m_glGetUniformfvARB
#define glGetUniformivARB GlobalOpenGL().m_glGetUniformivARB
#define glGetShaderSourceARB GlobalOpenGL().m_glGetShaderSourceARB

typedef char GLcharARB;
typedef unsigned int GLhandleARB;

#endif

// GL_ARB_vertex_shader
#if !defined(GL_ARB_vertex_shader)
#define GL_ARB_vertex_shader 1

#define GL_VERTEX_SHADER_ARB 0x8B31
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB 0x8B4A
#define GL_MAX_VARYING_FLOATS_ARB 0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB 0x8B4D
#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB 0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB 0x8B8A

#if 0
#define glVertexAttrib1fARB GlobalOpenGL().m_glVertexAttrib1fARB
#define glVertexAttrib1sARB GlobalOpenGL().m_glVertexAttrib1sARB
#define glVertexAttrib1dARB GlobalOpenGL().m_glVertexAttrib1dARB
#define glVertexAttrib2fARB GlobalOpenGL().m_glVertexAttrib2fARB
#define glVertexAttrib2sARB GlobalOpenGL().m_glVertexAttrib2sARB
#define glVertexAttrib2dARB GlobalOpenGL().m_glVertexAttrib2dARB
#define glVertexAttrib3fARB GlobalOpenGL().m_glVertexAttrib3fARB
#define glVertexAttrib3sARB GlobalOpenGL().m_glVertexAttrib3sARB
#define glVertexAttrib3dARB GlobalOpenGL().m_glVertexAttrib3dARB
#define glVertexAttrib4fARB GlobalOpenGL().m_glVertexAttrib4fARB
#define glVertexAttrib4sARB GlobalOpenGL().m_glVertexAttrib4sARB
#define glVertexAttrib4dARB GlobalOpenGL().m_glVertexAttrib4dARB
#define glVertexAttrib4NubARB GlobalOpenGL().m_glVertexAttrib4NubARB
#define glVertexAttrib1fvARB GlobalOpenGL().m_glVertexAttrib1fvARB
#define glVertexAttrib1svARB GlobalOpenGL().m_glVertexAttrib1svARB
#define glVertexAttrib1dvARB GlobalOpenGL().m_glVertexAttrib1dvARB
#define glVertexAttrib2fvARB GlobalOpenGL().m_glVertexAttrib2fvARB
#define glVertexAttrib2svARB GlobalOpenGL().m_glVertexAttrib2svARB
#define glVertexAttrib2dvARB GlobalOpenGL().m_glVertexAttrib2dvARB
#define glVertexAttrib3fvARB GlobalOpenGL().m_glVertexAttrib3fvARB
#define glVertexAttrib3svARB GlobalOpenGL().m_glVertexAttrib3svARB
#define glVertexAttrib3dvARB GlobalOpenGL().m_glVertexAttrib3dvARB
#define glVertexAttrib4fvARB GlobalOpenGL().m_glVertexAttrib4fvARB
#define glVertexAttrib4svARB GlobalOpenGL().m_glVertexAttrib4svARB
#define glVertexAttrib4dvARB GlobalOpenGL().m_glVertexAttrib4dvARB
#define glVertexAttrib4ivARB GlobalOpenGL().m_glVertexAttrib4ivARB
#define glVertexAttrib4bvARB GlobalOpenGL().m_glVertexAttrib4bvARB
#define glVertexAttrib4ubvARB GlobalOpenGL().m_glVertexAttrib4ubvARB
#define glVertexAttrib4usvARB GlobalOpenGL().m_glVertexAttrib4usvARB
#define glVertexAttrib4uivARB GlobalOpenGL().m_glVertexAttrib4uivARB
#define glVertexAttrib4NbvARB GlobalOpenGL().m_glVertexAttrib4NbvARB
#define glVertexAttrib4NsvARB GlobalOpenGL().m_glVertexAttrib4NsvARB
#define glVertexAttrib4NivARB GlobalOpenGL().m_glVertexAttrib4NivARB
#define glVertexAttrib4NubvARB GlobalOpenGL().m_glVertexAttrib4NubvARB
#define glVertexAttrib4NusvARB GlobalOpenGL().m_glVertexAttrib4NusvARB
#define glVertexAttrib4NuivARB GlobalOpenGL().m_glVertexAttrib4NuivARB
#define glVertexAttribPointerARB GlobalOpenGL().m_glVertexAttribPointerARB
#define glEnableVertexAttribArrayARB GlobalOpenGL().m_glEnableVertexAttribArrayARB
#define glDisableVertexAttribArrayARB GlobalOpenGL().m_glDisableVertexAttribArrayARB
#endif
#define glBindAttribLocationARB GlobalOpenGL().m_glBindAttribLocationARB
#define glGetActiveAttribARB GlobalOpenGL().m_glGetActiveAttribARB
#define glGetAttribLocationARB GlobalOpenGL().m_glGetAttribLocationARB
#if 0
#define glGetVertexAttribdvARB GlobalOpenGL().m_glGetVertexAttribdvARB
#define glGetVertexAttribfvARB GlobalOpenGL().m_glGetVertexAttribfvARB
#define glGetVertexAttribivARB GlobalOpenGL().m_glGetVertexAttribivARB
#define glGetVertexAttribPointervARB GlobalOpenGL().m_glGetVertexAttribPointervARB
#endif
#endif



// GL_ARB_fragment_shader
#if !defined(GL_ARB_fragment_shader)
#define GL_ARB_fragment_shader 1

#define GL_FRAGMENT_SHADER_ARB 0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB 0x8B49
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB 0x8B8B

#endif


// GL_ARB_shading_language_100
#if !defined(GL_ARB_shading_language_100)
#define GL_ARB_shading_language_100 1

#define GL_SHADING_LANGUAGE_VERSION_ARB 0x8B8C

#endif


// GL_NV_vertex_program2
#if !defined(GL_NV_vertex_program)
#define GL_NV_vertex_program 1

#define GL_VERTEX_PROGRAM_NV 0x8620
#define GL_VERTEX_STATE_PROGRAM_NV 0x8621
#define GL_ATTRIB_ARRAY_SIZE_NV 0x8623
#define GL_ATTRIB_ARRAY_STRIDE_NV 0x8624
#define GL_ATTRIB_ARRAY_TYPE_NV 0x8625
#define GL_CURRENT_ATTRIB_NV 0x8626
#define GL_PROGRAM_LENGTH_NV 0x8627
#define GL_PROGRAM_STRING_NV 0x8628
#define GL_MODELVIEW_PROJECTION_NV 0x8629
#define GL_IDENTITY_NV 0x862A
#define GL_INVERSE_NV 0x862B
#define GL_TRANSPOSE_NV 0x862C
#define GL_INVERSE_TRANSPOSE_NV 0x862D
#define GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV 0x862E
#define GL_MAX_TRACK_MATRICES_NV 0x862F
#define GL_MATRIX0_NV 0x8630
#define GL_MATRIX1_NV 0x8631
#define GL_MATRIX2_NV 0x8632
#define GL_MATRIX3_NV 0x8633
#define GL_MATRIX4_NV 0x8634
#define GL_MATRIX5_NV 0x8635
#define GL_MATRIX6_NV 0x8636
#define GL_MATRIX7_NV 0x8637
#define GL_CURRENT_MATRIX_STACK_DEPTH_NV 0x8640
#define GL_CURRENT_MATRIX_NV 0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_NV 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_NV 0x8643
#define GL_PROGRAM_PARAMETER_NV 0x8644
#define GL_ATTRIB_ARRAY_POINTER_NV 0x8645
#define GL_PROGRAM_TARGET_NV 0x8646
#define GL_PROGRAM_RESIDENT_NV 0x8647
#define GL_TRACK_MATRIX_NV 0x8648
#define GL_TRACK_MATRIX_TRANSFORM_NV 0x8649
#define GL_VERTEX_PROGRAM_BINDING_NV 0x864A
#define GL_PROGRAM_ERROR_POSITION_NV 0x864B
#define GL_VERTEX_ATTRIB_ARRAY0_NV 0x8650
#define GL_VERTEX_ATTRIB_ARRAY1_NV 0x8651
#define GL_VERTEX_ATTRIB_ARRAY2_NV 0x8652
#define GL_VERTEX_ATTRIB_ARRAY3_NV 0x8653
#define GL_VERTEX_ATTRIB_ARRAY4_NV 0x8654
#define GL_VERTEX_ATTRIB_ARRAY5_NV 0x8655
#define GL_VERTEX_ATTRIB_ARRAY6_NV 0x8656
#define GL_VERTEX_ATTRIB_ARRAY7_NV 0x8657
#define GL_VERTEX_ATTRIB_ARRAY8_NV 0x8658
#define GL_VERTEX_ATTRIB_ARRAY9_NV 0x8659
#define GL_VERTEX_ATTRIB_ARRAY10_NV 0x865A
#define GL_VERTEX_ATTRIB_ARRAY11_NV 0x865B
#define GL_VERTEX_ATTRIB_ARRAY12_NV 0x865C
#define GL_VERTEX_ATTRIB_ARRAY13_NV 0x865D
#define GL_VERTEX_ATTRIB_ARRAY14_NV 0x865E
#define GL_VERTEX_ATTRIB_ARRAY15_NV 0x865F
#define GL_MAP1_VERTEX_ATTRIB0_4_NV 0x8660
#define GL_MAP1_VERTEX_ATTRIB1_4_NV 0x8661
#define GL_MAP1_VERTEX_ATTRIB2_4_NV 0x8662
#define GL_MAP1_VERTEX_ATTRIB3_4_NV 0x8663
#define GL_MAP1_VERTEX_ATTRIB4_4_NV 0x8664
#define GL_MAP1_VERTEX_ATTRIB5_4_NV 0x8665
#define GL_MAP1_VERTEX_ATTRIB6_4_NV 0x8666
#define GL_MAP1_VERTEX_ATTRIB7_4_NV 0x8667
#define GL_MAP1_VERTEX_ATTRIB8_4_NV 0x8668
#define GL_MAP1_VERTEX_ATTRIB9_4_NV 0x8669
#define GL_MAP1_VERTEX_ATTRIB10_4_NV 0x866A
#define GL_MAP1_VERTEX_ATTRIB11_4_NV 0x866B
#define GL_MAP1_VERTEX_ATTRIB12_4_NV 0x866C
#define GL_MAP1_VERTEX_ATTRIB13_4_NV 0x866D
#define GL_MAP1_VERTEX_ATTRIB14_4_NV 0x866E
#define GL_MAP1_VERTEX_ATTRIB15_4_NV 0x866F
#define GL_MAP2_VERTEX_ATTRIB0_4_NV 0x8670
#define GL_MAP2_VERTEX_ATTRIB1_4_NV 0x8671
#define GL_MAP2_VERTEX_ATTRIB2_4_NV 0x8672
#define GL_MAP2_VERTEX_ATTRIB3_4_NV 0x8673
#define GL_MAP2_VERTEX_ATTRIB4_4_NV 0x8674
#define GL_MAP2_VERTEX_ATTRIB5_4_NV 0x8675
#define GL_MAP2_VERTEX_ATTRIB6_4_NV 0x8676
#define GL_MAP2_VERTEX_ATTRIB7_4_NV 0x8677
#define GL_MAP2_VERTEX_ATTRIB8_4_NV 0x8678
#define GL_MAP2_VERTEX_ATTRIB9_4_NV 0x8679
#define GL_MAP2_VERTEX_ATTRIB10_4_NV 0x867A
#define GL_MAP2_VERTEX_ATTRIB11_4_NV 0x867B
#define GL_MAP2_VERTEX_ATTRIB12_4_NV 0x867C
#define GL_MAP2_VERTEX_ATTRIB13_4_NV 0x867D
#define GL_MAP2_VERTEX_ATTRIB14_4_NV 0x867E
#define GL_MAP2_VERTEX_ATTRIB15_4_NV 0x867F

#define glAreProgramsResidentNV GlobalOpenGL().m_glAreProgramsResidentNV
#define glBindProgramNV GlobalOpenGL().m_glBindProgramNV
#define glDeleteProgramsNV GlobalOpenGL().m_glDeleteProgramsNV
#define glExecuteProgramNV GlobalOpenGL().m_glExecuteProgramNV
#define glGenProgramsNV GlobalOpenGL().m_glGenProgramsNV
#define glGetProgramParameterdvNV GlobalOpenGL().m_glGetProgramParameterdvNV
#define glGetProgramParameterfvNV GlobalOpenGL().m_glGetProgramParameterfvNV
#define glGetProgramivNV GlobalOpenGL().m_glGetProgramivNV
#define glGetProgramStringNV GlobalOpenGL().m_glGetProgramStringNV
#define glGetTrackMatrixivNV GlobalOpenGL().m_glGetTrackMatrixivNV
#define glGetVertexAttribdvNV GlobalOpenGL().m_glGetVertexAttribdvNV
#define glGetVertexAttribfvNV GlobalOpenGL().m_glGetVertexAttribfvNV
#define glGetVertexAttribivNV GlobalOpenGL().m_glGetVertexAttribivNV
#define glGetVertexAttribPointervNV GlobalOpenGL().m_glGetVertexAttribPointervNV
#define glIsProgramNV GlobalOpenGL().m_glIsProgramNV
#define glLoadProgramNV GlobalOpenGL().m_glLoadProgramNV
#define glProgramParameter4fNV GlobalOpenGL().m_glProgramParameter4fNV
#define glProgramParameter4fvNV GlobalOpenGL().m_glProgramParameter4fvNV
#define glProgramParameters4fvNV GlobalOpenGL().m_glProgramParameters4fvNV
#define glRequestResidentProgramsNV GlobalOpenGL().m_glRequestResidentProgramsNV
#define glTrackMatrixNV GlobalOpenGL().m_glTrackMatrixNV
#define glVertexAttribPointerNV GlobalOpenGL().m_glVertexAttribPointerNV
#define glVertexAttrib1fNV GlobalOpenGL().m_glVertexAttrib1fNV
#define glVertexAttrib1fvNV GlobalOpenGL().m_glVertexAttrib1fvNV
#define glVertexAttrib2fNV GlobalOpenGL().m_glVertexAttrib2fNV
#define glVertexAttrib2fvNV GlobalOpenGL().m_glVertexAttrib2fvNV
#define glVertexAttrib3fNV GlobalOpenGL().m_glVertexAttrib3fNV
#define glVertexAttrib3fvNV GlobalOpenGL().m_glVertexAttrib3fvNV
#define glVertexAttrib4fNV GlobalOpenGL().m_glVertexAttrib4fNV
#define glVertexAttrib4fvNV GlobalOpenGL().m_glVertexAttrib4fvNV
#define glVertexAttribs1fvNV GlobalOpenGL().m_glVertexAttribs1fvNV
#define glVertexAttribs2fvNV GlobalOpenGL().m_glVertexAttribs2fvNV
#define glVertexAttribs3fvNV GlobalOpenGL().m_glVertexAttribs3fvNV
#define glVertexAttribs4fvNV GlobalOpenGL().m_glVertexAttribs4fvNV

#endif


// GL_NV_fragment_program
#if !defined(GL_NV_fragment_program)

#define GL_NV_fragment_program 1

#define GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV 0x8868
#define GL_FRAGMENT_PROGRAM_NV 0x8870
#define GL_MAX_TEXTURE_COORDS_NV 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_NV 0x8872
#define GL_FRAGMENT_PROGRAM_BINDING_NV 0x8873
#define GL_PROGRAM_ERROR_STRING_NV 0x8874

#define glProgramNamedParameter4fNV GlobalOpenGL().m_glProgramNamedParameter4fNV
#define glProgramNamedParameter4fvNV GlobalOpenGL().m_glProgramNamedParameter4fvNV
#define glGetProgramNamedParameterfvNV GlobalOpenGL().m_glGetProgramNamedParameterfvNV

#endif


/// \brief A module which wraps a runtime-binding of the standard OpenGL functions.
/// Provides convenience functions for querying availabiliy of extensions, rendering text and error-checking.
struct OpenGLBinding
{
  INTEGER_CONSTANT(Version, 2);
  STRING_CONSTANT(Name, "qgl");

  /// \brief OpenGL version, extracted from the GL_VERSION string.
  int major_version, minor_version;

  /// \brief Is true if the global shared OpenGL context is valid.
  bool contextValid;

  OpenGLBinding() : contextValid(false)
  {
  }

  /// \brief Asserts that there no OpenGL errors have occurred since the last call to glGetError.
  void (*assertNoErrors)();

  GLuint m_font;
  int m_fontHeight;

  /// \brief Renders \p string at the current raster-position of the current context.
  void drawString(const char* string) const
  {
    m_glListBase(m_font);
    m_glCallLists(GLsizei(strlen(string)), GL_UNSIGNED_BYTE, reinterpret_cast<const GLubyte*>(string));
  }

  /// \brief Renders \p character at the current raster-position of the current context.
  void drawChar(char character) const
  {
    m_glListBase(m_font);
    m_glCallLists(1, GL_UNSIGNED_BYTE, reinterpret_cast<const GLubyte*>(&character));
  }


  // GL 1.1
  void (QGL_DLLEXPORT *m_glAccum)(GLenum op, GLfloat value);
  void (QGL_DLLEXPORT *m_glAlphaFunc)(GLenum func, GLclampf ref);
  GLboolean (QGL_DLLEXPORT *m_glAreTexturesResident)(GLsizei n, const GLuint *textures, GLboolean *residences);
  void (QGL_DLLEXPORT *m_glArrayElement)(GLint i);
  void (QGL_DLLEXPORT *m_glBegin)(GLenum mode);
  void (QGL_DLLEXPORT *m_glBindTexture)(GLenum target, GLuint texture);
  void (QGL_DLLEXPORT *m_glBitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
  void (QGL_DLLEXPORT *m_glBlendFunc)(GLenum sfactor, GLenum dfactor);
  void (QGL_DLLEXPORT *m_glCallList)(GLuint list);
  void (QGL_DLLEXPORT *m_glCallLists)(GLsizei n, GLenum type, const GLvoid *lists);
  void (QGL_DLLEXPORT *m_glClear)(GLbitfield mask);
  void (QGL_DLLEXPORT *m_glClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
  void (QGL_DLLEXPORT *m_glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
  void (QGL_DLLEXPORT *m_glClearDepth)(GLclampd depth);
  void (QGL_DLLEXPORT *m_glClearIndex)(GLfloat c);
  void (QGL_DLLEXPORT *m_glClearStencil)(GLint s);
  void (QGL_DLLEXPORT *m_glClipPlane)(GLenum plane, const GLdouble *equation);
  void (QGL_DLLEXPORT *m_glColor3b)(GLbyte red, GLbyte green, GLbyte blue);
  void (QGL_DLLEXPORT *m_glColor3bv)(const GLbyte *v);
  void (QGL_DLLEXPORT *m_glColor3d)(GLdouble red, GLdouble green, GLdouble blue);
  void (QGL_DLLEXPORT *m_glColor3dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glColor3f)(GLfloat red, GLfloat green, GLfloat blue);
  void (QGL_DLLEXPORT *m_glColor3fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glColor3i)(GLint red, GLint green, GLint blue);
  void (QGL_DLLEXPORT *m_glColor3iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glColor3s)(GLshort red, GLshort green, GLshort blue);
  void (QGL_DLLEXPORT *m_glColor3sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
  void (QGL_DLLEXPORT *m_glColor3ubv)(const GLubyte *v);
  void (QGL_DLLEXPORT *m_glColor3ui)(GLuint red, GLuint green, GLuint blue);
  void (QGL_DLLEXPORT *m_glColor3uiv)(const GLuint *v);
  void (QGL_DLLEXPORT *m_glColor3us)(GLushort red, GLushort green, GLushort blue);
  void (QGL_DLLEXPORT *m_glColor3usv)(const GLushort *v);
  void (QGL_DLLEXPORT *m_glColor4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
  void (QGL_DLLEXPORT *m_glColor4bv)(const GLbyte *v);
  void (QGL_DLLEXPORT *m_glColor4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
  void (QGL_DLLEXPORT *m_glColor4dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
  void (QGL_DLLEXPORT *m_glColor4fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glColor4i)(GLint red, GLint green, GLint blue, GLint alpha);
  void (QGL_DLLEXPORT *m_glColor4iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glColor4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
  void (QGL_DLLEXPORT *m_glColor4sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glColor4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
  void (QGL_DLLEXPORT *m_glColor4ubv)(const GLubyte *v);
  void (QGL_DLLEXPORT *m_glColor4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
  void (QGL_DLLEXPORT *m_glColor4uiv)(const GLuint *v);
  void (QGL_DLLEXPORT *m_glColor4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
  void (QGL_DLLEXPORT *m_glColor4usv)(const GLushort *v);
  void (QGL_DLLEXPORT *m_glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
  void (QGL_DLLEXPORT *m_glColorMaterial)(GLenum face, GLenum mode);
  void (QGL_DLLEXPORT *m_glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
  void (QGL_DLLEXPORT *m_glCopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
  void (QGL_DLLEXPORT *m_glCopyTexImage1D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
  void (QGL_DLLEXPORT *m_glCopyTexImage2D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
  void (QGL_DLLEXPORT *m_glCopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
  void (QGL_DLLEXPORT *m_glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
  void (QGL_DLLEXPORT *m_glCullFace)(GLenum mode);
  void (QGL_DLLEXPORT *m_glDeleteLists)(GLuint list, GLsizei range);
  void (QGL_DLLEXPORT *m_glDeleteTextures)(GLsizei n, const GLuint *textures);
  void (QGL_DLLEXPORT *m_glDepthFunc)(GLenum func);
  void (QGL_DLLEXPORT *m_glDepthMask)(GLboolean flag);
  void (QGL_DLLEXPORT *m_glDepthRange)(GLclampd zNear, GLclampd zFar);
  void (QGL_DLLEXPORT *m_glDisable)(GLenum cap);
  void (QGL_DLLEXPORT *m_glDisableClientState)(GLenum array);
  void (QGL_DLLEXPORT *m_glDrawArrays)(GLenum mode, GLint first, GLsizei count);
  void (QGL_DLLEXPORT *m_glDrawBuffer)(GLenum mode);
  void (QGL_DLLEXPORT *m_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
  void (QGL_DLLEXPORT *m_glDrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
  void (QGL_DLLEXPORT *m_glEdgeFlag)(GLboolean flag);
  void (QGL_DLLEXPORT *m_glEdgeFlagPointer)(GLsizei stride, const GLvoid *pointer);
  void (QGL_DLLEXPORT *m_glEdgeFlagv)(const GLboolean *flag);
  void (QGL_DLLEXPORT *m_glEnable)(GLenum cap);
  void (QGL_DLLEXPORT *m_glEnableClientState)(GLenum array);
  void (QGL_DLLEXPORT *m_glEnd)(void);
  void (QGL_DLLEXPORT *m_glEndList)(void);
  void (QGL_DLLEXPORT *m_glEvalCoord1d)(GLdouble u);
  void (QGL_DLLEXPORT *m_glEvalCoord1dv)(const GLdouble *u);
  void (QGL_DLLEXPORT *m_glEvalCoord1f)(GLfloat u);
  void (QGL_DLLEXPORT *m_glEvalCoord1fv)(const GLfloat *u);
  void (QGL_DLLEXPORT *m_glEvalCoord2d)(GLdouble u, GLdouble v);
  void (QGL_DLLEXPORT *m_glEvalCoord2dv)(const GLdouble *u);
  void (QGL_DLLEXPORT *m_glEvalCoord2f)(GLfloat u, GLfloat v);
  void (QGL_DLLEXPORT *m_glEvalCoord2fv)(const GLfloat *u);
  void (QGL_DLLEXPORT *m_glEvalMesh1)(GLenum mode, GLint i1, GLint i2);
  void (QGL_DLLEXPORT *m_glEvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
  void (QGL_DLLEXPORT *m_glEvalPoint1)(GLint i);
  void (QGL_DLLEXPORT *m_glEvalPoint2)(GLint i, GLint j);
  void (QGL_DLLEXPORT *m_glFeedbackBuffer)(GLsizei size, GLenum type, GLfloat *buffer);
  void (QGL_DLLEXPORT *m_glFinish)(void);
  void (QGL_DLLEXPORT *m_glFlush)(void);
  void (QGL_DLLEXPORT *m_glFogf)(GLenum pname, GLfloat param);
  void (QGL_DLLEXPORT *m_glFogfv)(GLenum pname, const GLfloat *params);
  void (QGL_DLLEXPORT *m_glFogi)(GLenum pname, GLint param);
  void (QGL_DLLEXPORT *m_glFogiv)(GLenum pname, const GLint *params);
  void (QGL_DLLEXPORT *m_glFrontFace)(GLenum mode);
  void (QGL_DLLEXPORT *m_glFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
  GLuint (QGL_DLLEXPORT *m_glGenLists)(GLsizei range);
  void (QGL_DLLEXPORT *m_glGenTextures)(GLsizei n, GLuint *textures);
  void (QGL_DLLEXPORT *m_glGetBooleanv)(GLenum pname, GLboolean *params);
  void (QGL_DLLEXPORT *m_glGetClipPlane)(GLenum plane, GLdouble *equation);
  void (QGL_DLLEXPORT *m_glGetDoublev)(GLenum pname, GLdouble *params);
  GLenum (QGL_DLLEXPORT *m_glGetError)(void);
  void (QGL_DLLEXPORT *m_glGetFloatv)(GLenum pname, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetIntegerv)(GLenum pname, GLint *params);
  void (QGL_DLLEXPORT *m_glGetLightfv)(GLenum light, GLenum pname, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetLightiv)(GLenum light, GLenum pname, GLint *params);
  void (QGL_DLLEXPORT *m_glGetMapdv)(GLenum target, GLenum query, GLdouble *v);
  void (QGL_DLLEXPORT *m_glGetMapfv)(GLenum target, GLenum query, GLfloat *v);
  void (QGL_DLLEXPORT *m_glGetMapiv)(GLenum target, GLenum query, GLint *v);
  void (QGL_DLLEXPORT *m_glGetMaterialfv)(GLenum face, GLenum pname, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetMaterialiv)(GLenum face, GLenum pname, GLint *params);
  void (QGL_DLLEXPORT *m_glGetPixelMapfv)(GLenum map, GLfloat *values);
  void (QGL_DLLEXPORT *m_glGetPixelMapuiv)(GLenum map, GLuint *values);
  void (QGL_DLLEXPORT *m_glGetPixelMapusv)(GLenum map, GLushort *values);
  void (QGL_DLLEXPORT *m_glGetPointerv)(GLenum pname, GLvoid* *params);
  void (QGL_DLLEXPORT *m_glGetPolygonStipple)(GLubyte *mask);
  const GLubyte * (QGL_DLLEXPORT *m_glGetString)(GLenum name);
  void (QGL_DLLEXPORT *m_glGetTexEnvfv)(GLenum target, GLenum pname, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetTexEnviv)(GLenum target, GLenum pname, GLint *params);
  void (QGL_DLLEXPORT *m_glGetTexGendv)(GLenum coord, GLenum pname, GLdouble *params);
  void (QGL_DLLEXPORT *m_glGetTexGenfv)(GLenum coord, GLenum pname, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetTexGeniv)(GLenum coord, GLenum pname, GLint *params);
  void (QGL_DLLEXPORT *m_glGetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
  void (QGL_DLLEXPORT *m_glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params);
  void (QGL_DLLEXPORT *m_glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
  void (QGL_DLLEXPORT *m_glHint)(GLenum target, GLenum mode);
  void (QGL_DLLEXPORT *m_glIndexMask)(GLuint mask);
  void (QGL_DLLEXPORT *m_glIndexPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
  void (QGL_DLLEXPORT *m_glIndexd)(GLdouble c);
  void (QGL_DLLEXPORT *m_glIndexdv)(const GLdouble *c);
  void (QGL_DLLEXPORT *m_glIndexf)(GLfloat c);
  void (QGL_DLLEXPORT *m_glIndexfv)(const GLfloat *c);
  void (QGL_DLLEXPORT *m_glIndexi)(GLint c);
  void (QGL_DLLEXPORT *m_glIndexiv)(const GLint *c);
  void (QGL_DLLEXPORT *m_glIndexs)(GLshort c);
  void (QGL_DLLEXPORT *m_glIndexsv)(const GLshort *c);
  void (QGL_DLLEXPORT *m_glIndexub)(GLubyte c);
  void (QGL_DLLEXPORT *m_glIndexubv)(const GLubyte *c);
  void (QGL_DLLEXPORT *m_glInitNames)(void);
  void (QGL_DLLEXPORT *m_glInterleavedArrays)(GLenum format, GLsizei stride, const GLvoid *pointer);
  GLboolean (QGL_DLLEXPORT *m_glIsEnabled)(GLenum cap);
  GLboolean (QGL_DLLEXPORT *m_glIsList)(GLuint list);
  GLboolean (QGL_DLLEXPORT *m_glIsTexture)(GLuint texture);
  void (QGL_DLLEXPORT *m_glLightModelf)(GLenum pname, GLfloat param);
  void (QGL_DLLEXPORT *m_glLightModelfv)(GLenum pname, const GLfloat *params);
  void (QGL_DLLEXPORT *m_glLightModeli)(GLenum pname, GLint param);
  void (QGL_DLLEXPORT *m_glLightModeliv)(GLenum pname, const GLint *params);
  void (QGL_DLLEXPORT *m_glLightf)(GLenum light, GLenum pname, GLfloat param);
  void (QGL_DLLEXPORT *m_glLightfv)(GLenum light, GLenum pname, const GLfloat *params);
  void (QGL_DLLEXPORT *m_glLighti)(GLenum light, GLenum pname, GLint param);
  void (QGL_DLLEXPORT *m_glLightiv)(GLenum light, GLenum pname, const GLint *params);
  void (QGL_DLLEXPORT *m_glLineStipple)(GLint factor, GLushort pattern);
  void (QGL_DLLEXPORT *m_glLineWidth)(GLfloat width);
  void (QGL_DLLEXPORT *m_glListBase)(GLuint base);
  void (QGL_DLLEXPORT *m_glLoadIdentity)(void);
  void (QGL_DLLEXPORT *m_glLoadMatrixd)(const GLdouble *m);
  void (QGL_DLLEXPORT *m_glLoadMatrixf)(const GLfloat *m);
  void (QGL_DLLEXPORT *m_glLoadName)(GLuint name);
  void (QGL_DLLEXPORT *m_glLogicOp)(GLenum opcode);
  void (QGL_DLLEXPORT *m_glMap1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
  void (QGL_DLLEXPORT *m_glMap1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
  void (QGL_DLLEXPORT *m_glMap2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
  void (QGL_DLLEXPORT *m_glMap2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
  void (QGL_DLLEXPORT *m_glMapGrid1d)(GLint un, GLdouble u1, GLdouble u2);
  void (QGL_DLLEXPORT *m_glMapGrid1f)(GLint un, GLfloat u1, GLfloat u2);
  void (QGL_DLLEXPORT *m_glMapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
  void (QGL_DLLEXPORT *m_glMapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
  void (QGL_DLLEXPORT *m_glMaterialf)(GLenum face, GLenum pname, GLfloat param);
  void (QGL_DLLEXPORT *m_glMaterialfv)(GLenum face, GLenum pname, const GLfloat *params);
  void (QGL_DLLEXPORT *m_glMateriali)(GLenum face, GLenum pname, GLint param);
  void (QGL_DLLEXPORT *m_glMaterialiv)(GLenum face, GLenum pname, const GLint *params);
  void (QGL_DLLEXPORT *m_glMatrixMode)(GLenum mode);
  void (QGL_DLLEXPORT *m_glMultMatrixd)(const GLdouble *m);
  void (QGL_DLLEXPORT *m_glMultMatrixf)(const GLfloat *m);
  void (QGL_DLLEXPORT *m_glNewList)(GLuint list, GLenum mode);
  void (QGL_DLLEXPORT *m_glNormal3b)(GLbyte nx, GLbyte ny, GLbyte nz);
  void (QGL_DLLEXPORT *m_glNormal3bv)(const GLbyte *v);
  void (QGL_DLLEXPORT *m_glNormal3d)(GLdouble nx, GLdouble ny, GLdouble nz);
  void (QGL_DLLEXPORT *m_glNormal3dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glNormal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
  void (QGL_DLLEXPORT *m_glNormal3fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glNormal3i)(GLint nx, GLint ny, GLint nz);
  void (QGL_DLLEXPORT *m_glNormal3iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glNormal3s)(GLshort nx, GLshort ny, GLshort nz);
  void (QGL_DLLEXPORT *m_glNormal3sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glNormalPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
  void (QGL_DLLEXPORT *m_glOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
  void (QGL_DLLEXPORT *m_glPassThrough)(GLfloat token);
  void (QGL_DLLEXPORT *m_glPixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat *values);
  void (QGL_DLLEXPORT *m_glPixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint *values);
  void (QGL_DLLEXPORT *m_glPixelMapusv)(GLenum map, GLsizei mapsize, const GLushort *values);
  void (QGL_DLLEXPORT *m_glPixelStoref)(GLenum pname, GLfloat param);
  void (QGL_DLLEXPORT *m_glPixelStorei)(GLenum pname, GLint param);
  void (QGL_DLLEXPORT *m_glPixelTransferf)(GLenum pname, GLfloat param);
  void (QGL_DLLEXPORT *m_glPixelTransferi)(GLenum pname, GLint param);
  void (QGL_DLLEXPORT *m_glPixelZoom)(GLfloat xfactor, GLfloat yfactor);
  void (QGL_DLLEXPORT *m_glPointSize)(GLfloat size);
  void (QGL_DLLEXPORT *m_glPolygonMode)(GLenum face, GLenum mode);
  void (QGL_DLLEXPORT *m_glPolygonOffset)(GLfloat factor, GLfloat units);
  void (QGL_DLLEXPORT *m_glPolygonStipple)(const GLubyte *mask);
  void (QGL_DLLEXPORT *m_glPopAttrib)(void);
  void (QGL_DLLEXPORT *m_glPopClientAttrib)(void);
  void (QGL_DLLEXPORT *m_glPopMatrix)(void);
  void (QGL_DLLEXPORT *m_glPopName)(void);
  void (QGL_DLLEXPORT *m_glPrioritizeTextures)(GLsizei n, const GLuint *textures, const GLclampf *priorities);
  void (QGL_DLLEXPORT *m_glPushAttrib)(GLbitfield mask);
  void (QGL_DLLEXPORT *m_glPushClientAttrib)(GLbitfield mask);
  void (QGL_DLLEXPORT *m_glPushMatrix)(void);
  void (QGL_DLLEXPORT *m_glPushName)(GLuint name);
  void (QGL_DLLEXPORT *m_glRasterPos2d)(GLdouble x, GLdouble y);
  void (QGL_DLLEXPORT *m_glRasterPos2dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glRasterPos2f)(GLfloat x, GLfloat y);
  void (QGL_DLLEXPORT *m_glRasterPos2fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glRasterPos2i)(GLint x, GLint y);
  void (QGL_DLLEXPORT *m_glRasterPos2iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glRasterPos2s)(GLshort x, GLshort y);
  void (QGL_DLLEXPORT *m_glRasterPos2sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glRasterPos3d)(GLdouble x, GLdouble y, GLdouble z);
  void (QGL_DLLEXPORT *m_glRasterPos3dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glRasterPos3f)(GLfloat x, GLfloat y, GLfloat z);
  void (QGL_DLLEXPORT *m_glRasterPos3fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glRasterPos3i)(GLint x, GLint y, GLint z);
  void (QGL_DLLEXPORT *m_glRasterPos3iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glRasterPos3s)(GLshort x, GLshort y, GLshort z);
  void (QGL_DLLEXPORT *m_glRasterPos3sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glRasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
  void (QGL_DLLEXPORT *m_glRasterPos4dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glRasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
  void (QGL_DLLEXPORT *m_glRasterPos4fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glRasterPos4i)(GLint x, GLint y, GLint z, GLint w);
  void (QGL_DLLEXPORT *m_glRasterPos4iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glRasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w);
  void (QGL_DLLEXPORT *m_glRasterPos4sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glReadBuffer)(GLenum mode);
  void (QGL_DLLEXPORT *m_glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
  void (QGL_DLLEXPORT *m_glRectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
  void (QGL_DLLEXPORT *m_glRectdv)(const GLdouble *v1, const GLdouble *v2);
  void (QGL_DLLEXPORT *m_glRectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
  void (QGL_DLLEXPORT *m_glRectfv)(const GLfloat *v1, const GLfloat *v2);
  void (QGL_DLLEXPORT *m_glRecti)(GLint x1, GLint y1, GLint x2, GLint y2);
  void (QGL_DLLEXPORT *m_glRectiv)(const GLint *v1, const GLint *v2);
  void (QGL_DLLEXPORT *m_glRects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
  void (QGL_DLLEXPORT *m_glRectsv)(const GLshort *v1, const GLshort *v2);
  GLint (QGL_DLLEXPORT *m_glRenderMode)(GLenum mode);
  void (QGL_DLLEXPORT *m_glRotated)(GLdouble anm_gle, GLdouble x, GLdouble y, GLdouble z);
  void (QGL_DLLEXPORT *m_glRotatef)(GLfloat anm_gle, GLfloat x, GLfloat y, GLfloat z);
  void (QGL_DLLEXPORT *m_glScaled)(GLdouble x, GLdouble y, GLdouble z);
  void (QGL_DLLEXPORT *m_glScalef)(GLfloat x, GLfloat y, GLfloat z);
  void (QGL_DLLEXPORT *m_glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
  void (QGL_DLLEXPORT *m_glSelectBuffer)(GLsizei size, GLuint *buffer);
  void (QGL_DLLEXPORT *m_glShadeModel)(GLenum mode);
  void (QGL_DLLEXPORT *m_glStencilFunc)(GLenum func, GLint ref, GLuint mask);
  void (QGL_DLLEXPORT *m_glStencilMask)(GLuint mask);
  void (QGL_DLLEXPORT *m_glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
  void (QGL_DLLEXPORT *m_glTexCoord1d)(GLdouble s);
  void (QGL_DLLEXPORT *m_glTexCoord1dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glTexCoord1f)(GLfloat s);
  void (QGL_DLLEXPORT *m_glTexCoord1fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glTexCoord1i)(GLint s);
  void (QGL_DLLEXPORT *m_glTexCoord1iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glTexCoord1s)(GLshort s);
  void (QGL_DLLEXPORT *m_glTexCoord1sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glTexCoord2d)(GLdouble s, GLdouble t);
  void (QGL_DLLEXPORT *m_glTexCoord2dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glTexCoord2f)(GLfloat s, GLfloat t);
  void (QGL_DLLEXPORT *m_glTexCoord2fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glTexCoord2i)(GLint s, GLint t);
  void (QGL_DLLEXPORT *m_glTexCoord2iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glTexCoord2s)(GLshort s, GLshort t);
  void (QGL_DLLEXPORT *m_glTexCoord2sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glTexCoord3d)(GLdouble s, GLdouble t, GLdouble r);
  void (QGL_DLLEXPORT *m_glTexCoord3dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glTexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
  void (QGL_DLLEXPORT *m_glTexCoord3fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glTexCoord3i)(GLint s, GLint t, GLint r);
  void (QGL_DLLEXPORT *m_glTexCoord3iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glTexCoord3s)(GLshort s, GLshort t, GLshort r);
  void (QGL_DLLEXPORT *m_glTexCoord3sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glTexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
  void (QGL_DLLEXPORT *m_glTexCoord4dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glTexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
  void (QGL_DLLEXPORT *m_glTexCoord4fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glTexCoord4i)(GLint s, GLint t, GLint r, GLint q);
  void (QGL_DLLEXPORT *m_glTexCoord4iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glTexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q);
  void (QGL_DLLEXPORT *m_glTexCoord4sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
  void (QGL_DLLEXPORT *m_glTexEnvf)(GLenum target, GLenum pname, GLfloat param);
  void (QGL_DLLEXPORT *m_glTexEnvfv)(GLenum target, GLenum pname, const GLfloat *params);
  void (QGL_DLLEXPORT *m_glTexEnvi)(GLenum target, GLenum pname, GLint param);
  void (QGL_DLLEXPORT *m_glTexEnviv)(GLenum target, GLenum pname, const GLint *params);
  void (QGL_DLLEXPORT *m_glTexGend)(GLenum coord, GLenum pname, GLdouble param);
  void (QGL_DLLEXPORT *m_glTexGendv)(GLenum coord, GLenum pname, const GLdouble *params);
  void (QGL_DLLEXPORT *m_glTexGenf)(GLenum coord, GLenum pname, GLfloat param);
  void (QGL_DLLEXPORT *m_glTexGenfv)(GLenum coord, GLenum pname, const GLfloat *params);
  void (QGL_DLLEXPORT *m_glTexGeni)(GLenum coord, GLenum pname, GLint param);
  void (QGL_DLLEXPORT *m_glTexGeniv)(GLenum coord, GLenum pname, const GLint *params);
  void (QGL_DLLEXPORT *m_glTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
  void (QGL_DLLEXPORT *m_glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
  void (QGL_DLLEXPORT *m_glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
  void (QGL_DLLEXPORT *m_glTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
  void (QGL_DLLEXPORT *m_glTexParameteri)(GLenum target, GLenum pname, GLint param);
  void (QGL_DLLEXPORT *m_glTexParameteriv)(GLenum target, GLenum pname, const GLint *params);
  void (QGL_DLLEXPORT *m_glTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
  void (QGL_DLLEXPORT *m_glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
  void (QGL_DLLEXPORT *m_glTranslated)(GLdouble x, GLdouble y, GLdouble z);
  void (QGL_DLLEXPORT *m_glTranslatef)(GLfloat x, GLfloat y, GLfloat z);
  void (QGL_DLLEXPORT *m_glVertex2d)(GLdouble x, GLdouble y);
  void (QGL_DLLEXPORT *m_glVertex2dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glVertex2f)(GLfloat x, GLfloat y);
  void (QGL_DLLEXPORT *m_glVertex2fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glVertex2i)(GLint x, GLint y);
  void (QGL_DLLEXPORT *m_glVertex2iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glVertex2s)(GLshort x, GLshort y);
  void (QGL_DLLEXPORT *m_glVertex2sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertex3d)(GLdouble x, GLdouble y, GLdouble z);
  void (QGL_DLLEXPORT *m_glVertex3dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glVertex3f)(GLfloat x, GLfloat y, GLfloat z);
  void (QGL_DLLEXPORT *m_glVertex3fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glVertex3i)(GLint x, GLint y, GLint z);
  void (QGL_DLLEXPORT *m_glVertex3iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glVertex3s)(GLshort x, GLshort y, GLshort z);
  void (QGL_DLLEXPORT *m_glVertex3sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
  void (QGL_DLLEXPORT *m_glVertex4dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glVertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
  void (QGL_DLLEXPORT *m_glVertex4fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glVertex4i)(GLint x, GLint y, GLint z, GLint w);
  void (QGL_DLLEXPORT *m_glVertex4iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glVertex4s)(GLshort x, GLshort y, GLshort z, GLshort w);
  void (QGL_DLLEXPORT *m_glVertex4sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
  void (QGL_DLLEXPORT *m_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

  // GL_ARB_multitexture
  bool support_ARB_multitexture;
  bool ARB_multitexture()
  {
    return support_ARB_multitexture;
  }
  void (QGL_DLLEXPORT *m_glActiveTextureARB)(GLenum texture);
  void (QGL_DLLEXPORT *m_glClientActiveTextureARB)(GLenum texture);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1dARB)(GLenum target, GLdouble s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1dvARB)(GLenum target, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1fARB)(GLenum target, GLfloat s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1fvARB)(GLenum target, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1iARB)(GLenum target, GLint s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1ivARB)(GLenum target, const GLint *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1sARB)(GLenum target, GLshort s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1svARB)(GLenum target, const GLshort *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2dARB)(GLenum target, GLdouble s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2dvARB)(GLenum target, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2fARB)(GLenum target, GLfloat s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2fvARB)(GLenum target, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2iARB)(GLenum target, GLint s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2ivARB)(GLenum target, const GLint *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2sARB)(GLenum target, GLshort s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2svARB)(GLenum target, const GLshort *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3dARB)(GLenum target, GLdouble s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3dvARB)(GLenum target, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3fARB)(GLenum target, GLfloat s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3fvARB)(GLenum target, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3iARB)(GLenum target, GLint s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3ivARB)(GLenum target, const GLint *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3sARB)(GLenum target, GLshort s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3svARB)(GLenum target, const GLshort *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4dARB)(GLenum target, GLdouble s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4dvARB)(GLenum target, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4fARB)(GLenum target, GLfloat s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4fvARB)(GLenum target, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4iARB)(GLenum target, GLint s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4ivARB)(GLenum target, const GLint *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4sARB)(GLenum target, GLshort s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4svARB)(GLenum target, const GLshort *v);

  // ARB_texture_compression
  bool support_ARB_texture_compression;
  bool ARB_texture_compression()
  {
    return support_ARB_texture_compression;
  }
  void (QGL_DLLEXPORT *m_glCompressedTexImage3DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
  void (QGL_DLLEXPORT *m_glCompressedTexImage2DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
  void (QGL_DLLEXPORT *m_glCompressedTexImage1DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid* data);
  void (QGL_DLLEXPORT *m_glCompressedTexSubImage3DARB)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
  void (QGL_DLLEXPORT *m_glCompressedTexSubImage2DARB)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
  void (QGL_DLLEXPORT *m_glCompressedTexSubImage1DARB)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid* data);
  void (QGL_DLLEXPORT *m_glGetCompressedTexImageARB)(GLenum target, GLint lod, GLvoid* img);

  // EXT_texture_compression_s3tc
  bool support_EXT_texture_compression_s3tc;
  bool EXT_texture_compression_s3tc()
  {
    return support_EXT_texture_compression_s3tc;
  }

  // GL 1.2
  bool support_GL_1_2;
  bool GL_1_2()
  {
    return support_GL_1_2;
  }
  void (QGL_DLLEXPORT *m_glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
  void (QGL_DLLEXPORT *m_glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
  void (QGL_DLLEXPORT *m_glTexImage3D)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
  void (QGL_DLLEXPORT *m_glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);

  // GL 1.3
  bool support_GL_1_3;
  bool GL_1_3()
  {
    return support_GL_1_3;
  }
  void (QGL_DLLEXPORT *m_glActiveTexture)(GLenum texture);
  void (QGL_DLLEXPORT *m_glClientActiveTexture)(GLenum texture);
  void (QGL_DLLEXPORT *m_glCompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
  void (QGL_DLLEXPORT *m_glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
  void (QGL_DLLEXPORT *m_glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
  void (QGL_DLLEXPORT *m_glCompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
  void (QGL_DLLEXPORT *m_glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
  void (QGL_DLLEXPORT *m_glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
  void (QGL_DLLEXPORT *m_glGetCompressedTexImage)(GLenum target, GLint lod, GLvoid *img);
  void (QGL_DLLEXPORT *m_glLoadTransposeMatrixd)(const GLdouble m[16]);
  void (QGL_DLLEXPORT *m_glLoadTransposeMatrixf)(const GLfloat m[16]);
  void (QGL_DLLEXPORT *m_glMultTransposeMatrixd)(const GLdouble m[16]);
  void (QGL_DLLEXPORT *m_glMultTransposeMatrixf)(const GLfloat m[16]);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1d)(GLenum target, GLdouble s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1dv)(GLenum target, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1f)(GLenum target, GLfloat s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1fv)(GLenum target, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1i)(GLenum target, GLint s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1iv)(GLenum target, const GLint *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1s)(GLenum target, GLshort s);
  void (QGL_DLLEXPORT *m_glMultiTexCoord1sv)(GLenum target, const GLshort *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2dv)(GLenum target, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2f)(GLenum target, GLfloat s, GLfloat t);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2fv)(GLenum target, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2i)(GLenum target, GLint s, GLint t);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2iv)(GLenum target, const GLint *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2s)(GLenum target, GLshort s, GLshort t);
  void (QGL_DLLEXPORT *m_glMultiTexCoord2sv)(GLenum target, const GLshort *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3dv)(GLenum target, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3f)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3fv)(GLenum target, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3iv)(GLenum target, const GLint *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r);
  void (QGL_DLLEXPORT *m_glMultiTexCoord3sv)(GLenum target, const GLshort *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4dv)(GLenum target, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4fv)(GLenum target, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4iv)(GLenum target, const GLint *v);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
  void (QGL_DLLEXPORT *m_glMultiTexCoord4sv)(GLenum target, const GLshort *v);
  void (QGL_DLLEXPORT *m_glSampleCoverage)(GLclampf value, GLboolean invert);

  // GL 1.4
  bool support_GL_1_4;
  bool GL_1_4()
  {
    return support_GL_1_4;
  }
  void (QGL_DLLEXPORT *m_glBlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
  void (QGL_DLLEXPORT *m_glBlendEquation)(GLenum mode);
  void (QGL_DLLEXPORT *m_glBlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
  void (QGL_DLLEXPORT *m_glFogCoordPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
  void (QGL_DLLEXPORT *m_glFogCoordd)(GLdouble coord);
  void (QGL_DLLEXPORT *m_glFogCoorddv)(const GLdouble *coord);
  void (QGL_DLLEXPORT *m_glFogCoordf)(GLfloat coord);
  void (QGL_DLLEXPORT *m_glFogCoordfv)(const GLfloat *coord);
  void (QGL_DLLEXPORT *m_glMultiDrawArrays)(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
  void (QGL_DLLEXPORT *m_glMultiDrawElements)(GLenum mode, GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount);
  void (QGL_DLLEXPORT *m_glPointParameterf)(GLenum pname, GLfloat param);
  void (QGL_DLLEXPORT *m_glPointParameterfv)(GLenum pname, GLfloat *params);
  void (QGL_DLLEXPORT *m_glSecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue);
  void (QGL_DLLEXPORT *m_glSecondaryColor3bv)(const GLbyte *v);
  void (QGL_DLLEXPORT *m_glSecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue);
  void (QGL_DLLEXPORT *m_glSecondaryColor3dv)(const GLdouble *v);
  void (QGL_DLLEXPORT *m_glSecondaryColor3f)(GLfloat red, GLfloat green, GLfloat blue);
  void (QGL_DLLEXPORT *m_glSecondaryColor3fv)(const GLfloat *v);
  void (QGL_DLLEXPORT *m_glSecondaryColor3i)(GLint red, GLint green, GLint blue);
  void (QGL_DLLEXPORT *m_glSecondaryColor3iv)(const GLint *v);
  void (QGL_DLLEXPORT *m_glSecondaryColor3s)(GLshort red, GLshort green, GLshort blue);
  void (QGL_DLLEXPORT *m_glSecondaryColor3sv)(const GLshort *v);
  void (QGL_DLLEXPORT *m_glSecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
  void (QGL_DLLEXPORT *m_glSecondaryColor3ubv)(const GLubyte *v);
  void (QGL_DLLEXPORT *m_glSecondaryColor3ui)(GLuint red, GLuint green, GLuint blue);
  void (QGL_DLLEXPORT *m_glSecondaryColor3uiv)(const GLuint *v);
  void (QGL_DLLEXPORT *m_glSecondaryColor3us)(GLushort red, GLushort green, GLushort blue);
  void (QGL_DLLEXPORT *m_glSecondaryColor3usv)(const GLushort *v);
  void (QGL_DLLEXPORT *m_glSecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, GLvoid *pointer);
  void (QGL_DLLEXPORT *m_glWindowPos2d)(GLdouble x, GLdouble y);
  void (QGL_DLLEXPORT *m_glWindowPos2dv)(const GLdouble *p);
  void (QGL_DLLEXPORT *m_glWindowPos2f)(GLfloat x, GLfloat y);
  void (QGL_DLLEXPORT *m_glWindowPos2fv)(const GLfloat *p);
  void (QGL_DLLEXPORT *m_glWindowPos2i)(GLint x, GLint y);
  void (QGL_DLLEXPORT *m_glWindowPos2iv)(const GLint *p);
  void (QGL_DLLEXPORT *m_glWindowPos2s)(GLshort x, GLshort y);
  void (QGL_DLLEXPORT *m_glWindowPos2sv)(const GLshort *p);
  void (QGL_DLLEXPORT *m_glWindowPos3d)(GLdouble x, GLdouble y, GLdouble z);
  void (QGL_DLLEXPORT *m_glWindowPos3dv)(const GLdouble *p);
  void (QGL_DLLEXPORT *m_glWindowPos3f)(GLfloat x, GLfloat y, GLfloat z);
  void (QGL_DLLEXPORT *m_glWindowPos3fv)(const GLfloat *p);
  void (QGL_DLLEXPORT *m_glWindowPos3i)(GLint x, GLint y, GLint z);
  void (QGL_DLLEXPORT *m_glWindowPos3iv)(const GLint *p);
  void (QGL_DLLEXPORT *m_glWindowPos3s)(GLshort x, GLshort y, GLshort z);
  void (QGL_DLLEXPORT *m_glWindowPos3sv)(const GLshort *p);

  // GL 1.5
  bool support_GL_1_5;
  bool GL_1_5()
  {
    return support_GL_1_5;
  }
  void (QGL_DLLEXPORT *m_glBeginQuery)(GLenum target, GLuint id);
  void (QGL_DLLEXPORT *m_glBindBuffer)(GLenum target, GLuint buffer);
  void (QGL_DLLEXPORT *m_glBufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
  void (QGL_DLLEXPORT *m_glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
  void (QGL_DLLEXPORT *m_glDeleteBuffers)(GLsizei n, const GLuint* buffers);
  void (QGL_DLLEXPORT *m_glDeleteQueries)(GLsizei n, const GLuint* ids);
  void (QGL_DLLEXPORT *m_glEndQuery)(GLenum target);
  void (QGL_DLLEXPORT *m_glGenBuffers)(GLsizei n, GLuint* buffers);
  void (QGL_DLLEXPORT *m_glGenQueries)(GLsizei n, GLuint* ids);
  void (QGL_DLLEXPORT *m_glGetBufferParameteriv)(GLenum target, GLenum pname, GLint* params);
  void (QGL_DLLEXPORT *m_glGetBufferPointerv)(GLenum target, GLenum pname, GLvoid** params);
  void (QGL_DLLEXPORT *m_glGetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data);
  void (QGL_DLLEXPORT *m_glGetQueryObjectiv)(GLuint id, GLenum pname, GLint* params);
  void (QGL_DLLEXPORT *m_glGetQueryObjectuiv)(GLuint id, GLenum pname, GLuint* params);
  void (QGL_DLLEXPORT *m_glGetQueryiv)(GLenum target, GLenum pname, GLint params);
  GLboolean (QGL_DLLEXPORT *m_glIsBuffer)(GLuint buffer);
  GLboolean (QGL_DLLEXPORT *m_glIsQuery)(GLuint id);
  GLvoid* (QGL_DLLEXPORT *m_glMapBuffer)(GLenum target, GLenum access);
  GLboolean (QGL_DLLEXPORT *m_glUnmapBuffer)(GLenum target);

  // GL_ARB_vertex_program
  bool support_ARB_vertex_program;
  bool ARB_vertex_program()
  {
    return support_ARB_vertex_program;
  }
  void (QGL_DLLEXPORT *m_glVertexAttrib1sARB)(GLuint index, GLshort x);
  void (QGL_DLLEXPORT *m_glVertexAttrib1fARB)(GLuint index, GLfloat x);
  void (QGL_DLLEXPORT *m_glVertexAttrib1dARB)(GLuint index, GLdouble x);
  void (QGL_DLLEXPORT *m_glVertexAttrib2sARB)(GLuint index, GLshort x, GLshort y);
  void (QGL_DLLEXPORT *m_glVertexAttrib2fARB)(GLuint index, GLfloat x, GLfloat y);
  void (QGL_DLLEXPORT *m_glVertexAttrib2dARB)(GLuint index, GLdouble x, GLdouble y);
  void (QGL_DLLEXPORT *m_glVertexAttrib3sARB)(GLuint index, GLshort x, GLshort y, GLshort z);
  void (QGL_DLLEXPORT *m_glVertexAttrib3fARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
  void (QGL_DLLEXPORT *m_glVertexAttrib3dARB)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
  void (QGL_DLLEXPORT *m_glVertexAttrib4sARB)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
  void (QGL_DLLEXPORT *m_glVertexAttrib4fARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
  void (QGL_DLLEXPORT *m_glVertexAttrib4dARB)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NubARB)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
  void (QGL_DLLEXPORT *m_glVertexAttrib1svARB)(GLuint index, const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib1fvARB)(GLuint index, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib1dvARB)(GLuint index, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib2svARB)(GLuint index, const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib2fvARB)(GLuint index, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib2dvARB)(GLuint index, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib3svARB)(GLuint index, const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib3fvARB)(GLuint index, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib3dvARB)(GLuint index, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4bvARB)(GLuint index, const GLbyte *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4svARB)(GLuint index, const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4ivARB)(GLuint index, const GLint *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4ubvARB)(GLuint index, const GLubyte *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4usvARB)(GLuint index, const GLushort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4uivARB)(GLuint index, const GLuint *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4fvARB)(GLuint index, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4dvARB)(GLuint index, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NbvARB)(GLuint index, const GLbyte *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NsvARB)(GLuint index, const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NivARB)(GLuint index, const GLint *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NubvARB)(GLuint index, const GLubyte *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NusvARB)(GLuint index, const GLushort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NuivARB)(GLuint index, const GLuint *v);
  void (QGL_DLLEXPORT *m_glVertexAttribPointerARB)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
  void (QGL_DLLEXPORT *m_glEnableVertexAttribArrayARB)(GLuint index);
  void (QGL_DLLEXPORT *m_glDisableVertexAttribArrayARB)(GLuint index);
  void (QGL_DLLEXPORT *m_glProgramStringARB)(GLenum target, GLenum format, GLsizei len, const GLvoid *string); 
  void (QGL_DLLEXPORT *m_glBindProgramARB)(GLenum target, GLuint program);
  void (QGL_DLLEXPORT *m_glDeleteProgramsARB)(GLsizei n, const GLuint *programs);
  void (QGL_DLLEXPORT *m_glGenProgramsARB)(GLsizei n, GLuint *programs);
  void (QGL_DLLEXPORT *m_glProgramEnvParameter4dARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
  void (QGL_DLLEXPORT *m_glProgramEnvParameter4dvARB)(GLenum target, GLuint index, const GLdouble *params);
  void (QGL_DLLEXPORT *m_glProgramEnvParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
  void (QGL_DLLEXPORT *m_glProgramEnvParameter4fvARB)(GLenum target, GLuint index, const GLfloat *params);
  void (QGL_DLLEXPORT *m_glProgramLocalParameter4dARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
  void (QGL_DLLEXPORT *m_glProgramLocalParameter4dvARB)(GLenum target, GLuint index, const GLdouble *params);
  void (QGL_DLLEXPORT *m_glProgramLocalParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
  void (QGL_DLLEXPORT *m_glProgramLocalParameter4fvARB)(GLenum target, GLuint index, const GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetProgramEnvParameterdvARB)(GLenum target, GLuint index, GLdouble *params);
  void (QGL_DLLEXPORT *m_glGetProgramEnvParameterfvARB)(GLenum target, GLuint index, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetProgramLocalParameterdvARB)(GLenum target, GLuint index, GLdouble *params);
  void (QGL_DLLEXPORT *m_glGetProgramLocalParameterfvARB)(GLenum target, GLuint index, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetProgramivARB)(GLenum target, GLenum pname, GLint *params);
  void (QGL_DLLEXPORT *m_glGetProgramStringARB)(GLenum target, GLenum pname, GLvoid *string);
  void (QGL_DLLEXPORT *m_glGetVertexAttribdvARB)(GLuint index, GLenum pname, GLdouble *params);
  void (QGL_DLLEXPORT *m_glGetVertexAttribfvARB)(GLuint index, GLenum pname, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetVertexAttribivARB)(GLuint index, GLenum pname, GLint *params);
  void (QGL_DLLEXPORT *m_glGetVertexAttribPointervARB)(GLuint index, GLenum pname, GLvoid **pointer);
  GLboolean (QGL_DLLEXPORT *m_glIsProgramARB)(GLuint program);

  // GL_ARB_fragment_program
  bool support_ARB_fragment_program;
  bool ARB_fragment_program()
  {
    return support_ARB_fragment_program;
  }

  // GL_ARB_shader_objects
  bool support_ARB_shader_objects;
  bool ARB_shader_objects()
  {
    return support_ARB_shader_objects;
  }
  void (QGL_DLLEXPORT *m_glDeleteObjectARB)(GLhandleARB obj);
  GLhandleARB (QGL_DLLEXPORT *m_glGetHandleARB)(GLenum pname);
  void (QGL_DLLEXPORT *m_glDetachObjectARB)(GLhandleARB containerObj, GLhandleARB attachedObj);
  GLhandleARB (QGL_DLLEXPORT *m_glCreateShaderObjectARB)(GLenum shaderType);
  void (QGL_DLLEXPORT *m_glShaderSourceARB)(GLhandleARB shaderObj, GLsizei count, const GLcharARB **string, const GLint *length);
  void (QGL_DLLEXPORT *m_glCompileShaderARB)(GLhandleARB shaderObj);
  GLhandleARB (QGL_DLLEXPORT *m_glCreateProgramObjectARB)(GLvoid);
  void (QGL_DLLEXPORT *m_glAttachObjectARB)(GLhandleARB containerObj, GLhandleARB obj);
  void (QGL_DLLEXPORT *m_glLinkProgramARB)(GLhandleARB programObj);
  void (QGL_DLLEXPORT *m_glUseProgramObjectARB)(GLhandleARB programObj);
  void (QGL_DLLEXPORT *m_glValidateProgramARB)(GLhandleARB programObj);
  void (QGL_DLLEXPORT *m_glUniform1fARB)(GLint location, GLfloat v0);
  void (QGL_DLLEXPORT *m_glUniform2fARB)(GLint location, GLfloat v0, GLfloat v1);
  void (QGL_DLLEXPORT *m_glUniform3fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
  void (QGL_DLLEXPORT *m_glUniform4fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
  void (QGL_DLLEXPORT *m_glUniform1iARB)(GLint location, GLint v0);
  void (QGL_DLLEXPORT *m_glUniform2iARB)(GLint location, GLint v0, GLint v1);
  void (QGL_DLLEXPORT *m_glUniform3iARB)(GLint location, GLint v0, GLint v1, GLint v2);
  void (QGL_DLLEXPORT *m_glUniform4iARB)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
  void (QGL_DLLEXPORT *m_glUniform1fvARB)(GLint location, GLsizei count, const GLfloat *value);
  void (QGL_DLLEXPORT *m_glUniform2fvARB)(GLint location, GLsizei count, const GLfloat *value);
  void (QGL_DLLEXPORT *m_glUniform3fvARB)(GLint location, GLsizei count, const GLfloat *value);
  void (QGL_DLLEXPORT *m_glUniform4fvARB)(GLint location, GLsizei count, const GLfloat *value);
  void (QGL_DLLEXPORT *m_glUniform1ivARB)(GLint location, GLsizei count, const GLint *value);
  void (QGL_DLLEXPORT *m_glUniform2ivARB)(GLint location, GLsizei count, const GLint *value);
  void (QGL_DLLEXPORT *m_glUniform3ivARB)(GLint location, GLsizei count, const GLint *value);
  void (QGL_DLLEXPORT *m_glUniform4ivARB)(GLint location, GLsizei count, const GLint *value);
  void (QGL_DLLEXPORT *m_glUniformMatrix2fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
  void (QGL_DLLEXPORT *m_glUniformMatrix3fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
  void (QGL_DLLEXPORT *m_glUniformMatrix4fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
  void (QGL_DLLEXPORT *m_glGetObjectParameterfvARB)(GLhandleARB obj, GLenum pname, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetObjectParameterivARB)(GLhandleARB obj, GLenum pname, GLint *params);
  void (QGL_DLLEXPORT *m_glGetInfoLogARB)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
  void (QGL_DLLEXPORT *m_glGetAttachedObjectsARB)(GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
  GLint (QGL_DLLEXPORT *m_glGetUniformLocationARB)(GLhandleARB programObj, const GLcharARB *name);
  void (QGL_DLLEXPORT *m_glGetActiveUniformARB)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
  void (QGL_DLLEXPORT *m_glGetUniformfvARB)(GLhandleARB programObj, GLint location, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetUniformivARB)(GLhandleARB programObj, GLint location, GLint *params);
  void (QGL_DLLEXPORT *m_glGetShaderSourceARB)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);

  // GL_ARB_vertex_shader
  bool support_ARB_vertex_shader;
  bool ARB_vertex_shader()
  {
    return support_ARB_vertex_shader;
  }
#if 0
  void (QGL_DLLEXPORT *m_glVertexAttrib1fARB)(GLuint index, GLfloat v0);
  void (QGL_DLLEXPORT *m_glVertexAttrib1sARB)(GLuint index, GLshort v0);
  void (QGL_DLLEXPORT *m_glVertexAttrib1dARB)(GLuint index, GLdouble v0);
  void (QGL_DLLEXPORT *m_glVertexAttrib2fARB)(GLuint index, GLfloat v0, GLfloat v1);
  void (QGL_DLLEXPORT *m_glVertexAttrib2sARB)(GLuint index, GLshort v0, GLshort v1);
  void (QGL_DLLEXPORT *m_glVertexAttrib2dARB)(GLuint index, GLdouble v0, GLdouble v1);
  void (QGL_DLLEXPORT *m_glVertexAttrib3fARB)(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);
  void (QGL_DLLEXPORT *m_glVertexAttrib3sARB)(GLuint index, GLshort v0, GLshort v1, GLshort v2);
  void (QGL_DLLEXPORT *m_glVertexAttrib3dARB)(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2);
  void (QGL_DLLEXPORT *m_glVertexAttrib4fARB)(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
  void (QGL_DLLEXPORT *m_glVertexAttrib4sARB)(GLuint index, GLshort v0, GLshort v1, GLshort v2, GLshort v3);
  void (QGL_DLLEXPORT *m_glVertexAttrib4dARB)(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NubARB)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
  void (QGL_DLLEXPORT *m_glVertexAttrib1fvARB)(GLuint index, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib1svARB)(GLuint index, const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib1dvARB)(GLuint index, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib2fvARB)(GLuint index, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib2svARB)(GLuint index, const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib2dvARB)(GLuint index, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib3fvARB)(GLuint index, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib3svARB)(GLuint index, const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib3dvARB)(GLuint index, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4fvARB)(GLuint index, const GLfloat *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4svARB)(GLuint index, const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4dvARB)(GLuint index, const GLdouble *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4ivARB)(GLuint index, const GLint *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4bvARB)(GLuint index, const GLbyte *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4ubvARB)(GLuint index, const GLubyte *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4usvARB)(GLuint index, const GLushort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4uivARB)(GLuint index, const GLuint *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NbvARB)(GLuint index, const GLbyte *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NsvARB)(GLuint index, const GLshort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NivARB)(GLuint index, const GLint *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NubvARB)(GLuint index, const GLubyte *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NusvARB)(GLuint index, const GLushort *v);
  void (QGL_DLLEXPORT *m_glVertexAttrib4NuivARB)(GLuint index, const GLuint *v);
  void (QGL_DLLEXPORT *m_glVertexAttribPointerARB)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
  void (QGL_DLLEXPORT *m_glEnableVertexAttribArrayARB)(GLuint index);
  void (QGL_DLLEXPORT *m_glDisableVertexAttribArrayARB)(GLuint index);
#endif
  void (QGL_DLLEXPORT *m_glBindAttribLocationARB)(GLhandleARB programObj, GLuint index, const GLcharARB *name);
  void (QGL_DLLEXPORT *m_glGetActiveAttribARB)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
  GLint (QGL_DLLEXPORT *m_glGetAttribLocationARB)(GLhandleARB programObj, const GLcharARB *name);
#if 0
  void (QGL_DLLEXPORT *m_glGetVertexAttribdvARB)(GLuint index, GLenum pname, GLdouble *params);
  void (QGL_DLLEXPORT *m_glGetVertexAttribfvARB)(GLuint index, GLenum pname, GLfloat *params);
  void (QGL_DLLEXPORT *m_glGetVertexAttribivARB)(GLuint index, GLenum pname, GLint *params);
  void (QGL_DLLEXPORT *m_glGetVertexAttribPointervARB)(GLuint index, GLenum pname, GLvoid **pointer);
#endif

  // ARB_fragment_shader
  bool support_ARB_fragment_shader;
  bool ARB_fragment_shader()
  {
    return support_ARB_fragment_shader;
  }

  // ARB_shading_language_100
  bool support_ARB_shading_language_100;
  bool ARB_shading_language_100()
  {
    return support_ARB_shading_language_100;
  }

  // GL_NV_vertex_program2
  bool support_NV_vertex_program2;
  bool NV_vertex_program2()
  {
    return support_NV_vertex_program2;
  }
  GLboolean (QGL_DLLEXPORT* m_glAreProgramsResidentNV)(GLsizei, const GLuint *, GLboolean *);
  void (QGL_DLLEXPORT* m_glBindProgramNV)(GLenum, GLuint);
  void (QGL_DLLEXPORT* m_glDeleteProgramsNV)(GLsizei, const GLuint *);
  void (QGL_DLLEXPORT* m_glExecuteProgramNV)(GLenum, GLuint, const GLfloat *);
  void (QGL_DLLEXPORT* m_glGenProgramsNV)(GLsizei, GLuint *);
  void (QGL_DLLEXPORT* m_glGetProgramParameterdvNV)(GLenum, GLuint, GLenum, GLdouble *);
  void (QGL_DLLEXPORT* m_glGetProgramParameterfvNV)(GLenum, GLuint, GLenum, GLfloat *);
  void (QGL_DLLEXPORT* m_glGetProgramivNV)(GLuint, GLenum, GLint *);
  void (QGL_DLLEXPORT* m_glGetProgramStringNV)(GLuint, GLenum, GLubyte *);
  void (QGL_DLLEXPORT* m_glGetTrackMatrixivNV)(GLenum, GLuint, GLenum, GLint *);
  void (QGL_DLLEXPORT* m_glGetVertexAttribdvNV)(GLuint, GLenum, GLdouble *);
  void (QGL_DLLEXPORT* m_glGetVertexAttribfvNV)(GLuint, GLenum, GLfloat *);
  void (QGL_DLLEXPORT* m_glGetVertexAttribivNV)(GLuint, GLenum, GLint *);
  void (QGL_DLLEXPORT* m_glGetVertexAttribPointervNV)(GLuint, GLenum, GLvoid* *);
  GLboolean (QGL_DLLEXPORT* m_glIsProgramNV)(GLuint);
  void (QGL_DLLEXPORT* m_glLoadProgramNV)(GLenum, GLuint, GLsizei, const GLubyte *);
  void (QGL_DLLEXPORT* m_glProgramParameter4fNV)(GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
  void (QGL_DLLEXPORT* m_glProgramParameter4fvNV)(GLenum, GLuint, const GLfloat *);
  void (QGL_DLLEXPORT* m_glProgramParameters4fvNV)(GLenum, GLuint, GLuint, const GLfloat *);
  void (QGL_DLLEXPORT* m_glRequestResidentProgramsNV)(GLsizei, const GLuint *);
  void (QGL_DLLEXPORT* m_glTrackMatrixNV)(GLenum, GLuint, GLenum, GLenum);
  void (QGL_DLLEXPORT* m_glVertexAttribPointerNV)(GLuint, GLint, GLenum, GLsizei, const GLvoid *);
  void (QGL_DLLEXPORT* m_glVertexAttrib1fNV)(GLuint, GLfloat);
  void (QGL_DLLEXPORT* m_glVertexAttrib1fvNV)(GLuint, const GLfloat *);
  void (QGL_DLLEXPORT* m_glVertexAttrib2fNV)(GLuint, GLfloat, GLfloat);
  void (QGL_DLLEXPORT* m_glVertexAttrib2fvNV)(GLuint, const GLfloat *);
  void (QGL_DLLEXPORT* m_glVertexAttrib3fNV)(GLuint, GLfloat, GLfloat, GLfloat);
  void (QGL_DLLEXPORT* m_glVertexAttrib3fvNV)(GLuint, const GLfloat *);
  void (QGL_DLLEXPORT* m_glVertexAttrib4fNV)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
  void (QGL_DLLEXPORT* m_glVertexAttrib4fvNV)(GLuint, const GLfloat *);
  void (QGL_DLLEXPORT* m_glVertexAttribs1fvNV)(GLuint, GLsizei, const GLfloat *);
  void (QGL_DLLEXPORT* m_glVertexAttribs2fvNV)(GLuint, GLsizei, const GLfloat *);
  void (QGL_DLLEXPORT* m_glVertexAttribs3fvNV)(GLuint, GLsizei, const GLfloat *);
  void (QGL_DLLEXPORT* m_glVertexAttribs4fvNV)(GLuint, GLsizei, const GLfloat *);

  // GL_NV_fragment_program
  bool support_NV_fragment_program;
  bool NV_fragment_program()
  {
    return support_NV_fragment_program;
  }
  void (QGL_DLLEXPORT* m_glProgramNamedParameter4fNV)(GLuint, GLsizei, const GLubyte *, GLfloat, GLfloat, GLfloat, GLfloat);
  void (QGL_DLLEXPORT* m_glProgramNamedParameter4fvNV)(GLuint, GLsizei, const GLubyte *, const GLfloat *);
  void (QGL_DLLEXPORT* m_glGetProgramNamedParameterfvNV)(GLuint, GLsizei, const GLubyte *, GLfloat *);
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<OpenGLBinding> GlobalOpenGLModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<OpenGLBinding> GlobalOpenGLModuleRef;

inline OpenGLBinding& GlobalOpenGL()
{
  return GlobalOpenGLModule::getTable();
}

#if defined(_DEBUG)
inline void GlobalOpenGL_debugAssertNoErrors()
{
  GlobalOpenGL().assertNoErrors();
}
#else
inline void GlobalOpenGL_debugAssertNoErrors()
{
}
#endif


#endif
