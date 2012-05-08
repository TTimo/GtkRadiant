textures/base_wall2/blue_warning_light
{

     qer_editorimage textures/proto2/bluelight_off.tga

        {
   		 map textures/proto2/bluelight_off.tga
   		 rgbgen identity
        }

	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}

}



textures/base_wall2/ntrl_techfloor_kc
{    
     	surfaceparm	metalsteps
	surfaceparm 	alphashadow
	surfaceparm	nomarks	 
        qer_editorimage textures/base_wall2/techfloor_kc.tga
        
        {
		clampmap textures/base_wall2/zzztntrl_kc.tga
               	alphaFunc GE128
		tcmod rotate 360
		rgbgen wave sin .8 .2 0 2
		depthWrite
	}
        {
                clampmap textures/base_wall2/techfloor2_kc.tga
		blendFunc blend
                tcmod rotate 180
		depthFunc equal
                tcMod stretch sin .8 0.1 0 .4
	        rgbGen identity
	}
        
	{
		map textures/base_wall2/techfloor_kc.tga
                blendFunc blend
		depthFunc equal
		rgbGen identity
	}
	
	{
		map $lightmap
		blendfunc filter
		depthFunc equal
		rgbgen identity
	}
	{
	        map textures/base_wall2/techfloor_kc_shadow.tga
                blendfunc filter
	        rgbGen identity
	}

        
}
textures/base_wall2/respawn_effect1_ntrl
{
    	cull disable
    	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nomarks
	qer_editorimage textures/base_wall2/ntrl_1_kc.tga
 	{
		map textures/base_wall2/ntrl_1_kc.tga
                blendFunc add
		tcmod scale 1 -1
		rgbgen wave sin .25 .25 0 .25
                tcmod scroll .5 0
	}
	{
		map textures/base_wall2/ntrl_2_kc.tga
                blendFunc add
		rgbgen wave sin .25 .25 .25 .25
		tcmod scale 1 -1
                tcmod scroll -.5 0
	}


	 {
		map textures/base_wall2/ntrl_3_kc.tga
                blendFunc add
		tcmod scale 1 -1
                rgbGen wave inversesawtooth -.3 1.3 0 .25
	}

	
}

textures/base_wall2/blue_metal
{

	{
		map textures/base_wall/chrome_env2.tga
	        rgbGen identity
		tcGen environment
		tcmod scale .25 .25
	}
	
	{
		map textures/base_wall2/blue_metal.tga
		blendFunc blend	
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
}
textures/base_wall2/scuffs1
{

	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap

	{
		map textures/base_wall2/scuffs1.tga
		blendfunc filter
		rgbGen identity
	}
}

textures/base_wall2/space_panel
{
	
		qer_editorimage textures/base_wall2/space_panel_alt2.tga
	{
		map textures/base_wall2/space_panel_alt.tga
		rgbgen identity
		tcmod scale .5 .5
	}	

	{
		map textures/base_wall2/space_panel_alt.tga
		blendFunc filter
		rgbgen wave sin .5 0 0 0 
	}
	
	
	{
		map textures/base_wall2/lensflare.tga
		blendFunc add
		tcgen environment
	}

        
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}	
}




textures/base_wall2/space_concrete
{
	qer_editorimage textures/base_wall2/space_panel_alt.tga

	{
		map textures/base_wall/concrete1.tga
		rgbgen identity
	}	

	{
		map textures/base_wall2/space_panel_alt.tga
		blendFunc filter
		tcmod scale .25 .25
	}
	
	
	{
		map textures/base_wall2/space_panel_alt2.tga
		blendFunc filter
		tcmod scale .25 .1
	}
	{
		map textures/base_wall2/lensflare.tga
		blendFunc add
		rgbgen wave sin .5 0 0 0 
		tcgen environment
	}
        
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}	
}


textures/base_wall2/jumppad_red_kc
{      
     	surfaceparm trans
	surfaceparm nomarks
        {
	        map textures/base_wall2/jumppad_red_kc.tga
                alphafunc GE128
		depthwrite
	        rgbGen identity
	}

        {
	        map textures/sfx2/jumpadr.tga
                blendFunc GL_ONE GL_ZERO
		depthfunc equal
                tcmod rotate 130
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		blendFunc blend
		depthfunc equal
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadr2.tga
		blendfunc Add
		depthfunc equal
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad01.tga
                blendFunc blend
		depthfunc equal
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc filter
		depthfunc equal
		rgbGen identity
	}

	{
	        map textures/base_wall2/jumppad_shadow.tga
                blendfunc filter
	        rgbGen identity
	}

}

