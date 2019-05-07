#version 120
#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DRect texture0;
uniform sampler2DRect texture1;
uniform sampler2DRect texture2;


void main(){
	vec2 tex_pos = gl_TexCoord[0].xy;
	
	vec4 color0 = texture2DRect(texture0, tex_pos);
	vec4 color1 = texture2DRect(texture1, tex_pos);
	vec4 color2 = texture2DRect(texture2, tex_pos);
	
	/********************
	color0 - 2 : whiteで描画している前提
	********************/
	vec4 color;
	color.a = max(max(color0.a, color1.a), color2.a);
	
	color.r = color0.r;
	color.g = color1.g;
	color.b = color2.b;
	
	//Output of the shader
	gl_FragColor = color;
}



/*
void main(){
	
	
	
	vec2 pos = gl_TexCoord[0].xy;
	vec4 color0 = texture2DRect(texture0, pos);
	vec4 color1 = texture2DRect(texture1, pos);
	//Compute resulted color
	vec4 color;
	color.rgb = color0.rgb;
	color.a = color1.r;

	//Output of the shader
	gl_FragColor = color;
}



uniform vec2 u_resolution;

void main(){
    vec2 st = gl_FragCoord.xy/u_resolution.xy;
    vec3 color = vec3(0.0);

    // Each result will return 1.0 (white) or 0.0 (black).
    float left = step(0.1,st.x);   // Similar to ( X greater than 0.1 )
    float bottom = step(0.1,st.y); // Similar to ( Y greater than 0.1 )

    // The multiplication of left*bottom will be similar to the logical AND.
    color = vec3( left * bottom );

    gl_FragColor = vec4(color,1.0);
}


*/
