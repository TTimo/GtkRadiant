
textures/liquids/protolava
{
	
	q3map_globaltexture
	surfaceparm trans
	//surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm lava
	surfaceparm nolightmap
	q3map_surfacelight 600
	cull disable
	
	tesssize 128
	cull disable
	deformVertexes wave 100 sin 3 2 .1 0.1
	
        {
		map textures/liquids/protolava2.tga
                tcmod scale .2 .2
                tcmod scroll .04 .03
                tcMod turb 0 .1 0 .01
                blendFunc GL_ONE GL_ZERO
                rgbGen identity
	}
	{
		map textures/liquids/protolava.tga
                blendfunc blend
		tcMod turb 0 .2 0 .1
	}

}

textures/liquids/watertest
	{
		qer_editorimage textures/liquids/pool3d_3b.tga
		qer_trans .5
		q3map_globaltexture
		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water

		cull disable
		tessSize 32
		deformVertexes normal .15 2 
		deformVertexes bulge 128 5 2
		{	
			map $lightmap
			blendfunc filter
		}
		
		{
			map textures/liquids/pool3d_4byell2.tga
			blendfunc add
			tcgen environment
		}

	
	}




textures/liquids/flatripplewater_1500
	{
		qer_editorimage textures/liquids/pool2.tga
		q3map_globaltexture
		q3map_lightsubdivide 32
		surfaceparm noimpact
		surfaceparm lava
		surfaceparm nolightmap
		q3map_surfacelight 500
		cull disable
	
		deformVertexes wave 100 sin 3 2 .1 0.1
	
	{
		map textures/liquids/pool2.tga
		tcMod turb 0 .2 0 .1
	}
}

textures/liquids/xctf_painwater
	{
		qer_editorimage textures/liquids/pool2.tga
		q3map_globaltexture
		qer_trans .5

		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water
		surfaceparm nolightmap
		q3map_surfacelight 450

		cull disable
		tesssize 64
		deformVertexes wave 100 sin 1 1 1 .1
		{
			map textures/liquids/pool2.tga
			blendfunc GL_ONE GL_SRC_COLOR	
			tcMod scale .03 .03
			tcMod scroll .001 .001
		}
		{
			map textures/liquids/pool3.tga
			blendfunc GL_DST_COLOR GL_ONE
			tcMod turb .1 .1 0 .01
			tcMod scale .5 .5
			tcMod scroll -.025 .02
		}
                {
			map textures/sfx/bolts.tga
			blendfunc GL_ONE GL_ONE
			tcMod scale .2 .2
			tcMod scroll .1 .1
                        tcmod rotate 290

		}
	}
textures/liquids/clear_calm1
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
		}

	
		{
			map $lightmap
			blendFunc GL_dst_color GL_zero
			rgbgen identity		
		}
	
	

}

textures/liquids/clear_ripple3
	{
		qer_editorimage textures/liquids/pool3d_3c.tga
		qer_trans .5
		q3map_globaltexture
		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water
	
		cull disable
		deformVertexes wave 64 sin .5 .5 0 .5	
		
		{ 
			map textures/liquids/pool3d_5c.tga
			blendFunc GL_dst_color GL_one
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 1.5 0 1.5 1 1 2
			tcmod scroll -.05 .001
		}
	
		{ 
			map textures/liquids/pool3d_6c.tga
			blendFunc GL_dst_color GL_one
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 0 1.5 1 1.5 2 1
			tcmod scroll .025 -.001
		}

		{ 
			map textures/liquids/pool3d_3c.tga
			blendFunc GL_dst_color GL_one
			rgbgen identity
			tcmod scale .25 .5
			tcmod scroll .001 .025
		}
	
		{
			map $lightmap
			blendFunc GL_dst_color GL_zero
			rgbgen identity		
		}
	

}


textures/liquids/clear_ripple2
	{
		qer_editorimage textures/liquids/pool3d_3b.tga
		qer_trans .5
		q3map_globaltexture
		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water

		cull disable
		deformVertexes wave 64 sin .5 .5 0 .5	

		{ 
			map textures/liquids/pool3d_5b.tga
			blendFunc GL_dst_color GL_one
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 1.5 0 1.5 1 1 2
			tcmod scroll -.05 .001
		}
	
		{ 
			map textures/liquids/pool3d_6b.tga
			blendFunc GL_dst_color GL_one
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 0 1.5 1 1.5 2 1
			tcmod scroll .025 -.001
		}

		{ 
			map textures/liquids/pool3d_3b.tga
			blendFunc GL_dst_color GL_one
			rgbgen identity
			tcmod scale .25 .5
			tcmod scroll .001 .025
		}

		{
			map $lightmap
			blendFunc GL_dst_color GL_zero
			rgbgen identity		
		}
	

}


