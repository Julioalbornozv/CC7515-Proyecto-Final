//#include <glad/glad.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

class Monitor{
public:
    int width, height;
	
	Monitor(int mon_x, int mon_y);
	GLFWwindow* GLFW_init();
	void OpenGL_init();
	
private:
	GLFWwindow* window;
    //void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	};