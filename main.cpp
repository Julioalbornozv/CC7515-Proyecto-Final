#include "Renderer.h"
#include "ShaderManager.h"
#include <iostream>
#include <string>
#include <map>

#define WIDTH 1000
#define HEIGHT 1000


int main(int argc, char** argv){
    // Initialize GLFW
    // Initialize OpenGL
    std::cout << "Initializing monitor" << std::endl;
    Monitor screen = Monitor(WIDTH, HEIGHT);
    
    // Initialize OpenGL and GLFW
    std::cout << "Initializing renderer" << std::endl;
    Renderer Rd = Renderer(&screen);
    
    // Compile shaders
    std::cout << "Compiling shaders" << std::endl;
    ShaderManager manager = ShaderManager("ocean_vertN.glsl", "ocean_frag.glsl");
    int shaderProgram = manager.compile_shaders();
    
    std::cout << "Obtained shaderID: " << shaderProgram << std::endl;
    // Run render loop
    std::cout << "Starting" << std::endl;
    //Rd.runSingleWFS(shaderProgram);
    Rd.runMultipleWFS(shaderProgram, 200, 300);
    }