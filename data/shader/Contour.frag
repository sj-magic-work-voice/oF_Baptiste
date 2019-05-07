/************************************************************
ビルトイン関数(一部)
	http://qiita.com/edo_m18/items/71f6064f3355be7e4f45
************************************************************/
#version 120
#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable

#define PI (3.1415)

uniform float Alpha = 0.7;

uniform float DispGain_Sin;
uniform int dt;

#define MaxNumFreqs (7)
uniform int Low_NumFreqs = 2;
uniform float Low_f[MaxNumFreqs];
uniform float Low_Amp[MaxNumFreqs];
uniform float Low_Phase[MaxNumFreqs];

uniform int Mid_NumFreqs = 2;
uniform float Mid_f[MaxNumFreqs];
uniform float Mid_Amp[MaxNumFreqs];
uniform float Mid_Phase[MaxNumFreqs];

uniform int High_NumFreqs = 3;
uniform float High_f[MaxNumFreqs];
uniform float High_Amp[MaxNumFreqs];
uniform float High_Phase[MaxNumFreqs];


float map(float x, float x_from, float x_to, float y_from, float y_to, bool b_clamp);

/************************************************************
************************************************************/

/******************************
******************************/
void main() {
	/********************
	********************/
	vec4 color;
	vec2 pos = gl_FragCoord.xy;
	
	/********************
	********************/
	float z;
	
	z = 0;
	for(int i = 0; i < Low_NumFreqs; i++){
		float _z = Low_Amp[i] * cos( 2 * PI * Low_f[i] * ((pos.y * dt + Low_Phase[i]) / 1e+6) );
		_z = map(_z, 0.0, DispGain_Sin, 0.0, 1.0, true);
		
		z = max(z, _z);
	}
	color.b = z;
	
	/* */
	z = 0;
	for(int i = 0; i < Mid_NumFreqs; i++){
		float _z = Mid_Amp[i] * cos( 2 * PI * Mid_f[i] * ((pos.y * dt + Mid_Phase[i]) / 1e+6) );
		_z = map(_z, 0.0, DispGain_Sin, 0.0, 1.0, true);
		
		z = max(z, _z);
	}
	color.r = z;
	
	/* */
	z = 0;
	for(int i = 0; i < High_NumFreqs; i++){
		float _z = High_Amp[i] * cos( 2 * PI * High_f[i] * ((pos.y * dt + High_Phase[i]) / 1e+6) );
		_z = map(_z, 0.0, DispGain_Sin, 0.0, 1.0, true);
		
		z = max(z, _z);
	}
	color.g = z;
	
	
	/********************
	********************/
	if( (color.r == 0) && (color.g == 0) && (color.b == 0) )	color.a = 0;
	else														color.a = Alpha;
	
	/********************
	********************/
	gl_FragColor = color;
}

/******************************
******************************/
float map(float x, float x_from, float x_to, float y_from, float y_to, bool b_clamp)
{
	if(x_to == x_from) return 0;
	
	if(x_to < x_from){
		float temp = x_from;
		x_from = x_to;
		x_to = temp;
		
		temp = y_from;
		y_from = y_to;
		y_to = temp;
	}
	
	float tan = (y_to - y_from) / (x_to - x_from);
	float y = tan * (x - x_from) + y_from;
	
	if(b_clamp){
		if(y < y_from)	y = y_from;
		if(y_to < y)	y = y_to;
	}
	
	return y;
}

