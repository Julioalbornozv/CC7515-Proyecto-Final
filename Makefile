CXX= g++
CXXFLAGS= -Wall -Wfatal-errors -std=c++17 $(INCLUDES)
INCLUDES= -I .\include
LDFLAGS= -L .\lib
LDLIBS= -lglfw3 -lopengl32 -lgdi32 -lglu32 -lglew32 -lOpenCL

install: 
	$(CXX) -o ocean_viewer main.cpp Monitor.cpp Renderer.cpp ShaderManager.cpp OpenCL_Component.cpp -g $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f *.o
