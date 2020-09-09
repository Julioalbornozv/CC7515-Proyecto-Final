#version 330 core
#ifdef GL_ES
precision mediump float;
#endif

in vec3 pos;
out vec4 fragColor;

void main(){
    float col = pos.z;
    fragColor = vec4(col, col, col, 1.0);
    }


