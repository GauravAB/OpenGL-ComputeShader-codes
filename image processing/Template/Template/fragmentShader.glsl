#version 460 core
out vec4 fColor;
in vec2 fTexCoords;
uniform sampler2D sampleTexture;

void main(void)
{
	vec4 textureColor = texture(sampleTexture, fTexCoords);

	fColor = vec4(vec3(textureColor),1.0);
}



