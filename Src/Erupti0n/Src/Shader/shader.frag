#version 450
#extension GL_ARB_separate_shader_objects : enable

#define R vec2(780.0,557.0);
#define V4 vec4
#define V3 vec3
#define V2 vec2
#define t pc.time

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 uv;
layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(push_constant) uniform pushConstant
{
    float time;
}pc;

void main()
{
    V2 U =( uv *2.0) -1.0;
    U.y *= -1.;
    U.x *= 1920./1080.;

	V2 uvv = uv * V2(1,-1);
	
    outColor.xyz = texture(texSampler,uvv).rgb;
	//outColor.xyz = vec3(uv,0.);
}