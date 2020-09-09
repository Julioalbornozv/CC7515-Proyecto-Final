#include "Monitor.h"
#include <iostream>
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
	}

Monitor::Monitor(int mon_x, int mon_y){
	width = mon_x;
	height = mon_y;
	}
	
GLFWwindow* Monitor::GLFW_init(){
	/***
	 * Initializes the GLFW context, returns a pointer to the newly created  
     * GLFWwindow object
	 */
	if(!glfwInit()){
        throw std::runtime_error("Unable to start GLFW");
		}
	
	window = glfwCreateWindow(width, height, "Ocean Simulation", NULL, NULL);
	
	if (!window){
        glfwTerminate();
        throw std::runtime_error("Unable to create an GLFW window");
		}
	
	glfwMakeContextCurrent(window);
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    //if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
    //    throw std::runtime_error("Unable to start GLAD");
    //    }
	glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("Couldn't initalize OpenGL");
        }
    
    return window;
	}

void Monitor::OpenGL_init(){
    /***
     * Sets up initial OpenGL parameters
     */
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, -50.0, 50.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0);
	}
    

	