textures/liquids/clear_ripple1
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
			rgbgen identity
			tcmod scale .25 .5
			tcmod scroll .001 .025
		}	

		{
			map $lightmap
			blendFunc GL_dst_color GL_zero
			rgbgen identity		
		}

}

textures/liquids/clear_ripple1_q3dm1
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
			rgbgen identity
			tcmod scale .25 .5
			tcmod scroll .001 .025
		}	

		{
			map $lightmap
			blendFunc GL_dst_color GL_zero
			rgbgen identity		
		}
}

textures/liquids/lavahell
{
	q3map_globaltexture
	surfaceparm noimpact
	surfaceparm trans
	surfaceparm lava
	surfaceparm nolightmap
	q3map_surfacelight 600
	cull disable
	
	tesssize 128
	cull disable
	deformVertexes wave 100 sin 3 2 .1 0.1
	
	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .2 0 .1
	}


}

textures/liquids/lavahell_xdm1
{
	qer_editorimage textures/liquids/lavahell.tga
	q3map_globaltexture
	surfaceparm nodlight
	surfaceparm noimpact
	surfaceparm nolightmap
	q3map_surfacelight 600
	cull disable
	
	tesssize 128
	cull disable
	deformVertexes wave 100 sin 3 2 .1 0.1
	
	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .2 0 .1
	}
	
}

textures/liquids/lavahell_1000
{
	qer_editorimage textures/liquids/lavahell.tga
	q3map_globaltexture
	surfaceparm trans
	surfaceparm noimpact
	surfaceparm lava
	surfaceparm nolightmap
	q3map_surfacelight 1000
	cull disable
	
	tesssize 128
	cull disable
	deformVertexes wave 100 sin 3 2 .1 0.1
	
	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .2 0 .1
	}
}

textures/liquids/lavahell_2000
	{
		qer_editorimage textures/liquids/lavahell.tga
		q3map_globaltexture
		surfaceparm trans
		surfaceparm noimpact
		surfaceparm lava
		surfaceparm nolightmap
		q3map_surfacelight 2000
		cull disable
	
		tesssize 128
		cull disable
		deformVertexes wave 100 sin 3 2 .1 0.1
	
	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .2 0 .1
	}
}
textures/liquids/lavahell_750
	{

		qer_editorimage textures/liquids/lavahell.tga
		q3map_globaltexture
		q3map_lightsubdivide 32
		surfaceparm trans
		surfaceparm noimpact
		surfaceparm lava
		surfaceparm nolightmap
		q3map_surfacelight 500
		cull disable
	
	tesssize 128
	cull disable
	deformVertexes wave 100 sin 3 2 .1 0.1
	
	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .2 0 .1
	}
}

textures/liquids/flatlavahell_1500
	{
		qer_editorimage textures/liquids/lavahell.tga
		q3map_globaltexture
		q3map_lightsubdivide 32
		surfaceparm noimpact
		surfaceparm lava
		surfaceparm nolightmap
		q3map_surfacelight 1500
		cull disable
	
	deformVertexes wave 100 sin 3 2 .1 0.1
	
	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .2 0 .1
	}
       
}


textures/liquids/lavahell_2500
	{

		qer_editorimage textures/liquids/lavahell.tga
		q3map_globaltexture
		surfaceparm trans
		surfaceparm noimpact
		surfaceparm lava
		surfaceparm nolightmap
		q3map_surfacelight 2500
		cull disable
		
		tesssize 128
		cull disable
		deformVertexes wave 100 sin 3 2 .1 0.1
	
	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .2 0 .1
	}
}

textures/liquids/lavahelldark
	{
		qer_editorimage textures/liquids/lavahell.tga
		q3map_globaltexture
		surfaceparm trans
		surfaceparm noimpact
		surfaceparm lava
		surfaceparm nolightmap
		q3map_surfacelight 150
		cull disable
	
		tesssize 128
		cull disable
		deformVertexes wave 100 sin 3 2 .1 0.1
	
	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .2 0 .1
	}
	

}

textures/liquids/lavahellflat_400
{
	
	qer_editorimage textures/liquids/lavahell.tga
	q3map_globaltexture
	surfaceparm trans
	surfaceparm noimpact
	surfaceparm lava
	surfaceparm nolightmap
	q3map_surfacelight 400
	cull disable
	
	tesssize 128
	cull disable
	
	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .2 0 .1
	}
	
}

