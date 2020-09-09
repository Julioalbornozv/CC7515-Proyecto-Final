#version 330 core
#define PI 3.1415926538


layout (location = 0) in vec2 aPos;

uniform vec2 heightmap_size;
uniform float dp;
uniform mat4 trans;
uniform sampler2D heightmapA;
uniform sampler2D heightmapB;
uniform sampler2D heightmapC;

out vec3 pos;

void main(){
    vec2 texPos = vec2((aPos + vec2(1.0f)) / 2.0f);
    
    vec4 vA = texture(heightmapA, texPos);
    vec4 vB = texture(heightmapB, texPos);
    vec4 vC = texture(heightmapC, texPos);
    
    // We compute the weights of each frame
    
    // Paper Weights
    
    //float wA = cos((PI/3.0f)*(dp + 2));
    //float wB = cos((PI/3.0f)*(dp + 1));
    //float wC = cos((PI/3.0f)*(dp));
    
    // Custom Weights
    
    float wA = sin((PI/3.0f)*(dp+2));
    float wB = sin((PI/2.0f)*(dp));
    float wC = sin((PI)*(dp+1));
    
    // We compute the final value
    float fz = wA * vA.x + wB * vB.x + wC * vC.x;
    
    pos = vec3(vA.x, vB.x, fz);
    gl_Position = trans * vec4(aPos.x, aPos.y, fz, 1.0);
    }