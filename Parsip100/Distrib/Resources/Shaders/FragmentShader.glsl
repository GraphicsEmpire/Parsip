uniform float time;
void main(void)
{
	float d = length(gl_FragCoord.xy);
	gl_FragColor.r = 0.5*(1.0 + sin(0.001*time))*gl_FragCoord.x/d;
	gl_FragColor.g = 0.5*(1.0 + cos(0.001*time))*gl_FragCoord.y/d;
	gl_FragColor.b = gl_FragCoord.z;
	gl_FragColor.a = 0.3;
}