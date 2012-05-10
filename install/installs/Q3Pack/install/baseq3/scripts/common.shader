textures/common/areaportal
{
	qer_trans 0.50
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm structural
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm areaportal
}

textures/common/caulk
{
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nomarks
}

textures/common/clip
{
	qer_trans 0.40
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm noimpact
	surfaceparm playerclip
}

textures/common/clusterportal
{
	qer_trans 0.50
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm clusterportal
}

textures/common/cushion
{
	qer_nocarve
	qer_trans 0.50
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nodamage
}

textures/common/donotenter
{
	qer_trans 0.50
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm donotenter
}

// never used
//
//textures/common/energypad
//{
//	qer_editorimage textures/sfx/bluegoal.tga
//	surfaceparm nolightmap
//	cull twosided
//	{
//		map textures//bluegoal.tga
//		blendFunc GL_ONE GL_SRC_ALPHA
//		tcGen environment
//		tcMod turb 0 0.25 0 0.05
//	}
//}

textures/common/full_clip	// silly shader, use weapclip instead
{
	qer_trans 0.40
	surfaceparm nodraw
	surfaceparm playerclip
}

textures/common/hint		// should NOT use surfaceparm hint... strange but true
{
	qer_nocarve
	qer_trans 0.30
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm structural
	surfaceparm trans
	surfaceparm noimpact
	surfaceparm hint	// ydnar: yes it should.
}

// Obsidian: Local hint works like normal hint but doesn't create portals beyond local structural brushes. Experimental, use with caution.
textures/common/hintlocal
{
	qer_nocarve
	qer_trans 0.30
	qer_editorImage textures/common/hintlocal.tga
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm structural
	surfaceparm trans
	surfaceparm noimpact
}

textures/common/invisible	// solid, transparent polygons, casts shadows
{
	surfaceparm nolightmap
	{
		map textures/common/invisible.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
}

textures/common/mirror1
{
	qer_editorimage textures/common/mirror1.tga
	surfaceparm nolightmap
	portal
	{
		map textures/common/mirror1.tga
		blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		depthWrite
	}
}

textures/common/mirror2
{
	qer_editorimage textures/common/qer_mirror.tga
	surfaceparm nolightmap
	portal
	{
		map textures/common/mirror1.tga
		blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		depthWrite
	}
	{
		map textures/sfx/mirror.tga
		blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
	}
}

textures/common/missileclip
{
	qer_trans 0.40
	surfaceparm nodamage
	surfaceparm nomarks
	surfaceparm nodraw
	//surfaceparm nonsolid
	surfaceparm playerclip
	surfaceparm trans
}

textures/common/nodraw
{
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm nomarks
}

textures/common/nodrawnonsolid
{
	surfaceparm nonsolid
	surfaceparm nolightmap
	surfaceparm nodraw
}

textures/common/nodrop
{
	qer_nocarve
	qer_trans 0.5
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nodrop
}

textures/common/noimpact
{
	qer_editorimage textures/common/nolightmap.tga
	surfaceparm noimpact
}

textures/common/nolightmap
{
	surfaceparm nolightmap
}

textures/common/origin
{
	qer_nocarve
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm origin
}

textures/common/portal
{
	qer_editorimage textures/common/qer_portal.tga
	surfaceparm nolightmap
	portal
	{
		map textures/common/mirror1.tga
		tcMod turb 0 0.25 0 0.05
		blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		depthWrite
	}
}

textures/common/slick
{
	qer_trans 0.50
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm trans
	surfaceparm slick
}

textures/common/terrain
{
	q3map_terrain
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nomarks
}

textures/common/terrain2
{
	q3map_terrain
	qer_editorimage textures/common/terrain.tga
	surfaceparm dust
	surfaceparm nodraw
	surfaceparm nomarks
	surfaceparm nolightmap
}

textures/common/timportal
{
	qer_editorimage textures/common/qer_portal.tga
	portal
	surfaceparm nolightmap
	{
		map textures/common/portal.tga
		tcMod turb 0 0.25 0 0.05
		blendFunc GL_ONE GL_SRC_ALPHA
		depthWrite
	}
}

textures/common/trigger
{
	qer_trans 0.50
	qer_nocarve
	surfaceparm nodraw
}

textures/common/weapclip
{
	qer_trans 0.40
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm trans
	surfaceparm nomarks
}

textures/common/metalclip
{
	qer_trans 0.40
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm noimpact
	surfaceparm playerclip
	surfaceparm metalsteps
}

textures/common/botclip
{
	qer_trans 0.40
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm noimpact
	surfaceparm botclip
}

// ydnar q3map lightgrid bounds
//
// the min/max bounds of brushes with this shader in a map
// will define the bounds of the map's lightgrid (model lighting)
// note: make it as small as possible around player space
// to minimize bsp size and compile time

textures/common/lightgrid
{
	qer_trans 0.5
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm detail
	surfaceparm nomarks
	surfaceparm trans
	surfaceparm lightgrid
}




//******************************************************************************
// Obsidian 2007-08-24 (update 2010-12-03)
// Q3Map2 "common" shaders
//******************************************************************************


// ydnar: antiportal works like hint, but supresses portals
// add this to your common.shader file
textures/common/antiportal
{
	qer_nocarve
	qer_trans 0.30
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm structural
	surfaceparm trans
	surfaceparm noimpact
	surfaceparm antiportal
}


// ydnar: skip works like quake 2 hint: it doesn't generate bsp splits
// use on sides of hint brushes where you don't want bsp splits or portals
// add this to your common.shader file
textures/common/skip
{
	qer_nocarve
	qer_trans 0.30
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm structural
	surfaceparm trans
	surfaceparm noimpact
	surfaceparm skip
}

// Obsidian: same as skip but name changed to allow Radiant to filter both hint and skip
textures/common/hintskip
{
	qer_nocarve
	qer_trans 0.30
	qer_editorImage textures/common/skip.tga
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm structural
	surfaceparm trans
	surfaceparm noimpact
	surfaceparm skip
}


// Obsidian: 2 usages for watercaulk depending on water brush complexity
// SIMPLE WATER BRUSHES - use watercaulk on faces between water brushes
// COMPLEX WATER BRUSHES - overlap complex water brushes with watercaulk.
//		Water shader should be nodraw, nonsolid, trans, *sans-water*
textures/common/watercaulk
{
	qer_trans 0.5
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm water
}

textures/common/slimecaulk
{
	qer_trans 0.5
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm slime
}

textures/common/lavacaulk
{
	qer_trans 0.5
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm lava
}

// alpha fade shaders
// (c) 2004 randy reddig
// http://www.shaderlab.com
// distribution, in part or in whole, in any medium, permitted

textures/common/alpha_100
{
	qer_trans 0.5
	q3map_alphaMod volume
	q3map_alphaMod scale 1.0
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
}

textures/common/alpha_75
{
	qer_trans 0.5
	q3map_alphaMod volume
	q3map_alphaMod scale 0.75
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
}

textures/common/alpha_50
{
	qer_trans 0.5
	q3map_alphaMod volume
	q3map_alphaMod scale 0.5
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
}

textures/common/alpha_25
{
	qer_trans 0.5
	q3map_alphaMod volume
	q3map_alphaMod scale 0.25
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
}

textures/common/alpha_0
{
	qer_trans 0.5
	q3map_alphaMod volume
	q3map_alphaMod scale 0
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
}