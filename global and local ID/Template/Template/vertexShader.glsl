#version 460 core

layout (location = 0)in vec4 vPosition;
layout(location = 3)in vec2 vTexCoords;

uniform mat4 u_model_matrix;
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
out vec2 fTexCoords;

void main(void)
{
	fTexCoords = vTexCoords;
	gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;
}

