#version 450 core

layout (location = 0)in vec4 vPosition;

uniform mat4 u_model_matrix;
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
out float intensity;

void main(void)
{
	intensity = vPosition.w;

	gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(vPosition.xyz,1.0);
}


