#version 460 core
out vec4 fColor;
in vec2 fTexCoords;
uniform sampler2D sampleTexture;
uniform vec2 uResolution;


float noise(in vec2 p)
{
	vec4 textureColor = texture(sampleTexture, fTexCoords*0.5);
	float f;

	return f;
}


void main(void)
{ 
	vec2 uv = gl_FragCoord.xy / uResolution;
	vec2 pos = (uv + 1) / 2;
	//pos.x *= uResolution.x / uResolution.y;
	//float f = fbm(0.2*pos,textureColor);

	vec4 textureColor = texture(sampleTexture, pos*0.1);

	vec3 color = vec3(textureColor.y);

	fColor = vec4(color,1.0);
}
