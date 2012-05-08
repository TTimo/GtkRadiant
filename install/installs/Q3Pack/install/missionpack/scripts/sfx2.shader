textures/sfx2/senbouncenolit
{
	surfaceparm nodamage
	surfaceparm nomarks
	q3map_lightimage textures/sfx/jumppadsmall.tga	
	qer_editorimage textures/sfx/bouncepad01block18b.tga

	{
		map textures/sfx/bouncepad01block18b.tga
		rgbGen identity
	}
	
	{
		map $lightmap
		rgbGen identity
		blendfunc gl_dst_color gl_zero
	}
	
	{
		map textures/sfx/bouncepad01b_layer1.tga
		blendfunc gl_one gl_one
		rgbGen wave sin .5 .5 0 1.5	
	}

	{
		clampmap textures/sfx/jumppadsmall.tga
		blendfunc gl_one gl_one
		tcMod stretch sin 1.2 .8 0 1.5
		rgbGen wave square .5 .5 .25 1.5
	}
}


textures/sfx2/flame_nolit_noshad
{
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm trans
	cull none
	qer_editorimage textures/sfx/flame1.tga
	

	{
		animMap 10 textures/sfx/flame1.tga textures/sfx/flame2.tga textures/sfx/flame3.tga textures/sfx/flame4.tga textures/sfx/flame5.tga textures/sfx/flame6.tga textures/sfx/flame7.tga textures/sfx/flame8.tga
		blendFunc GL_ONE GL_ONE
	}	

}

textures/sfx2/demonltblacknolight
{
	q3map_lightimage textures/sfx/demonltblackfinal_glow2.tga
	surfaceparm nomarks
	qer_editorimage textures/sfx/demonltblackfinal.tga
	
	{
		map $lightmap
		rgbgen identity
	}

	{
		map textures/sfx/demonltblackfinal.tga
		blendfunc gl_dst_color gl_zero
		rgbgen identity
	}

	{
		map textures/sfx/demonltblackfinal_glow2.tga
		blendfunc gl_one gl_one
		rgbgen wave sin .9 .1 0 5
	}

}

