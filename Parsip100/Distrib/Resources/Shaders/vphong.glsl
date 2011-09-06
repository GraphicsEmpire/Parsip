varying vec3 N;
varying vec3 L;

void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	
	
	vec4 v        = gl_ModelViewMatrix * gl_Vertex;
	vec4 lightPos = gl_LightSource[0].position;
	
	L = normalize(lightPos.xyz - v.xyz);
	N = normalize(gl_NormalMatrix * gl_Normal);
}