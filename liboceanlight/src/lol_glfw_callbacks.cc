#include <GLFW/glfw3.h>
#include <iostream>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_window.hpp>

bool first_mouse {true};
double lastx {320.0f}, lasty {240.0f};
extern liboceanlight::engine::lol_camera camera;

void lol_glfw_error_callback(int error_code, const char* description)
{
	std::cerr << "Error " << error_code << ": " << description << "\n";
}

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

void lol_glfw_scroll_callback(GLFWwindow* window_pointer,
							  double xoffset,
							  double yoffset)
{
	scroll_offset = yoffset;
}

void lol_glfw_cursor_pos_callback(GLFWwindow* window_pointer,
								  double posx,
								  double posy)
{
	if (first_mouse)
	{
		lastx = posx;
		lasty = posy;
		first_mouse = false;
	}

	double xoffset = posx - lastx;
	double yoffset = lasty - posy;
	lastx = posx;
	lasty = posy;

	float sensitivity = camera.sensitivity;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	camera.yaw -= static_cast<float>(xoffset);
	camera.pitch += static_cast<float>(yoffset);

	if (camera.pitch > 89.0f)
	{
		camera.pitch = 89.0f;
	}

	if (camera.pitch < -89.0f)
	{
		camera.pitch = -89.0f;
	}

	camera.direction.x = sin(glm::radians(camera.yaw)) *
						 cos(glm::radians(camera.pitch));
	camera.direction.y = sin(glm::radians(camera.pitch));
	camera.direction.z = cos(glm::radians(camera.yaw)) *
						 cos(glm::radians(camera.pitch));
	camera.center = glm::normalize(camera.direction);
}

void lol_glfw_framebuffer_size_callback(GLFWwindow* window_pointer,
										int width,
										int height)
{
	auto window = static_cast<liboceanlight::window*>(
		glfwGetWindowUserPointer(window_pointer));
	window->framebuffer_resized = true;
}
