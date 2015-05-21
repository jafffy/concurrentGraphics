#include <cstdio>
#include <cstdlib>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
using namespace glm;

#include <thread>
#include <mutex>

#define GLSL(src) "#version 330 core\n" #src

class System
{
public:
	void update()
	{

	}

	void draw()
	{

	}
};

static bool g_Running = true;

void work(System* system)
{
	while (g_Running) {
		printf("Running\n");
	}
}

int main(int argc, char* argv[])
{
	GLFWwindow* window;

	if (!glfwInit()) {
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
	if (window == nullptr) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = true;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	System sys;

	std::thread workerThread { work, &sys };
	std::mutex m{};

	while (!glfwWindowShouldClose(window)) {
		glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		sys.update();
		sys.draw();

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	m.lock();
	g_Running = false;
	m.unlock();

	workerThread.join();

	glfwTerminate();

	return 0;
}