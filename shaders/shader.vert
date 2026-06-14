#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location=0) out vec3 Normal;
layout(location=1) out vec2 texCoords;
layout(location=2) out vec3 fragPos;
layout(location=3) out vec3 camPos;


layout(location=0) in vec3 mPos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 tex;

layout(set=0,binding=0) uniform UBO
{
       mat4 model;
	   mat4 view;
	   mat4 proj;
	   vec3 camPos;
} ubo;



void main()
{
   
    
   
   gl_Position =ubo.proj*ubo.view*ubo.model*vec4(mPos, 1.0);
   fragPos=mPos;
   Normal = normal;
   texCoords=tex;
   camPos=ubo.camPos;
}