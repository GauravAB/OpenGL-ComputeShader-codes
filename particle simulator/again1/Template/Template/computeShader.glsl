#version 460 core

//uniform block containing positions and masses of the attractors
layout (std140 , binding = 0)	uniform attractor_block
{
	vec4 attractor[64];	//xyz = position , w = mass
};

//process particles in blocks of 128
layout(local_size_x = 128)in;

//Buffers containing the positions and velocities of the particles
layout(rgba32f, binding = 0) uniform imageBuffer velocity_buffer;
layout(rgba32f, binding = 1) uniform imageBuffer position_buffer;

//Delta time
uniform float dt;

void main(void)
{
		//read the current position and velocity from the buffers
	vec4 vel = imageLoad(velocity_buffer, int(gl_GlobalInvocationID.x));
	vec4 pos = imageLoad(position_buffer, int(gl_GlobalInvocationID.x));

	int i;

	//update position using current velocity * time
	pos.xyz += vel.xyz  * dt;
	//update 'life' of particle in w component
	pos.w -= 0.0001 * dt;

	//for each attractor
	for (int i = 0; i < 4; i++)
	{
		//calculate force and update velocity
		vec3 dist = (attractor[i].xyz - pos.xyz);
		vel.xyz += dt * dt * attractor[i].w * normalize(dist) / (dot(dist, dist) + 10.0);
	}
	
	//if the particle expires , reset it
	if (pos.w <= 0.0)
	{

		pos.xyz = -pos.xyz * 0.01;
		vel.xyz *= 0.01;
		pos.w += 1.0f;
	}

	//store the new position and velocity back to the buffers
	imageStore(position_buffer, int(gl_GlobalInvocationID.x), pos);
	imageStore(velocity_buffer, int(gl_GlobalInvocationID.x), vel);
}















