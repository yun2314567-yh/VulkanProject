#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex
{
	vec4 gl_Position;
};




layout(location=0) in vec3 mPos;



layout(set=1,binding=0) uniform UBO
{
       mat4 model;
	   mat4 view;
	   mat4 proj;	   
	   mat4 lightProj;
	   mat4 lightView;
	   vec3 camPos;
	   vec3 lightPos;
} ubo;



void main()
{
   
    
   
   gl_Position =ubo.lightProj*ubo.lightView*ubo.model*vec4(mPos, 1.0);
   
}