textures/sfx2/jumpadx
{      
   
        
        {
	        map textures/sfx2/jumpadn.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadn2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpadx.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}

textures/sfx2/beam01
{
        surfaceparm trans	
        surfaceparm nomarks	
        surfaceparm nonsolid
	surfaceparm nolightmap
	cull none
        nomipmaps
	{
		map textures/sfx2/beam01.tga
                tcMod Scroll .1 0
                blendFunc add
        }
     
}

textures/sfx2/ouchfog
{
        qer_editorimage textures/sfx/hellfog.tga
	surfaceparm	trans
	surfaceparm	nonsolid
	surfaceparm	fog
	surfaceparm 	nodrop
	surfaceparm 	nolightmap
	q3map_globaltexture
	q3map_surfacelight 50
	fogparms ( .66 0 0 ) 392

	
	
	{
		map textures/sfx2/ouchfog.tga
		blendfunc gl_dst_color gl_zero
		tcmod scale -.05 -.05
		tcmod scroll .01 -.01
		rgbgen identity
	}

	{
		map textures/sfx2/ouchfog.tga
		blendfunc gl_dst_color gl_zero
		tcmod scale .05 .05
		tcmod scroll .01 -.01
		rgbgen identity
	}

}


textures/sfx2/autorain
{
        surfaceparm trans	
        surfaceparm nomarks	
        surfaceparm nonsolid
	surfaceparm nolightmap
        qer_trans .5
        deformVertexes move 3 1 0  sin 0 5 0 0.2
        deformVertexes move .6 3.3 0  sin 0 5 0 0.4
        deformVertexes autoSprite2
	cull none
        //nopicmip
       
	{
		map textures/sfx2/autorain.tga
                tcMod Scroll .5 -8
                tcMod turb .1 .25 0 -.1
                blendFunc GL_ONE GL_ONE
        }
        //{
	//	map textures/sfx/rain.tga
        //        tcMod Scroll .01 -6.3
        //       
        //        blendFunc GL_ONE GL_ONE
        //}
       
     
}

textures/sfx2/flameanim_red_lowlite
{

	//	*************************************************
	//	*      	Red Flame 				*
	//	*      	April 24, 2000 Surface Light 1000    	*	
	//	*	Please Comment Changes			*
	//	*************************************************
	qer_editorimage textures/sfx/r_flame3.tga
	q3map_lightimage textures/sfx/r_flame3.tga
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap
	cull none
	q3map_surfacelight 1000
	// surfacelight changed to 1000 on 04/24/00.... PAJ

	{
		animMap 10 textures/sfx/r_flame1.tga textures/sfx/r_flame2.tga textures/sfx/r_flame3.tga textures/sfx/r_flame4.tga textures/sfx/r_flame5.tga textures/sfx/r_flame6.tga textures/sfx/r_flame7.tga textures/sfx/r_flame8.tga
		blendFunc GL_ONE GL_ONE
		rgbGen wave inverseSawtooth 0 1 0 10
		
	}	
	{
		animMap 10 textures/sfx/r_flame2.tga textures/sfx/r_flame3.tga textures/sfx/r_flame4.tga textures/sfx/r_flame5.tga textures/sfx/r_flame6.tga textures/sfx/r_flame7.tga textures/sfx/r_flame8.tga textures/sfx/r_flame1.tga
		blendFunc GL_ONE GL_ONE
		rgbGen wave sawtooth 0 1 0 10
	}	


	{
		map textures/sfx/r_flameball.tga
		blendFunc GL_ONE GL_ONE
		rgbGen wave sin .6 .2 0 .6	
	}

}
textures/sfx2/flameanim_blue_lowlite
{

	//	*************************************************
	//	*      	Blue Flame 				*
	//	*      	April 24, 2000 Surface Light 1000    	*	
	//	*	Please Comment Changes			*
	//	*************************************************
	qer_editorimage textures/sfx/b_flame7.tga
	q3map_lightimage textures/sfx/b_flame7.tga
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap
	cull none
	q3map_surfacelight 1000
	// texture changed to 1K surface light.... PAJ
	

	{
		animMap 10 textures/sfx/b_flame1.tga textures/sfx/b_flame2.tga textures/sfx/b_flame3.tga textures/sfx/b_flame4.tga textures/sfx/b_flame5.tga textures/sfx/b_flame6.tga textures/sfx/b_flame7.tga textures/sfx/b_flame8.tga
		blendFunc GL_ONE GL_ONE
		rgbGen wave inverseSawtooth 0 1 0 10
		
	}	
	{
		animMap 10 textures/sfx/b_flame2.tga textures/sfx/b_flame3.tga textures/sfx/b_flame4.tga textures/sfx/b_flame5.tga textures/sfx/b_flame6.tga textures/sfx/b_flame7.tga textures/sfx/b_flame8.tga textures/sfx/b_flame1.tga
		blendFunc GL_ONE GL_ONE
		rgbGen wave sawtooth 0 1 0 10
	}	


	{
		map textures/sfx/b_flameball.tga
		blendFunc GL_ONE GL_ONE
		rgbGen wave sin .6 .2 0 .6	
	}

}

textures/sfx2/blue_jumpad05
{      
      qer_editorimage textures/sfx2/jumpad05.tga 
        
        {
	        map textures/sfx2/jumpadb.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadb2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad05.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}

textures/sfx2/red_jumpad05
{      
      qer_editorimage textures/sfx2/jumpad05.tga 
        
        {
	        map textures/sfx2/jumpadr.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadr2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad05.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}
textures/sfx2/screen01
{
    cull disable
    surfaceparm nomarks

	{
		map textures/sfx2/screen01.tga
                tcMod scroll 9 .1
               	rgbGen identity
	}
        {
		map textures/effects2/tinfx_scr.tga
                blendfunc add
                tcGen environment
               	rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
        


}

textures/sfx2/blue_edge
{
    cull disable
    surfaceparm trans
    surfaceparm nolightmap
   
	{
		map textures/sfx2/blue_edge.tga
		blendFunc add
                tcMod scale .2 .2
                tcMod scroll .2 .2
              	rgbGen identity
	}
        


}
textures/sfx2/b_smack01
{
    cull disable
    surfaceparm trans
    surfaceparm nolightmap
	{
		map textures/sfx2/b_smack01.tga
		blendFunc add
                tcMod turb 0 .05 .06 .25
		rgbgen wave sin 0 1 0 .19
	}
        {
		map textures/sfx2/b_smack03.tga
		blendFunc add
                tcMod turb 0 .05 .07 .33
		rgbgen wave sin 0 1 0 7
	}
        {
		map textures/sfx2/b_smack02.tga
		blendFunc add
                tcMod turb 0 .07 .05 .4
		rgbgen wave sin 0 1 0 .5
	}
        


}
textures/sfx2/r_fight01
{
    cull disable
    surfaceparm trans
    surfaceparm nolightmap
	{
		map textures/sfx2/r_fight01.tga
		blendFunc add
                tcMod turb 0 .05 .06 .25
		rgbgen wave sin 0 1 0 .19
	}
        {
		map textures/sfx2/r_fight02.tga
		blendFunc add
                tcMod turb 0 .05 .07 .33
		rgbgen wave sin 0 1 0 7
	}
        {
		map textures/sfx2/r_fight03.tga
		blendFunc add
                tcMod turb 0 .07 .05 .4
		rgbgen wave sin 0 1 0 .5
	}
        


}

textures/sfx2/ven_beam
{
    qer_editorimage textures/sfx2/ven_beam.tga
    cull disable
    surfaceparm trans
   
	{
		map textures/sfx2/ven_beam.tga
		blendFunc add
		rgbGen identity
	}
        


}

textures/sfx2/padwall_red
	{
	
	q3map_lightimage textures/sfx2/dm3padwallglow_red.tga
	qer_editorimage textures/sfx2/dm3padwall_red.tga
	
	{
		map textures/sfx2/dm3padwall_red.tga
		rgbGen identity
	}
	
	{
		map $lightmap
		rgbGen identity
		blendfunc gl_dst_color gl_zero
	}


	{
		map textures/sfx2/dm3padwallglow_red.tga
		blendfunc gl_one gl_one
		rgbgen wave sin 0 1 0 .5
		tcmod scale 1 .05
		tcmod scroll 0 1
	}
	
}

textures/sfx2/padwall_red_light
	{
	
	q3map_lightimage textures/sfx2/dm3padwallglow_red.tga
	qer_editorimage textures/sfx2/dm3padwall_red.tga
	q3map_surfacelight 100
	q3map_lightsubdivide 64
	{
		map textures/sfx2/dm3padwall_red.tga
		rgbGen identity
	}
	
	{
		map $lightmap
		rgbGen identity
		blendfunc gl_dst_color gl_zero
	}


	{
		map textures/sfx2/dm3padwallglow_red.tga
		blendfunc gl_one gl_one
		rgbgen wave sin 0 1 0 .5
		tcmod scale 1 .05
		tcmod scroll 0 1
	}
	
}



textures/sfx2/padwall_ntl
	{
	
	q3map_lightimage textures/sfx2/dm3padwallglow_ntl.tga
	qer_editorimage textures/sfx2/dm3padwall_red.tga
	
	{
		map textures/sfx2/dm3padwall_red.tga
		rgbGen identity
	}
	
	{
		map $lightmap
		rgbGen identity
		blendfunc gl_dst_color gl_zero
	}


	{
		map textures/sfx2/dm3padwallglow_ntl.tga
		blendfunc gl_one gl_one
		rgbgen wave sin 0 1 0 .5
		tcmod scale 1 .05
		tcmod scroll 0 1
	}
	
}

textures/sfx2/ntrl_launchpad
{      
      qer_editorimage textures/sfx2/launchpad03.tga 
        
        {
	        map textures/sfx2/tread.tga
                blendFunc GL_ONE GL_ZERO
                tcmod scroll 0 .7
                tcmod scale  1 2
	        rgbGen identity
	}
        {
	        map textures/sfx2/nlaunch.tga
                blendFunc Add
                tcmod scale  1 .5
                tcmod scroll 0 2
	        rgbgen wave square 0 1 0 2
                alphagen wave square 0 1 .1 2
	}
        
        {
		map textures/sfx2/launchpad03.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
        {
	        map textures/sfx2/nlaunch2.tga
                blendFunc Add
	        rgbgen wave square 0 1 0 1
                alphagen wave square 0 1 .1 1
            
	}
       
}
textures/sfx2/blue_launchpad
{      
      qer_editorimage textures/sfx2/launchpad03.tga 
        
        {
	        map textures/sfx2/tread.tga
                blendFunc GL_ONE GL_ZERO
                tcmod scroll 0 .7
                tcmod scale  1 2
	        rgbGen identity
	}
        {
	        map textures/sfx2/blaunch.tga
                blendFunc Add
                tcmod scale  1 .5
                tcmod scroll 0 2
	        rgbgen wave square 0 1 0 2
                alphagen wave square 0 1 .1 2
	}
        
        {
		map textures/sfx2/launchpad03.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
        {
	        map textures/sfx2/blaunch2.tga
                blendFunc Add
	        rgbgen wave square 0 1 0 1
                alphagen wave square 0 1 .1 1
            
	}
       
}
textures/sfx2/red_launchpad
{      
      qer_editorimage textures/sfx2/launchpad03.tga 
        
        {
	        map textures/sfx2/tread.tga
                blendFunc GL_ONE GL_ZERO
                tcmod scroll 0 .7
                tcmod scale  1 2
	        rgbGen identity
	}
        {
	        map textures/sfx2/rlaunch.tga
                blendFunc Add
                tcmod scale  1 .5
                tcmod scroll 0 2
	        rgbgen wave square 0 1 0 2
                alphagen wave square 0 1 .1 2
	}
        
        {
		map textures/sfx2/launchpad03.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
        {
	        map textures/sfx2/rlaunch2.tga
                blendFunc Add
	        rgbgen wave square 0 1 0 1
                alphagen wave square 0 1 .1 1
            
	}
       
}


textures/sfx2/tread2
{      
      
        {
	        map textures/sfx2/tread2.tga
                blendFunc GL_ONE GL_ZERO
                tcmod scroll 0 1
	        rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}
textures/sfx2/ntrl_jumpad
{      
      qer_editorimage textures/sfx2/jumpad02.tga 
        
        {
	        map textures/sfx2/jumpadn.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadn2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad02.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}
textures/sfx2/blue_jumpad
{      
      qer_editorimage textures/sfx2/jumpad02.tga 
        
        {
	        map textures/sfx2/jumpadb.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadb2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad02.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}

textures/sfx2/red_jumpad
{      
      qer_editorimage textures/sfx2/jumpad02.tga 
        
        {
	        map textures/sfx2/jumpadr.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadr2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad02.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}
textures/sfx2/ntrl_jumpad2
{      
      qer_editorimage textures/sfx2/jumpad01.tga 
        
        {
	        map textures/sfx2/jumpadn.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadn2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad01.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}
textures/sfx2/blue_jumpad2
{      
      qer_editorimage textures/sfx2/jumpad01.tga 
        
        {
	        map textures/sfx2/jumpadb.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadb2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad01.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}

textures/sfx2/red_jumpad2
{      
      qer_editorimage textures/sfx2/jumpad01.tga 
        
        {
	        map textures/sfx2/jumpadr.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadr2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad01.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}

/////
textures/sfx2/ntrl_jumpad3
{      
      qer_editorimage textures/sfx2/jumpad03.tga 
        
        {
	        map textures/sfx2/jumpadn.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadn2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad03.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}
textures/sfx2/blue_jumpad3
{      
      qer_editorimage textures/sfx2/jumpad03.tga 
        
        {
	        map textures/sfx2/jumpadb.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadb2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad03.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}

textures/sfx2/red_jumpad3
{      
      qer_editorimage textures/sfx2/jumpad03.tga 
        
        {
	        map textures/sfx2/jumpadr.tga
                blendFunc GL_ONE GL_ZERO
                tcmod rotate 130
                //tcMod stretch sin .7 0.5 0 .2
	        rgbGen identity
	}
        {
	        map textures/sfx2/fan01.tga
		  blendFunc blend
                tcmod rotate -311
	        rgbGen identity
	}
        {
		clampmap textures/sfx2/jumpadr2.tga
		blendfunc Add
                tcmod rotate 130
		tcMod stretch sin 1.2 .8 0 1.4
		rgbGen wave square .5 .5 .25 1.4
	}
        {
		map textures/sfx2/jumpad03.tga
                blendFunc blend
		rgbGen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
}

textures/sfx2/redgoal2
{
	surfaceparm nolightmap
	//surfaceparm nonsolid
	surfaceparm trans
	cull twosided
	{
		map textures/sfx2/redgoal2.tga
		tcGen environment
                tcmod scale 2 2
		blendfunc GL_ONE GL_ONE
		tcMod turb 0 0.25 0 0.5
	}
}


textures/sfx2/bluegoal2
{
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	cull twosided
	{
		map textures/sfx2/bluegoal2.tga
		tcGen environment
                tcmod scale 2 2
		blendfunc GL_ONE GL_ONE
		tcMod turb 0 0.25 0 0.5
	}
}
textures/sfx2/swirl_b1
{
        deformVertexes wave 100 sin 1 2 .1 1
        //q3map_surfacelight	300

	surfaceparm	trans
	surfaceparm nomarks
	surfaceparm nolightmap
        surfaceparm nonsolid
	cull none
	
	{
		clampmap textures/sfx2/swirl_b1.tga
		blendFunc GL_ONE GL_ONE
                tcMod rotate -188
	}	
        {
		clampmap textures/sfx2/swirl_b2.tga
		blendFunc GL_ONE GL_ONE
                tcMod rotate 333
	}

}	
textures/sfx2/swirl_r1
{
        deformVertexes wave 100 sin 1 2 .1 1
        //q3map_surfacelight	300

	surfaceparm	trans
	surfaceparm nomarks
	surfaceparm nolightmap
        surfaceparm nonsolid
	cull none
	
	{
		clampmap textures/sfx2/swirl_r1.tga
		blendFunc GL_ONE GL_ONE
                tcMod rotate -188
	}	
        {
		clampmap textures/sfx2/swirl_r2.tga
		blendFunc GL_ONE GL_ONE
                tcMod rotate 333
	}

}	
textures/sfx2/jumportal01
{
	
       	
	surfaceparm trans
	cull none
       
	{
		map textures/sfx2/jumportal01.tga
		alphaFunc GE128
		depthWrite
		rgbGen identity
                  
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}
textures/sfx2/jumportal02
{
	
       	
	surfaceparm trans
	cull none
       
	{
		map textures/sfx2/jumportal02.tga
		alphaFunc GE128
		depthWrite
		rgbGen identity
                  
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}
textures/sfx2/scrolling_red
{
	qer_editorimage textures/base_light/jaildr1_3.tga
	surfaceparm nomarks
	q3map_lightimage textures/base_light/jaildr1_3.blend.tga
	q3map_surfacelight 1000
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/base_light/jaildr1_3.tga
		blendFunc GL_DST_COLOR GL_ZERO
		tcMod scroll 0 -.25
		rgbGen identity
	}
	{
		map textures/base_light/jaildr1_3.blend.tga
		rgbGen wave sin 0.5 0.5 1 1
		tcMod scroll 0 -.25
		blendfunc GL_ONE GL_ONE
	}
}

textures/sfx2/b_flame1
{
	qer_editorimage textures/sfx/b_flame1.tga
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nolightmap
	cull none
        {
		animMap 10 textures/sfx/b_flame1.tga textures/sfx/b_flame2.tga textures/sfx/b_flame3.tga textures/sfx/b_flame4.tga textures/sfx/b_flame5.tga textures/sfx/b_flame6.tga textures/sfx/b_flame7.tga textures/sfx/b_flame8.tga
		blendFunc GL_ONE GL_ONE
		rgbGen wave inverseSawtooth 0 1 0 10
		
	}	
	{
		animMap 10 textures/sfx/b_flame2.tga textures/sfx/b_flame3.tga textures/sfx/b_flame4.tga textures/sfx/b_flame5.tga textures/sfx/b_flame6.tga textures/sfx/b_flame7.tga textures/sfx/b_flame8.tga textures/sfx/b_flame1.tga
		blendFunc GL_ONE GL_ONE
		rgbGen wave sawtooth 0 1 0 10
	}	


	{
		map textures/sfx/b_flameball.tga
		blendFunc GL_ONE GL_ONE
		rgbGen wave sin .6 .2 0 .6	
	}

}