textures/base_wall2/jumppad_blue_kc
{      
     	surfaceparm trans
	surfaceparm nomarks
        {
	        map textures/base_wall2/jumppad_blue_kc.tga
                alphafunc GE128
		depthwrite
	        rgbGen identity
	}

        {
	        map textures/sfx2/jumpadb.tga
                blendFunc GL_ONE GL_ZERO
		depthfunc equal
                tcmod rotate 130
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		blendFunc blend
		depthfunc equal
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadb2.tga
		blendfunc Add
		depthfunc equal
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad01.tga
                blendFunc blend
		depthfunc equal
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc filter
		depthfunc equal
		rgbGen identity
	}

	{
	        map textures/base_wall2/jumppad_shadow.tga
                blendfunc filter
	        rgbGen identity
	}

}

textures/base_wall2/jumppad_ntrl_kc
{      
	qer_editorimage textures/base_wall2/jumppad_blue_kc.tga
     	surfaceparm trans
	surfaceparm nomarks
        {
	        map textures/base_wall2/jumppad_blue_kc.tga
                alphafunc GE128
		depthwrite
	        rgbGen identity
	}

        {
	        map textures/sfx2/jumpadn.tga
                blendFunc GL_ONE GL_ZERO
		depthfunc equal
                tcmod rotate 130
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		blendFunc blend
		depthfunc equal
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadn2.tga
		blendfunc Add
		depthfunc equal
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad01.tga
                blendFunc blend
		depthfunc equal
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc filter
		depthfunc equal
		rgbGen identity
	}

	{
	        map textures/base_wall2/jumppad_shadow.tga
                blendfunc filter
	        rgbGen identity
	}

}


textures/base_wall2/respawn_effect1
{
    	cull disable
    	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nomarks
	qer_editorimage textures/base_wall2/redjet_1_kc.tga
 	{
		map textures/base_wall2/redjet_1_kc.tga
                blendFunc add
		tcmod scale 1 -1
		rgbgen wave sin .25 .25 0 .25
                tcmod scroll .5 0
	}
	{
		map textures/base_wall2/redjet_2_kc.tga
                blendFunc add
		rgbgen wave sin .25 .25 .25 .25
		tcmod scale 1 -1
                tcmod scroll -.5 0
	}


	 {
		map textures/base_wall2/redjet_3_kc.tga
                blendFunc add
		tcmod scale 1 -1
                rgbGen wave inversesawtooth -.3 1.3 0 .25
	}




	
	
}

textures/base_wall2/respawn_effect1_blue
{
    	cull disable
    	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nomarks
	qer_editorimage textures/base_wall2/bluejet_1_kc.tga
 	{
		map textures/base_wall2/bluejet_1_kc.tga
                blendFunc add
		tcmod scale 1 -1
		rgbgen wave sin .25 .25 0 .25
                tcmod scroll .5 0
	}
	{
		map textures/base_wall2/bluejet_2_kc.tga
                blendFunc add
		rgbgen wave sin .25 .25 .25 .25
		tcmod scale 1 -1
                tcmod scroll -.5 0
	}


	 {
		map textures/base_wall2/bluejet_3_kc.tga
                blendFunc add
		tcmod scale 1 -1
                rgbGen wave inversesawtooth -.3 1.3 0 .25
	}

	
}

