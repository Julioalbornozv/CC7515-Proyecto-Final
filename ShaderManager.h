#pragma once
//#include <glad/glad.h>
#include <GL/glew.h>
#include <string>

class ShaderManager{
public:
    const char* vert_str;
    const char* frag_str;

    ShaderManager(const char* vertex_path, const char* fragment_path);
    const char* load_file(const char* path);
    int compile_shaders();
    };