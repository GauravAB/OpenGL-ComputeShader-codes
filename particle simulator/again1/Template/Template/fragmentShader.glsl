#version 460 core

out vec4 fColor;
in float intensity;

void main(void)
{

	fColor = mix(vec4(0.0f,0.2f,1.0f,1.0f),vec4(0.2f,0.05f,0.0f,1.0f),intensity);
	
}