textures/base_wall2/red_metal
{

	{
		map textures/base_wall/chrome_env2.tga
	        rgbGen identity
		tcGen environment
		tcmod scale .25 .25
	}
	
	{
		map textures/base_wall2/red_metal.tga
		blendFunc blend	
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
}

textures/base_wall2/blue_line
{
	q3map_surfacelight 100
	q3map_lightimage textures/base_wall2/blue_line_glow.tga

	{
		map textures/base_wall/chrome_env.tga
	        rgbGen identity
		tcGen environment
		tcmod scale .25 .25
	}
	
	{
		map textures/base_wall2/blue_line.tga
		blendFunc blend	
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}

	{
		map textures/base_wall2/blue_line_glow.tga
		blendFunc add	
	//	rgbGen wave sin .5 .5 0 1	
	}


}

textures/base_wall2/red_line
{
	q3map_surfacelight 100
	q3map_lightimage textures/base_wall2/red_line_glow.tga

	{
		map textures/base_wall/chrome_env.tga
	        rgbGen identity
		tcGen environment
		tcmod scale .25 .25
	}
	
	{
		map textures/base_wall2/red_line.tga
		blendFunc blend	
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}

	{
		map textures/base_wall2/red_line_glow.tga
		blendFunc add	
	//	rgbGen wave sin .5 .5 0 1	
	}
}


textures/base_wall2/double_line_blue
{

	surfaceparm trans
	surfaceparm nomipmap
	surfaceparm nolightmap

	{
		map textures/base_wall2/double_line_blue.tga
		blendfunc gl_src_alpha gl_one_minus_src_alpha
		rgbGen identity

	}

}

textures/base_wall2/double_line_red
{

	surfaceparm trans
	surfaceparm nomipmap
	surfaceparm nolightmap

	{
		map textures/base_wall2/double_line_red.tga
		blendfunc gl_src_alpha gl_one_minus_src_alpha
		rgbGen identity

	}

}

textures/base_wall2/techfloor_kc_blue
{    
	qer_editorimage textures/base_wall2/zzztblue_kc.tga.tga
     	surfaceparm	metalsteps
	surfaceparm 	alphashadow
	surfaceparm	nomarks	   
        
        {
		clampmap textures/base_wall2/zzztblue_kc.tga
               	alphaFunc GE128
		tcmod rotate 360
		rgbgen wave sin .8 .2 0 2
		depthWrite
	}
        {
                clampmap textures/base_wall2/techfloor2_kc.tga
		blendFunc blend
                tcmod rotate 180
		depthFunc equal
                tcMod stretch sin .8 0.1 0 .4
	        rgbGen identity
	}
        
	{
		map textures/base_wall2/techfloor_kc.tga
                blendFunc blend
		depthFunc equal
		rgbGen identity
	}
	
	{
		map $lightmap
		blendfunc filter
		depthFunc equal
		rgbgen identity
	}
	{
	        map textures/base_wall2/techfloor_kc_shadow.tga
                blendfunc filter
	        rgbGen identity
	}

        
}


textures/base_wall2/techfloor_kc
{    
     	surfaceparm	metalsteps
	surfaceparm 	alphashadow
	surfaceparm	nomarks	   
        
        {
		clampmap textures/base_wall2/zzztred_kc.tga
               	alphaFunc GE128
		tcmod rotate 360
		rgbgen wave sin .8 .2 0 2
		depthWrite
	}
        {
                clampmap textures/base_wall2/techfloor2_kc.tga
		blendFunc blend
                tcmod rotate 180
		depthFunc equal
                tcMod stretch sin .8 0.1 0 .4
	        rgbGen identity
	}
        
	{
		map textures/base_wall2/techfloor_kc.tga
                blendFunc blend
		depthFunc equal
		rgbGen identity
	}
	
	{
		map $lightmap
		blendfunc filter
		depthFunc equal
		rgbgen identity
	}
	{
	        map textures/base_wall2/techfloor_kc_shadow.tga
                blendfunc filter
	        rgbGen identity
	}

        
}

textures/base_wall2/blue_arrow_small
{
  	
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap


	{
		map textures/base_wall2/blue_arrow_small.tga
		blendfunc blend
		rgbGen identity

	}


}
textures/base_wall2/red_arrow_small
{
  	
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap


	{
		map textures/base_wall2/red_arrow_small.tga
		blendfunc blend
		rgbGen identity

	}


}

textures/base_wall2/yellow_circle
{
  	
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap


	{
		map textures/base_wall2/yellow_circle.tga
		blendfunc gl_src_alpha gl_one_minus_src_alpha
		rgbGen identity

	}


}

textures/base_wall2/red_circle
{
  	
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap


	{
		map textures/base_wall2/red_circle.tga
		blendfunc gl_src_alpha gl_one_minus_src_alpha
		rgbGen identity

	}


}

textures/base_wall2/blue_circle
{
  	
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap


	{
		map textures/base_wall2/blue_circle.tga
		blendfunc gl_src_alpha gl_one_minus_src_alpha
		rgbGen identity

	}


}


textures/base_wall2/name_vortex
{
  	
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap

	{
		map textures/base_wall2/name_vortex.tga
		blendfunc blend
		rgbGen identity      
	}

}


textures/base_wall2/bluearrows
{
  	
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap


	{
		map textures/base_wall2/bluearrows.tga
		blendfunc add
		rgbGen identity

	}


}

textures/base_wall2/redarrows
{
  	
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap


	{
		map textures/base_wall2/redarrows.tga
		blendfunc add
		rgbGen identity

	}


}

textures/base_wall2/red_solid
{
	q3map_surfacelight 100
	q3map_lightimage textures/base_wall2/red_solid.tga

	{
		map textures/base_wall/chrome_env.tga
	        rgbGen identity
		tcGen environment
		tcmod scale .25 .25
	}
	
	{
		map textures/base_wall2/red_solid.tga
		blendFunc filter	
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}

	{
		map textures/base_wall2/red_solid.tga
		blendFunc add	
		rgbGen wave sin .25 .25 0 1	
	}


}

textures/base_wall2/blue_solid
{
	q3map_surfacelight 100
	q3map_lightimage textures/base_wall2/blue_solid.tga

	{
		map textures/base_wall/chrome_env.tga
	        rgbGen identity
		tcGen environment
		tcmod scale .25 .25
	}
	
	{
		map textures/base_wall2/blue_solid.tga
		blendFunc filter	
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}

	{
		map textures/base_wall2/blue_solid.tga
		blendFunc add	
		rgbGen wave sin .25 .25 0 1	
	}


}






