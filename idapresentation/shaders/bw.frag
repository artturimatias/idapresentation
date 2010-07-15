uniform sampler2D myTexture;

varying vec2 vTexCoord;

uniform float Br;

void main(void)

{
	vec4 c = texture2D(myTexture, vTexCoord);
	float gray =  (0.30*c.r + 0.59*c.g + 0.11*c.b)*Br;
	gl_FragColor = vec4(gray,c.r,gray,c.a);
	
 //  gl_FragColor = texture2D(myTexture, vTexCoord).bgra;
}
