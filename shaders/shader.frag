#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) out vec4 fragColor;

layout(location=0) in vec3 Normal;
layout(location=1) in vec2 texCoords;
layout(location=2) in vec3 fragPos;

layout(location=3) in vec3 camPos;


layout(set=1,Binding=0) uniform sampler2D diffuseMap;


void main()
{
       vec3 viewDir=normalize(camPos-fragPos);

       float diff=max(dot(Normal,viewDir),0.0);
       vec3 diffuse=texture(diffuseMap,texCoords).rgb;
       vec3 diffuseColor=diff * diffuse;
       
       fragColor=vec4(vec3(1,0,0), 1.0) ;
}