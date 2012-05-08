textures/liquids2/calm_poollight2nt
	{


	
		qer_editorimage textures/liquids/pool3d_4b.tga
		q3map_globaltexture
		qer_trans .75

		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water
		surfaceparm nolightmap
		q3map_surfacelight 50

		cull disable
		
	
	
		{ 
			map textures/liquids/pool3d_5c2.tga
			blendFunc GL_dst_color GL_zero
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 1.5 0 1.5 1 1 2
			tcmod scroll -.05 .001
		}
	
		{ 
			map textures/liquids/pool3d_6c2.tga
			blendFunc GL_dst_color GL_zero
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 0 1.5 1 1.5 2 1
			tcmod scroll .025 -.001
		}

		{ 
			map textures/liquids/pool3d_3c2.tga
			blendFunc GL_dst_color GL_zero
			rgbgen identity
			tcmod scale .25 .5
			tcmod scroll .001 .025
		}
		{
			map textures/liquids/pool3d_4b2.tga
			blendfunc add
			tcmod scale .125 .125	
			rgbgen identity
		}


		//	END
	}

textures/liquids2/water_volume

	{
		qer_editorimage textures/liquids/pool3d_3.tga
		qer_trans .5
		surfaceparm nodraw
		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water

		cull disable

//	END
	}


textures/liquids2/clear_ripple1v

	{
		qer_editorimage textures/liquids/pool3d_3.tga
		qer_trans .5
		q3map_globaltexture
		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water

		cull disable
		deformVertexes wave 64 sin .5 .5 0 .5	
	
		
		{ 
			map textures/liquids/pool3d_5.tga
			blendFunc GL_dst_color GL_one
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 1.5 0 1.5 1 1 2
			tcmod scroll -.05 .001
		}
	
		{ 
			map textures/liquids/pool3d_6.tga
			blendFunc GL_dst_color GL_one
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 0 1.5 1 1.5 2 1
			tcmod scroll .025 -.001
		}

		{ 
			map textures/liquids/pool3d_3.tga
			blendFunc GL_dst_color GL_one
			rgbgen vertex
			tcmod scale .25 .5
			tcmod scroll .001 .025
		}	

//	END
	}

textures/liquids2/clear_ripple1_q3dm1v
	
	{
		qer_editorimage textures/liquids/pool3d_3.tga
		qer_trans .5
		q3map_globaltexture
		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water

		cull disable
		deformVertexes wave 64 sin .5 .5 0 .5	
	
		
		{ 
			map textures/liquids/pool3d_5.tga
			blendFunc GL_dst_color GL_one
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 1.5 0 1.5 1 1 2
			tcmod scroll -.05 .001
		}
	
		{ 
			map textures/liquids/pool3d_6.tga
			blendFunc GL_dst_color GL_one
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 0 1.5 1 1.5 2 1
			tcmod scroll .025 -.001
		}

		{ 
			map textures/liquids/pool3d_3.tga
			blendFunc GL_dst_color GL_one
			rgbgen vertex
			tcmod scale .25 .5
			tcmod scroll .001 .025
		}	

//	END
	}
textures/liquids2/clear_calm1v
	
	{
		qer_editorimage textures/liquids/pool3d_3e.tga
		qer_trans .5
		q3map_globaltexture
		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water
	
		cull disable
		deformVertexes wave 64 sin .25 .25 0 .5	
		{ 
			map textures/liquids/pool3d_5e.tga
			blendFunc GL_dst_color GL_one
			rgbgen identity
			tcmod scale .5 .5
			tcmod scroll .025 .01
                        
		}
	
		{ 
			map textures/liquids/pool3d_3e.tga
			blendFunc GL_dst_color GL_one
			tcmod scale -.5 -.5
			tcmod scroll .025 .025
                        rgbGen vertex
		}


	
	

//	END
	}
textures/liquids2/pjslime
	{
	//	*************************************************		
	//	* ORIGINAL SLIME Feb 11 1999 		        *
	//	* IF YOU CHANGE THIS PLEASE COMMENT THE CHANGE	*
        //      *
	//	*************************************************	
		//Changed light value to 400 on May 17, 2000
		// Added to g3map_global texture on May 11, 1999
		q3map_lightimage textures/proto2/concrete_blue.tga
		q3map_globaltexture
		qer_trans .5

		surfaceparm noimpact
		surfaceparm lava
		surfaceparm trans		

		q3map_surfacelight 500
		tessSize 64
		//cull disable
		deformVertexes wave 100 sin 1 2 0 .5
		
		
		{	
			map $lightmap
			rgbgen identity
		}
		

		{
			map textures/liquids2/pjslime.tga
			blendFunc GL_DST_COLOR GL_SRC_ALPHA
			rgbGen identity
			alphaGen lightingSpecular
			tcMod turb 0 .2 .5 .05
			tcmod scroll .01 .01
		}


		
		{
			map textures/liquids2/pjslime2.tga
			blendfunc add
			rgbgen wave inversesawtooth 0 1 0 4
			tcMod turb 0 .2 .5 .05
			tcmod scroll .01 .01
		}

}


	