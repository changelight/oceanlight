#include <GLFW/glfw3.h>

void lol_glfw_key_callback(GLFWwindow* window,
				  int key,
				  int scancode,
				  int action,
				  int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}
