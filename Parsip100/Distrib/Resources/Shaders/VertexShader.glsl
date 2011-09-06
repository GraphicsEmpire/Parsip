uniform float time;

void main(void)
{
	float twist = 0.4*sin(0.001*time);
	
	vec4 a = gl_ModelViewProjectionMatrix * gl_Vertex;
	float d = length(a.xz);
	
	
	gl_Position.x = cos(d * twist)*a.x - sin(d * twist)*a.z;
	gl_Position.z = cos(d * twist)*a.z + sin(d * twist)*a.x;
	gl_Position.y = a.y;
	gl_Position.w = a.w;

	gl_FrontColor = gl_Color;
}