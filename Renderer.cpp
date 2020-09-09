#include "Renderer.h"
#include "OpenCL_Component.h"
#include <iostream>
#include <random>
#include <utility>
#include <time.h>

// Window size
#define WIDTH 1000
#define HEIGHT 1000

// Surface mesh dimensions
#define VX 512
#define VY 512  

// Perlin noise grid size
#define PX 32
#define PY 32

int n = 0;
glm::mat4 trans = glm::mat4(1.0f);

Renderer::Renderer(Monitor* screen){
    window = screen->GLFW_init();
    screen->OpenGL_init();
    OCLmanager = new OpenCL_Component();
    int status = OCLmanager->OpenCL_init();
    if (status != CL_SUCCESS){
        std::cout << "OpenCL failed to initialize" << std::endl;
        }
    std::cout << "OpenCL compilation completed" << std::endl;
    }

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_Q && action == GLFW_PRESS && n > 0 && n <= 10){
        n--;
        }
    else if (key == GLFW_KEY_E && action == GLFW_PRESS && n >= 0 && n < 9){
        n++;
        }
    }

void mouse_input(GLFWwindow* win, int button, int action, int mods){
    double xpos, ypos;
    glfwGetCursorPos(win, &xpos, &ypos);
        
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){ 
        trans = glm::scale(trans, glm::vec3(2.0f, 2.0f, 2.0f));
        
        }
    else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        trans = glm::scale(trans, glm::vec3(0.5f, 0.5f,0.5f));
        
        }
    }
    
void Renderer::manage_input(){
    
    std::cout << "Current setting: " << names[n] << " = " << *settings[n] << "           \r";
        
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
		glfwSetWindowShouldClose(window, true);
		}   
    
    // Modify setting
    else if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && *settings[n] < maximum[n]){
        *settings[n] += intervals[n];
        }
    else if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && *settings[n] > minimum[n]){
        *settings[n] -= intervals[n];
        }
    }
   
float* generate_heightmap(int size){
    // Returns array of given size with random data between 0 and 1
    float* data = new float[size];
    std::default_random_engine eng{static_cast<long unsigned int>(time(0))};
    std::uniform_real_distribution<> dis(0.0, 1.0);
    for (int i = 0; i < size; i++){
        data[i] = dis(eng);
        }
    return data;
    }
    
void basic_triangulation(float* vertices, unsigned int* indices){
        
    std::cout << "Generating vertices" << std::endl;
    // Generate vertex coordinates for the grid
    
    float step_x = 1.0f/((VX-1)/2.0f);
    float step_y = 1.0f/((VY-1)/2.0f);
    int caret;
    for (int i = 0; i < VX; i++){
        for (int j = 0; j < VY; j++){
            caret = 2*(i + VX*j);
            vertices[caret] = -1.0f + step_x*i;
            vertices[caret+1] = 1.0f - step_y*j;
            }
        }
    
    std::cout << "Generating indeces" << std::endl;
    // Generate indeces for each triangle
    
    for (int i = 0; i < VX-1; i++){
        for (int j = 0; j < VY-1; j++){
            caret = 6*(i +(VX-1)*j);
            // "Upper Right triangle"
            indices[caret] = i + j*VX;
            indices[caret+1] = (i+1) + j*VX;
            indices[caret+2] = (i+1) + (j+1)*VX;
            
            // "Lower Left triangle"
            indices[caret+3] = i + j*VX;
            indices[caret+4] = (i+1) + (j+1)*VX;
            indices[caret+5] = i + (j+1)*VX;
            }
        }
    }

void Renderer::runSingleWFS(int programID){
    glfwSetMouseButtonCallback(window, mouse_input);
    glfwSetKeyCallback(window, key_callback);
    
    // Generate texture with gradient data
    //float* data = generate_heightmap(VX*VY);
    
    OCLmanager->execute(PX,PY,VX,VY);
    float* data = (float*)OCLmanager->getResult();
    
    // Generate triangulation
    float* vertices = new float[VX*VY*2];
    unsigned int* indices = new unsigned int[(VX-1)*(VY-1)*2*3];
    
    basic_triangulation(vertices, indices);
    
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Populate texture with data
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, PX, PY, 0, GL_RG, GL_FLOAT, data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, VX, VY, 0, GL_RED, GL_FLOAT, data);
    
    // Compile Buffer objects for the mesh
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*VX*VY*2, vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*(VX-1)*(VY-1)*2*3, indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
    
    //trans = glm::scale(trans, glm::vec3(0.5f, 0.5f, 0.5f));
    GLfloat tdim[2] = {VX, VY};
    
    GLuint tex = glGetUniformLocation(programID, "heightmap");
    std::cout << tex << std::endl;
    glUniform1i(tex, 0);    
    
    /// Wireframe mode 
    /// TODO: Make it a toggleable option
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    while (!glfwWindowShouldClose(window)){
        //manage_input();
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(programID);
        
        GLuint tdimID = glGetUniformLocation(programID, "heightmap_size");
        GLuint tID = glGetUniformLocation(programID, "trans");
        
        glUniform2fv(tdimID, 1, tdim);
        glUniformMatrix4fv(tID, 1, GL_FALSE, glm::value_ptr(trans));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES , ((VX-1)*(VY-1)*2)*3, GL_UNSIGNED_INT, 0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        }
        
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(programID);
    
    glfwTerminate();
    }
    
