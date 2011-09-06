#define VERSION 100

varying vec3 N;
varying vec3 V;

void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;	
	gl_FrontColor = gl_Color;
	N = normalize(gl_NormalMatrix * gl_Normal);
	V = vec3(gl_ModelViewMatrix * gl_Vertex);
	
	
	//Pass materials for this vertex as texture values
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_TexCoord[2] = gl_MultiTexCoord2;
	gl_TexCoord[3] = gl_MultiTexCoord3;	
	
}