//	*************************************************
//	*      	
//	*   Vertex Lighting Replacement Shaders
//	*	
//	*************************************************

textures/terrain/vxmpterra2

{
	qer_editorimage textures/stone/pjrock10bc.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight


	{

		map textures/stone/pjrock10b_2.tga
		rgbGen vertex
		tcmod scale 0.125 0.125
	}
	
}


textures/terrain/vxmpterra1

{
	qer_editorimage textures/stone/pjrock12c.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight


	{

		map textures/stone/pjrock12c.tga
		rgbGen vertex
		tcmod scale 0.125 0.125
	}
	
}


// *********************************************************

//	*************************************************
//	*      	
//	*   MPTerra2 terrain shaders (PaulJ)
//	*	
//	*************************************************


textures/terrain/mpterra2_0

{
	qer_editorimage textures/stone/pjrock9b_2.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight

	{

		map textures/stone/pjrock9b_2.tga
		rgbGen vertex
		tcmod scale 0.125 0.125
	}
	
	{

                map textures/skies2/clouds.tga
		blendfunc filter
                detail
		tcmod scale 0.01 0.01
                tcMod scroll -0.05 0.05
		tcmod transform 1 0 1 1 1 1
		
	}

	
}

textures/terrain/mpterra2_1

{
	qer_editorimage textures/stone/pjrock12b_2.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight

	{
		
		map textures/stone/pjrock12b_2.tga
		rgbGen vertex
		tcmod scale 0.1 0.1
    
	}

	{
	
                map textures/skies2/clouds.tga
		blendfunc filter
		detail
		tcmod scale 0.01 0.01
                tcMod scroll -0.05 0.05
		tcmod transform 1 0 1 1 1 1
		
	}


}

textures/terrain/mpterra2_2
{
	qer_editorimage textures/stone/pjrock10b_2.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight

	{
		
		map textures/stone/pjrock10b_2.tga
		tcmod scale 0.05 0.05
		rgbGen vertex
	}

	{
                map textures/skies2/clouds.tga
		blendfunc filter
		detail
		tcmod scale 0.01 0.01
                tcMod scroll -0.05 0.05
		tcmod transform 1 0 1 1 1 1
		
	}

}

textures/terrain/mpterra2_0to1

{
	qer_editorimage textures/stone/pjrock9b_2.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight

	{
		map textures/stone/pjrock9b_2.tga
		rgbGen vertex
		alphaGen vertex
		tcmod scale 0.125 0.125 
	}

	{
		map textures/stone/pjrock12b_2.tga
		tcmod scale 0.1 0.1
		rgbGen vertex
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}

	{
                map textures/skies2/clouds.tga
		blendfunc filter
		detail
		tcmod scale 0.01 0.01
                tcMod scroll -0.05 0.05
		tcmod transform 1 0 1 1 1 1		
	}


}

textures/terrain/mpterra2_0to2
{
	qer_editorimage textures/stone/pjrock9b_2.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight
	{
		map textures/stone/pjrock9b_2.tga
		rgbGen vertex
		alphaGen vertex
		tcmod scale 0.125 0.125
	}
	{
		map textures/stone/pjrock10b_2.tga
		rgbGen vertex
		alphaGen vertex
                tcmod scale 0.05 0.05
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}

	{
                map textures/skies2/clouds.tga
		blendfunc filter
		detail
		tcmod scale 0.01 0.01
                tcMod scroll -0.05 0.05
		tcmod transform 1 0 1 1 1 1
		
	}


}

textures/terrain/mpterra2_1to2
{
	qer_editorimage textures/stone/pjrock12b_2.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight

	{
		map textures/stone/pjrock12b_2.tga
		rgbGen vertex
		alphaGen vertex
		tcmod scale 0.1 0.1
	}

	{
		map textures/stone/pjrock10b_2.tga
		tcmod scale 0.05 0.05
		rgbGen vertex
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}

	{
                map textures/skies2/clouds.tga
		blendfunc filter
		detail
		tcmod scale 0.01 0.01
                tcMod scroll -0.05 0.05
		tcmod transform 1 0 1 1 1 1
	}


}

//////////////////////////////////////////////
//                                          //
//                x_mpterra                 //
//                                          //
//////////////////////////////////////////////

textures/terrain/vxmpterra3

{
	qer_editorimage textures/proto2/px02.tga
	surfaceparm nolightmap
	//q3map_novertexshadows
	//q3map_forcesunlight	
	surfaceparm dust
	{
		map textures/proto2/px02.tga
		tcmod scale .1 .1
		rgbGen vertex
	}

}



textures/proto2/xproto_0to1

