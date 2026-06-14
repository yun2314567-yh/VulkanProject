#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) out vec4 fragColor;


layout(location=0) in vec2 texCoords;
layout(location=1) in vec3 fragPos;

layout(location=2) in vec3 camPos;

layout(location=3) in vec3 Normal;

layout(location=4) in vec3 t;
layout(location=5) in float f;
layout(location=6) in vec4 lightSpacePos;
layout(location=7) in vec3 lightPos;

layout(set=0,Binding=0) uniform sampler2D diffuseMap;
layout(set=0,Binding=1) uniform sampler2D normalMap;

layout(set=2,binding=0) uniform sampler2DShadow shadowMap;

float computeShadow()
{
     vec3 fragPos=lightSpacePos.xyz/lightSpacePos.w;

     if(fragPos.x<0.0||fragPos.x>1.
     ||fragPos.y<0.0||fragPos.y>1.
     ||fragPos.z<0.0||fragPos.z>1.)
     return 0.0;

     float shadow=0.;
     shadow=texture(shadowMap,fragPos);

     return shadow;
}


void main()
{
       
       vec3 viewDir=normalize(camPos-fragPos);
       vec3 lightDir=normalize(lightPos-fragPos);

       vec3 normal=texture(normalMap,texCoords).rgb;
       normal=normal*2-1.;
       normal=normalize(normal);

       float s=computeShadow();

       vec3 B=cross(Normal,t)*f;

       mat3 tbn=mat3(t,B,Normal);

       normal=tbn*normal;
       float diff=max(dot(normal,lightDir),0.0);
       vec3 diffuse=texture(diffuseMap,texCoords).rgb;
       vec3 diffuseColor=diff * diffuse;
       
       vec3 finalCol=s*0.3*diffuse+diffuseColor;

       fragColor=vec4(finalCol, 1.0) ;
}