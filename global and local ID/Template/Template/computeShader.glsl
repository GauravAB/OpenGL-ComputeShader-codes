#version 460 core

layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f) uniform image2D img_output;

void main(void)
{
	imageStore(img_output,
		ivec2(gl_GlobalInvocationID.xy),
		vec4(vec2(gl_LocalInvocationID.xy) / vec2(gl_WorkGroupSize.xy), 0.0, 0.0));
}


















