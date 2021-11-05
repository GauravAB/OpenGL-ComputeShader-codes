#version 460 core

layout(local_size_x = 512) in;

layout(binding = 0 , rgba32f) uniform image2D img_input;
layout(binding = 1 , rgba32f) uniform image2D img_output;

shared vec4 scanline[512];

void main(void)
{
	ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
	scanline[pos.x] = imageLoad(img_input, pos);
	barrier();

	imageStore(img_output, pos.yx ,scanline[min(pos.x + 1 , 511)] - scanline[max(pos.x - 1,0)]);
}







