#version 330 core

layout (location = 0) in vec2 aPos;

uniform vec2 heightmap_size;
uniform mat4 trans;
uniform sampler2D heightmap;

out vec3 pos;

void main(){
    const float frequency = 1.0;
    const float amplitude = 1.5;
    vec2 texPos = vec2((aPos + vec2(1.0f)) / 2.0f);
    vec4 v = texture(heightmap, texPos);
        	  
    pos = vec3(aPos.x, aPos.y, v.x);
    gl_Position = trans * vec4(aPos.x, aPos.y, 0.0, 1.0);
    }