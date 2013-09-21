$input a_position, a_texcoord0, a_color0
$output v_color0, v_texcoord0, v_texcoord1, v_texcoord2

/*
 * Copyright 2011-2013 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include "../common/common.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	v_texcoord0 = mul(u_model[0], vec4(a_position, 1.0) ).xyz;
	v_texcoord1 = mul(u_modelView, vec4(a_position, 1.0) ).xyz;
	v_texcoord2 = a_texcoord0;
	v_color0 = a_color0;
}
