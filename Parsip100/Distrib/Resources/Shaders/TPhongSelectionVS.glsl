#define VERSION 100

uniform float time;
varying vec3 N;
varying vec3 V;

void main(void)
{
	float t = sin(time * 0.001);
	N = normalize(gl_NormalMatrix * gl_Normal);
	V = vec3(gl_ModelViewMatrix * gl_Vertex);

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex + t*vec4(N, 1.0);	
	//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;	
	gl_FrontColor = gl_Color;
	
	
	//Pass materials for this vertex as texture values
	//gl_TexCoord[0] = gl_MultiTexCoord0;
	//gl_TexCoord[1] = gl_MultiTexCoord1;
	//gl_TexCoord[2] = gl_MultiTexCoord2;
	//gl_TexCoord[3] = gl_MultiTexCoord3;	
	
}