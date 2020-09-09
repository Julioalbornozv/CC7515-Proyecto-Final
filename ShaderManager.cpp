#include "ShaderManager.h"
#include <iostream>
#include <fstream>

ShaderManager::ShaderManager(const char* vert_path, const char* frag_path){
    vert_str = load_file(vert_path);
    frag_str = load_file(frag_path);
    }
    
const char* ShaderManager::load_file(const char* path){
    FILE *fp;
    char *source_str;
    size_t program_size;
    
    fp = fopen(path, "rb");
    if (!fp) {
        printf("Failed to load kernel\n");
            return NULL;
    }
    
    fseek(fp, 0, SEEK_END);
    program_size = ftell(fp);
    rewind(fp);
    source_str = (char*)malloc(program_size + 1);
    source_str[program_size] = '\0';
    fread(source_str, sizeof(char), program_size, fp);
    fclose(fp);
    
    return source_str;
    }
    
int ShaderManager::compile_shaders(){
    int success;
    char infoLog[512];
    
    int vertexID   = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexID, 1, &vert_str, NULL);
    glCompileShader(vertexID);
    glGetShaderiv(vertexID, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(vertexID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    int fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentID, 1, &frag_str, NULL);
    glCompileShader(fragmentID);
    glGetShaderiv(fragmentID, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(fragmentID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        
    int programID  = glCreateProgram();
    glAttachShader(programID, vertexID);
    glAttachShader(programID, fragmentID);
    glLinkProgram(programID);
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        
    glDeleteShader(vertexID);
    glDeleteShader(fragmentID);
    return programID;
    }