/// ============================================================================
/*
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
Please see the file "CONTRIBUTORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================

uniform sampler2D	u_diffusemap;
uniform sampler2D	u_bumpmap;
uniform sampler2D	u_specularmap;
uniform sampler2D	u_attenuationmap_xy;
uniform sampler2D	u_attenuationmap_z;
uniform vec3		u_view_origin;
uniform vec3		u_light_origin;
uniform vec3		u_light_color;
uniform float		u_bump_scale;
uniform float		u_specular_exponent;

varying vec3		var_vertex;
varying vec4		var_tex_diffuse_bump;
varying vec2		var_tex_specular;
varying vec4		var_tex_atten_xy_z;
varying mat3		var_mat_os2ts;

void	main()
{	
	// compute view direction in tangent space
	vec3 V = normalize(var_mat_os2ts * (u_view_origin - var_vertex));
	
	// compute light direction in tangent space
	vec3 L = normalize(var_mat_os2ts * (u_light_origin - var_vertex));
	
	// compute half angle in tangent space
	vec3 H = normalize(L + V);
	
	// compute normal in tangent space from bumpmap
	vec3 N = 2.0 * (texture2D(u_bumpmap, var_tex_diffuse_bump.pq).xyz - 0.5);
	N.z *= u_bump_scale;
	N = normalize(N);
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_diffusemap, var_tex_diffuse_bump.st);
	diffuse.rgb *= u_light_color * clamp(dot(N, L), 0.0, 1.0);
	
	// compute the specular term
	vec3 specular = texture2D(u_specularmap, var_tex_specular).rgb * u_light_color * pow(clamp(dot(N, H), 0.0, 1.0), u_specular_exponent);
	
	// compute attenuation
	vec3 attenuation_xy	= texture2DProj(u_attenuationmap_xy, vec3(var_tex_atten_xy_z.x, var_tex_atten_xy_z.y, var_tex_atten_xy_z.w)).rgb;
	vec3 attenuation_z	= texture2D(u_attenuationmap_z, vec2(var_tex_atten_xy_z.z, 0)).rgb;
					
	// compute final color
	gl_FragColor.rgba = diffuse;
	gl_FragColor.rgb += specular;
	gl_FragColor.rgb *= attenuation_xy;
	gl_FragColor.rgb *= attenuation_z;
}

