varying vec3 N;
varying vec3 L;
uniform sampler2D TexMap;

void main(void)
{
	vec4 cr = gl_FrontMaterial.diffuse;
	vec4 cl = gl_LightSource[0].diffuse;
	vec3 Normal = normalize(N);
	vec3 Light = normalize(L);
	
	vec4 diffuse = max(dot(Normal, Light), 0.0) * cr * cl;
	vec4 texColor = texture2D(TexMap, gl_TexCoord[0].st);
	
	gl_FragColor  = diffuse * texColor;	
}