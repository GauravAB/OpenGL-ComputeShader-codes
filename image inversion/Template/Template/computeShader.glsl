#version 460 core

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0 , rgba32f) uniform image2D img_input;
layout(binding = 1 , rgba32f) uniform image2D img_output;


void main(void)
{
	vec4 texel;

	ivec2 p = ivec2(gl_GlobalInvocationID.xy);
	texel = imageLoad(img_input, p);
	
	texel = vec4(1.0) - texel;

	imageStore(img_output, p, texel);
}