{
	qer_editorimage textures/proto2/px01.tga
	surfaceparm nolightmap
	//q3map_novertexshadows
	//q3map_forcesunlight	
        surfaceparm dust

	{
		map textures/proto2/px01.tga
		tcmod scale .1 .1
		rgbGen vertex
		
	}

	{
		map textures/proto2/px02.tga
		rgbGen vertex
		alphaGen vertex
		tcmod scale .1 .1
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	
	
}

textures/proto2/xproto_1to2

{
	qer_editorimage textures/proto2/px02.tga
	surfaceparm nolightmap
	//q3map_novertexshadows
	//q3map_forcesunlight	
	    surfaceparm dust
	{
		map textures/proto2/px02.tga
		tcmod scale .1 .1
		rgbGen vertex
		alphaGen vertex
	}

	{
		map textures/proto2/px03.tga
		rgbGen vertex
		alphaGen vertex
		tcmod scale .1 .1
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}


}

textures/proto2/xproto_0to2

{
	qer_editorimage textures/proto2/px02.tga
	surfaceparm nolightmap
	//q3map_novertexshadows
	//q3map_forcesunlight	
	    surfaceparm dust

	{
		map textures/proto2/px01.tga
		tcmod scale .1 .1
		rgbGen vertex
	}
	{
		map textures/proto2/px03.tga
		rgbGen vertex
		alphaGen vertex
		tcmod scale .1 .1
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}


}



textures/proto2/xproto_0

{
	qer_editorimage textures/proto2/px01.tga
	surfaceparm nolightmap
	//q3map_novertexshadows
	//q3map_forcesunlight	
	surfaceparm dust


	{
		map textures/proto2/px01.tga
		tcmod scale .1 .1
		rgbGen vertex
	}

	

}

textures/proto2/xproto_1

{
	qer_editorimage textures/proto2/px02.tga
	surfaceparm nolightmap
	//q3map_novertexshadows
	//q3map_forcesunlight	
	surfaceparm dust
	{
		map textures/proto2/px02.tga
		tcmod scale .1 .1
		rgbGen vertex
	}
	
	
	
}

textures/proto2/xproto_2

{
	qer_editorimage textures/proto2/px03.tga
	surfaceparm nolightmap
	//q3map_novertexshadows
	//q3map_forcesunlight	
    	surfaceparm dust
	{
		map textures/proto2/px03.tga
		tcmod scale .1 .1
		rgbGen vertex
	}

}



//	*************************************************
//	*      	
//	*   MPTerra1 terrain shaders (Tim)
//	*	
//	*************************************************

textures/terrain/mpterra1_0
{
	qer_editorimage textures/stone/pjrock9c.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight	
	{
		map textures/stone/pjrock9c.tga
		rgbGen vertex
		tcmod scale 0.125 0.125
	}
	{
                map textures/skies2/clouds2.tga
		blendfunc filter
		tcmod scale 0.01 0.01
                 tcMod scroll -0.1 0.1
		
	}
	
}

textures/terrain/mpterra1_1
{
	qer_editorimage textures/stone/pjrock12c.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight	
	{
		map textures/stone/pjrock12c.tga
		//tcmod transform 1 0 1 1 1 1
		rgbGen vertex
		tcmod scale 0.125 0.125 
	}
	{
                map textures/skies2/clouds2.tga
		blendfunc filter
		tcmod scale 0.01 0.01
                tcMod scroll -0.1 0.1
		
	}

}

textures/terrain/mpterra1_2
{
	qer_editorimage textures/stone/pjrock10c.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight	
	{
		map textures/stone/pjrock10c.tga
		tcmod scale 0.05 0.05 
		rgbGen vertex
	}
	{
                map textures/skies2/clouds2.tga
		blendfunc filter
		tcmod scale 0.01 0.01
                tcMod scroll -0.1 0.1
		
	}

}

textures/terrain/mpterra1_0to1
{
	qer_editorimage textures/stone/pjrock9c.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight	
	{
		map textures/stone/pjrock9c.tga	
		rgbGen vertex
		alphaGen vertex
		tcmod scale 0.125 0.125 
	}
	{
		map textures/stone/pjrock12c.tga
		tcmod scale 0.125 0.125 
		rgbGen vertex
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	{
                map textures/skies2/clouds2.tga
		blendfunc filter
		tcmod scale 0.01 0.01
                tcMod scroll -0.1 0.1
		
	}

}

textures/terrain/mpterra1_0to2
{
	qer_editorimage textures/stone/pjrock9c.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight	
	{
		map textures/stone/pjrock9c.tga
		rgbGen vertex
		alphaGen vertex
		tcmod scale 0.125 0.125
	}
	{
		map textures/stone/pjrock10c.tga
		//tcmod transform 1 0 1 1 1 1
		tcmod scale 0.05 0.05 
		rgbGen vertex
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	{
                map textures/skies2/clouds2.tga
		blendfunc filter
		tcmod scale 0.01 0.01
                tcMod scroll -0.1 0.1
		
	}

}

textures/terrain/mpterra1_1to2
{
	qer_editorimage textures/stone/pjrock12c.tga
	surfaceparm nolightmap
	q3map_novertexshadows
	q3map_forcesunlight	
	{
		map textures/stone/pjrock12c.tga
		rgbGen vertex
		alphaGen vertex
		tcmod scale 0.125 0.125 
	}
	{
		map textures/stone/pjrock10c.tga
		//tcmod transform 1 0 1 1 1 1
		tcmod scale 0.05 0.05 
		rgbGen vertex
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	{
                map textures/skies2/clouds2.tga
		blendfunc filter
		tcmod scale 0.01 0.01
                tcMod scroll -0.1 0.1
		
	}
}