textures/liquids/calm_pool
	{
		qer_editorimage textures/liquids/pool2.tga
		qer_trans .5
		q3map_globaltexture

		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water
		surfaceparm nolightmap

		cull disable
		tesssize 64
		deformVertexes wave 100 sin 1 1 1 .1
		
	
	
		{
			map textures/liquids/pool2.tga
			blendfunc GL_ONE GL_SRC_COLOR	
			tcMod scale .03 .03
			tcMod scroll .001 .001
		}
	
		{
			map textures/liquids/pool3.tga
			blendfunc GL_DST_COLOR GL_ONE
			tcMod turb .1 .1 0 .01
			tcMod scale .5 .5
			tcMod scroll -.025 .02
		}


}
	
textures/liquids/calm_pool_bgtess
	{
		qer_editorimage textures/liquids/pool2.tga
		qer_trans .5
		q3map_globaltexture

		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water
		surfaceparm nolightmap

		cull disable
		tesssize 256
		deformVertexes wave 256 sin 1 1 1 .1
		
	
	
		{
			map textures/liquids/pool2.tga
			blendfunc GL_ONE GL_SRC_COLOR	
			tcMod scale .03 .03
			tcMod scroll .001 .001
		}
	
		{
			map textures/liquids/pool3.tga
			blendfunc GL_DST_COLOR GL_ONE
			tcMod turb .1 .1 0 .01
			tcMod scale .5 .5
			tcMod scroll -.025 .02
		}


}

textures/liquids/ripplewater2_back
{
    qer_editorimage textures/liquids/ripplewater3.tga
		q3map_globaltexture
		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water
		surfaceparm nolightmap

		{
			map textures/liquids/ripplewater3.tga
			blendfunc GL_DST_COLOR GL_SRC_COLOR
			tcMod scale .05 .05
			tcGen environment
			tcMod scroll .001 .001
		}

}

textures/liquids/slime1
	{
		qer_editorimage textures/liquids/slime7.tga
		q3map_lightimage textures/liquids/slime7.tga
		q3map_globaltexture
		qer_trans .5

		surfaceparm noimpact
		surfaceparm slime
		surfaceparm nolightmap
		surfaceparm trans		

		q3map_surfacelight 100
		tessSize 32
		cull disable

		deformVertexes wave 100 sin 0 1 .5 .5

		{
			map textures/liquids/slime7c.tga
			tcMod turb .3 .2 1 .05
			tcMod scroll .01 .01
		}
	
		{
			map textures/liquids/slime7.tga
			blendfunc GL_ONE GL_ONE
			tcMod turb .2 .1 1 .05
			tcMod scale .5 .5
			tcMod scroll .01 .01
		}

		{
			map textures/liquids/bubbles.tga
			blendfunc GL_ZERO GL_SRC_COLOR
			tcMod turb .2 .1 .1 .2
			tcMod scale .05 .05
			tcMod scroll .001 .001
		}		


}

textures/liquids/slime1_2000
	{
		qer_editorimage textures/liquids/slime7.tga
		q3map_lightimage textures/liquids/slime7.tga
		q3map_globaltexture
		qer_trans .5

		surfaceparm noimpact
		surfaceparm slime
		surfaceparm nolightmap
		surfaceparm trans		

		q3map_surfacelight 2000
		tessSize 32
		cull disable

		deformVertexes wave 100 sin 0 1 .5 .5

		{
			map textures/liquids/slime7c.tga
			tcMod turb .3 .2 1 .05
			tcMod scroll .01 .01
		}
	
		{
			map textures/liquids/slime7.tga
			blendfunc GL_ONE GL_ONE
			tcMod turb .2 .1 1 .05
			tcMod scale .5 .5
			tcMod scroll .01 .01
		}

		{
			map textures/liquids/bubbles.tga
			blendfunc GL_ZERO GL_SRC_COLOR
			tcMod turb .2 .1 .1 .2
			tcMod scale .05 .05
			tcMod scroll .001 .001
		}		


}

textures/hell/lavasolid
{
	q3map_globaltexture
	surfaceparm noimpact
	surfaceparm nolightmap
	q3map_surfacelight 450

	tesssize 64
	deformVertexes wave 100 sin 2 2 0 0.1

	{
		map textures/hell/lava2d.tga
		tcGen environment
		tcMod turb 0 0.25 0 0.05
	}

	{
		map textures/hell/lava2d.tga
		blendfunc GL_ONE GL_ONE
		tcMod turb 0 0.25 0 0.05
	}
}

textures/liquids/lavahell_2500_subd
{
	qer_editorimage textures/liquids/lavahell.tga
	q3map_lightsubdivide 32
	q3map_globaltexture
	surfaceparm trans
	surfaceparm noimpact
	surfaceparm lava
	surfaceparm nolightmap
	q3map_surfacelight 2500
	cull disable
	
	tesssize 128
	cull disable
	deformVertexes wave 100 sin 3 2 .1 0.1
	
	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .2 0 .1
	}
}
