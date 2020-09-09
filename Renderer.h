#include "Monitor.h"
#include "OpenCL_Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Renderer{
public:
    Renderer(Monitor* win);
    void runSingleWFS(int shaderProgram);
    void runMultipleWFS(int shaderProgram, int WFSnum, int btwnFramesNum);
    
private:
    GLFWwindow* window;
    OpenCL_Component* OCLmanager;
    void manage_input();
    
    // Settings
    float dt = 0.001f;    
    GLfloat c[2] = {-0.45f, 0.6f};
    float R = 2;
    float ms = 100;
    float right_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    float left_color[4]  = {0.0f, 0.0f, 1.0f, 1.0f};
    
    const char* names[10] = {"Re(C)", "Im(C)", "Nr. Iteraciones", "Radio Limite", "Color_1 R", "Color_1 G", "Color_1_B", "Color_2 R", "Color_2 G", "Color_2 B"};
    float* settings[10] = {&c[0], &c[1], &ms, &R, &right_color[0], &right_color[1], &right_color[2], &left_color[0], &left_color[1], &left_color[2]};
    float intervals[10] = {0.001f, 0.001f, 1.0f, 1.0f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f};
    float minimum[10] = {-100.0f, -100.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float maximum[10] = {100.0f, 100.0f, 10000.0f, 1000.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    };