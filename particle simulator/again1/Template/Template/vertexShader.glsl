#version 460 core

layout(location = 0)in vec4 vert;
uniform mat4 u_model_matrix;
uniform mat4 u_projection_matrix;
uniform mat4 u_view_matrix;

out float intensity;

void main(void)
{
	intensity = vert.w;
	gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(vert.xyz, 1.0);
}
