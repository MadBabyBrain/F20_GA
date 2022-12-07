#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

int main() {
	// init glfw
	glfwInit();

	// add hints to glfw to tell what version of opengl should be used
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// creates window object from glfw
	GLFWwindow* window = glfwCreateWindow(800, 800, "Test Window", NULL, NULL);

	// check if window managed to be created
	if (window == NULL) {
		std::cout << "Failed to create window" << std::endl;
		// exit glfw due to window not being created
		glfwTerminate();
		return  -1;
	}

	// add window to current context
	glfwMakeContextCurrent(window);
	// load glad for opengl
	gladLoadGL();

	// create glad as same size as window
	glViewport(0, 0, 800, 800);

	// set back texture of window
	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	// clear back buffer
	glClear(GL_COLOR_BUFFER_BIT);
	// swap buffers to view background color
	glfwSwapBuffers(window);

	// while window is open
	while (!glfwWindowShouldClose(window)) {
		// handle glfw events
		glfwPollEvents();
	}

	// delete window before ending program
	glfwDestroyWindow(window);
	// exit glfw
	glfwTerminate();

	return 0;

}