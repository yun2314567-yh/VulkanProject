#version 450
layout(location=0) in vec3 mPos;
layout(location=1) in vec2 tex;
layout(location=2) in vec3 normal;
layout(location=3) in vec3 T;
layout(location=4) in float flip;

layout(location=0) out vec2 vTex;
layout(location=1) out vec3 vWorldPos;
layout(location=2) out vec3 vNormal;
layout(location=3) out vec3 vTangent;
layout(location=4) out float vFlip;
layout(location=5) out vec4 vLightSpacePos;
layout(location=6) out vec3 flightPos;

layout(set=1,binding=0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 lightProj;
    mat4 lightView;
    vec3 camPos;
    vec3 lightPos;
} ubo;

void main() {
    vec4 worldPos = ubo.model * vec4(mPos, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = mat3(transpose(inverse(ubo.model))) * normal;
    vTangent = mat3(ubo.model) * T; // 若 T 是局部空间切线
    vTex = tex;
    vFlip = flip;
    vLightSpacePos = ubo.lightProj * ubo.lightView * worldPos;
    flightPos=ubo.lightPos;
    gl_Position =  ubo.proj * ubo.view * worldPos;
}