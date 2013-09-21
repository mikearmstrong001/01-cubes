$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2

/*
 * Copyright 2011-2013 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include "../common/common.sh"
uniform float u_flash;
SAMPLER2D(u_tex, 0);


void main()
{
	vec4 tex = texture2D(u_tex, v_texcoord2);
	float fogblend = saturate( dot( v_texcoord1, v_texcoord1 )/100000.f );
	vec3 n = normalize( cross( dFdx( v_texcoord0 ), dFdy( v_texcoord0 ) ) );
	vec4 diffuse = (v_color0 * tex);// * saturate(0.75 + 0.25 * dot( n, normalize(vec3( 0, -0.5, -1.0 )) ));
	vec3 litDiffuse = diffuse.xyz * saturate(0.75 + 0.25 * dot( n, normalize(vec3( 0, -0.5, -1.0 )) ));
	vec3 fogColour = vec3( 0.5, 0.6, 0.7 );
	gl_FragColor.xyz = mix( litDiffuse.xyz + u_flash, fogColour, fogblend );
	gl_FragColor.w = diffuse.w;
}