void Renderer::runMultipleWFS(int programID, int WFSnum, int btwnFramesNum){
    glfwSetMouseButtonCallback(window, mouse_input);
    glfwSetKeyCallback(window, key_callback);
    
    // Generate texture with gradient data
    OCLmanager->executeN(WFSnum,PX,PY,VX,VY);
    float** data = (float**)OCLmanager->getNResult();
    
    
    // Generate triangulation
    float* vertices = new float[VX*VY*2];
    unsigned int* indices = new unsigned int[(VX-1)*(VY-1)*2*3];
    
    basic_triangulation(vertices, indices);
    
    // We generate a texture from each heightmap created by the GPU
    GLuint hmap_textures[WFSnum];
    glGenTextures(WFSnum, hmap_textures);
        
    for (int i = 0; i < WFSnum; i++){
        // Define texture
        glBindTexture(GL_TEXTURE_2D, hmap_textures[i]);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        // Populate texture with data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, VX, VY, 0, GL_RED, GL_FLOAT, data[i]);
        }
    
    // Compile Buffer objects for the mesh
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*VX*VY*2, vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*(VX-1)*(VY-1)*2*3, indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
    
    //trans = glm::scale(trans, glm::vec3(0.5f, 0.5f, 0.5f));
    GLfloat tdim[2] = {VX, VY};
    
    // We now need to pass three textures to the vertex buffer at all times, these
    // textures correspond to the previous frame, the next frame before our point in 
    // time and the subsequent frame. (Named A, B and C respectively)
    
    glUseProgram(programID); 
    GLuint texA = glGetUniformLocation(programID, "heightmapA");    
    GLuint texB = glGetUniformLocation(programID, "heightmapB");
    GLuint texC = glGetUniformLocation(programID, "heightmapC");
    
    glUniform1i(texA, 0);    
    glUniform1i(texB, 1);    
    glUniform1i(texC, 2);    
    
    /// Wireframe mode 
    /// TODO: Make it a toggleable option
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    int btwnFrameCounter = 0;
    int hmapIndex = 0;
    float prev_time = 0;
    float time = 0;
    float dt;
    
    // Time units are not specified. For our purposes we will assume that each
    // texture are "located" at even intervals.
    
    //  Time between each stamp: 60 secs
    float between = 60.0f;
    float time_step = between / (float)btwnFramesNum;
    
    while (!glfwWindowShouldClose(window)){
        //manage_input();
        //time += time_step;
        btwnFrameCounter++;
        if (btwnFrameCounter >= btwnFramesNum){  // Enough intermediate frames where created
            // We need to move our texture window
            hmapIndex = (hmapIndex + 1) % WFSnum;
            btwnFrameCounter = 0;
            prev_time = time;
            }
        
        time += time_step;
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(programID);
        
        GLuint tdimID = glGetUniformLocation(programID, "heightmap_size");
        GLuint tID = glGetUniformLocation(programID, "trans");
        GLuint dp = glGetUniformLocation(programID, "dp");
        
        
        dt = time-prev_time;
        //std::cout << dt << std::endl;
        glUniform1f(dp, dt/between);
        
        glUniform2fv(tdimID, 1, tdim);
        glUniformMatrix4fv(tID, 1, GL_FALSE, glm::value_ptr(trans));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hmap_textures[hmapIndex]);
        
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, hmap_textures[(hmapIndex + 1) % WFSnum]);
        
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, hmap_textures[(hmapIndex + 2) % WFSnum]);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES , ((VX-1)*(VY-1)*2)*3, GL_UNSIGNED_INT, 0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        }
        
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(programID);
    
    glfwTerminate();
    }