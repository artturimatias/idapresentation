
varying vec2 vTexCoord;


void main(void)
{
   vTexCoord = gl_TextureMatrix[0] * gl_MultiTexCoord0;
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
   
