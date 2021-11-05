#version 460 core

layout(local_size_x = 32, local_size_y = 32) in;
layout (binding = 0 , rgba32f) uniform image2D img_output1;
layout (binding = 1, rgba32f) uniform image2D img_output2;
layout (binding = 2, rgba32f) uniform image2D img_output3;
layout (binding = 3, rgba32f) uniform image2D img_output4;


uniform float uTimer;


float hash(float n)  
{
	//p = 50.0*fract(p*0.3183099 + vec2(0.71, 0.113));
	//return -1.0 + 2.0*fract(p.x*p.y*(p.x + p.y));

	return fract(sin(n) * 999.0);
}

// return value noise (in x) and its derivatives (in yz)
float noised(in vec2 p)
{
	vec2 i = floor(p);
	vec2 f = fract(p);

	// quintic interpolation
	vec2 u = f * f*f*(f*(f*6.0 - 15.0) + 10.0);
	vec2 du = 30.0*f*f*(f*(f - 2.0) + 1.0);


	//	float va = hash(i + vec2(0.0, 0.0));
	//float vb = hash(i + vec2(1.0, 0.0));
	//float vc = hash(i + vec2(0.0, 1.0));
	//float vd = hash(i + vec2(1.0, 1.0));

	//	float k0 = va;
	//	float k1 = vb - va;
	//float k2 = vc - va;
	//float k4 = va - vb - vc + vd;

	//return vec3(va + (vb - va)*u.x + (vc - va)*u.y + (va - vb - vc + vd)*u.x*u.y, // value
	//du*(u.yx*(va - vb - vc + vd) + vec2(vb, vc) - va));     // derivative                

	f = f * f*(3.0 - 2.0*f);
	float n = i.x + i.y*57.0;

	return (mix(
		mix(hash(n), hash(n + 1.0), f.x),
		mix(hash(n + 57.0), hash(n + 58.0), f.x),
		f.y
	));
}

//noise function

vec3 mod289(vec3 x) { return x - floor(x * (1. / 289.)) * 289.; }
vec4 mod289(vec4 x) { return x - floor(x * (1. / 289.)) * 289.; }
vec4 permute(vec4 x) { return mod289(((x*34.) + 1.)*x); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - .85373472095314 * r; }
vec3 fade(vec3 t) { return t * t*t*(t*(t*6. - 15.) + 10.); }

float noise(vec3 P) {

	vec3 i0 = mod289(floor(P)), i1 = mod289(i0 + vec3(1.)),
		f0 = fract(P), f1 = f0 - vec3(1.), f = fade(f0);
	vec4 ix = vec4(i0.x, i1.x, i0.x, i1.x), iy = vec4(i0.yy, i1.yy),
		iz0 = i0.zzzz, iz1 = i1.zzzz,
		ixy = permute(permute(ix) + iy), ixy0 = permute(ixy + iz0), ixy1 = permute(ixy + iz1),
		gx0 = ixy0 * (1. / 7.), gy0 = fract(floor(gx0) * (1. / 7.)) - .5,
		gx1 = ixy1 * (1. / 7.), gy1 = fract(floor(gx1) * (1. / 7.)) - .5;
	gx0 = fract(gx0); gx1 = fract(gx1);
	vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0), sz0 = step(gz0, vec4(0.)),
		gz1 = vec4(0.5) - abs(gx1) - abs(gy1), sz1 = step(gz1, vec4(0.));
	gx0 -= sz0 * (step(0., gx0) - .5); gy0 -= sz0 * (step(0., gy0) - .5);
	gx1 -= sz1 * (step(0., gx1) - .5); gy1 -= sz1 * (step(0., gy1) - .5);
	vec3 g0 = vec3(gx0.x, gy0.x, gz0.x), g1 = vec3(gx0.y, gy0.y, gz0.y),
		g2 = vec3(gx0.z, gy0.z, gz0.z), g3 = vec3(gx0.w, gy0.w, gz0.w),
		g4 = vec3(gx1.x, gy1.x, gz1.x), g5 = vec3(gx1.y, gy1.y, gz1.y),
		g6 = vec3(gx1.z, gy1.z, gz1.z), g7 = vec3(gx1.w, gy1.w, gz1.w);
	vec4 norm0 = taylorInvSqrt(vec4(dot(g0, g0), dot(g2, g2), dot(g1, g1), dot(g3, g3))),
		norm1 = taylorInvSqrt(vec4(dot(g4, g4), dot(g6, g6), dot(g5, g5), dot(g7, g7)));
	g0 *= norm0.x; g2 *= norm0.y; g1 *= norm0.z; g3 *= norm0.w;
	g4 *= norm1.x; g6 *= norm1.y; g5 *= norm1.z; g7 *= norm1.w;
	vec4 nz = mix(vec4(dot(g0, vec3(f0.x, f0.y, f0.z)), dot(g1, vec3(f1.x, f0.y, f0.z)),
		dot(g2, vec3(f0.x, f1.y, f0.z)), dot(g3, vec3(f1.x, f1.y, f0.z))),
		vec4(dot(g4, vec3(f0.x, f0.y, f1.z)), dot(g5, vec3(f1.x, f0.y, f1.z)),
			dot(g6, vec3(f0.x, f1.y, f1.z)), dot(g7, vec3(f1.x, f1.y, f1.z))), f.z);
	return 2.2 * mix(mix(nz.x, nz.z, f.y), mix(nz.y, nz.w, f.y), f.x);
}

mat2 m = mat2(0.8, 0.6, -0.6, 0.8);

//fractional brownian motion using value noise
float fbm(vec2 p)
{
	float f = 0.0;

	f += 0.5000 * noised(p); p *= m * 2.01;
	f += 0.2500 * noised(p); p *= m * 2.02;
	f += 0.1250 * noised(p); p *= m * 2.03;
	f += 0.0625 * noised(p); p *= m * 2.04;

	f /= 0.9375;

	return f;
}

float turbulence(vec3 P)
{												// Turbulence is a fractal sum of abs(noise).

	float f = 0., s = 1.;							// The domain is rotated after every iteration

	for (int i = 0; i < 9; i++)
	{												// To avoid any visible grid artifacts.
		f += abs(noise(s * P)) / s;
		s *= 2.;
		P = vec3(.866 * P.x + .5 * P.z, P.y + 100., -.5 * P.x + .866 * P.z);
	}

	return f;
}

void main(void)
{
	ivec2 pos = ivec2(gl_GlobalInvocationID.xy);

	float color = noise(vec3(pos , 1.0));
	imageStore(img_output1, pos, vec4(vec3(noise( 0.02*vec3(, uTimer))),1.0));
	
	/*
	color = noised(pos + pow(2, 8));
	imageStore(img_output2, pos, vec4(color));

	color = noised(pos + pow(2, 4));
	imageStore(img_output3, pos, vec4(color));

	color = noised(pos);
	imageStore(img_output4, pos, vec4(color));
	
	*/

}




















