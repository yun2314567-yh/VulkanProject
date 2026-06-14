#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex
{
	vec4 gl_Position;
};


layout(location=0) out vec2 texCoords;
layout(location=1) out vec3 fragPos;
layout(location=2) out vec3 camPos;
layout(location=3) out vec3 Normal;
layout(location=4) out vec3 t;
layout(location=5) out float f;
layout(location=6) out vec4 lightSpacePos;
layout(location=7) out vec3 lightPos;

layout(location=0) in vec3 mPos;

layout(location=1) in vec2 tex;
layout(location=2) in vec3 normal;
layout(location=3) in vec3 T;
layout(location=4) in float flip;

layout(set=1,binding=0) uniform UBO
{
       mat4 model;
	   mat4 view;
	   mat4 proj;
	   vec3 camPos;
	   mat4 lightProj;
	   mat4 lightView;
	   vec3 lightPos;
} ubo;



void main()
{
   
    
   
   gl_Position =ubo.proj*ubo.view*ubo.model*vec4(mPos, 1.0);
   fragPos=mPos;
   lightSpacePos=ubo.lightProj*ubo.lightView*vec4(mPos, 1.0);
   Normal=normal;
   t=T;
   texCoords=tex;
   camPos=ubo.camPos;
   lightPos=ubo.lightPos;
}