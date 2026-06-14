#version 450
layout(location=0) in vec2 vTex;
layout(location=1) in vec3 vWorldPos;
layout(location=2) in vec3 vNormal;
layout(location=3) in vec3 vTangent;
layout(location=4) in float vFlip;
layout(location=5) in vec4 vLightSpacePos;
layout(location=6) in vec3 flightPos;
layout(location=0) out vec4 fragColor;

layout(set=0,binding=0) uniform sampler2D diffuseMap;
layout(set=0,binding=1) uniform sampler2D normalMap;
layout(set=2,binding=0) uniform sampler2D shadowMap; // 或 sampler2D，根据你的 shadow sampler 类型



float computeShadow(vec4 lightSpacePos, vec3 normal, vec3 lightDir) {
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    vec2 uv = projCoords.xy * 0.5 + 0.5;
    float currentDepth = projCoords.z; // ZO: 已经在 [0,1]

    // outside light frustum -> no shadow
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0 || currentDepth > 1.0) {
        return 1.0;
    }

    // bias to reduce self-shadowing
    float bias = 0.005;

  

    // PCF 3x3
    ivec2 shadowTexSize = textureSize(shadowMap, 0);
    float texelSizeX = 1.0 / float(shadowTexSize.x);
    float texelSizeY = 1.0 / float(shadowTexSize.y);

    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(float(x) * texelSizeX, float(y) * texelSizeY);
            ivec2 fuv=ivec2((uv+offset) * shadowTexSize);
            float closestDepth = texelFetch(shadowMap, fuv,0).r;
            if (currentDepth - bias > closestDepth) {
                shadow += 1.0;
            }
        }
    }
    shadow /= 9;
    // 返回可见度（1 = lit, 0 = in shadow）
    

    float eps = 2; // 控制软化宽度（世界单位或相对值）
    float s = smoothstep(-eps, eps, 1-shadow); // 从0到1平滑
    return s;
    
   
}

void main() {
    // normal map
    vec3 n = texture(normalMap, vTex).rgb * 2.0 - 1.0;
    // build TBN
    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent);
    T = normalize(T - N * dot(N, T));
    if (length(T) < 1e-4) {
    // 选择一个与 N 不共线的任意向量作为切线基
    vec3 tmp = abs(N.x) > 0.9 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    T = normalize(tmp - N * dot(N, tmp));
    }
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    vec3 mappedN = normalize(TBN * n);

    // light dir
    vec3 lightDir = normalize(flightPos - vWorldPos);
    float diff = max(dot(mappedN, lightDir), 0.0);

    // shadow
    float shadow =computeShadow(vLightSpacePos,mappedN,lightDir); 
    
    vec3 diffuse = texture(diffuseMap, vTex).rgb;
    vec3 albedo=0.3*diffuse;
    vec3 color = ( diff*diffuse+albedo)*shadow; // simple lambert + shadow
    fragColor = vec4(vec3(shadow), 1.0);


  

}