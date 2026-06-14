#pragma once
#include<string>
#include<stdexcept>
#define VK_NO_PROTOTYPES
#include<volk.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace myEngine
{
	

	class Window
	{
	public:
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		


		Window(const int windowWidth, const int windowHeight, const std::string windowName) :width(windowWidth), height(windowHeight), name(windowName)
		{
			initWindow();
		}

		~Window()
		{
			glfwDestroyWindow(window);
			glfwTerminate();
		}


		GLFWwindow* getWindow() { return window; }
	    
		bool shouldBeClosed() { return glfwWindowShouldClose(window); }
		
		bool isWindowResized() { return windowIsResized; }

		void setWindowResize(bool isResized) { this->windowIsResized = isResized; }

		void setWindowSize(int width, int height) { this->width=width; this->height = height; }

		void resetWindowResizedFlag() { windowIsResized = false; }
		
		//创建Vulkan表面
		void createWindowSurface(VkInstance &instance,VkSurfaceKHR &surface);
		
		VkExtent2D getExtent2D() { return { static_cast<uint32_t>(width),static_cast<uint32_t>(height) }; }
		
	private:
		void initWindow();
		
		 int width;
		 int height;
		 std::string name;
		 bool windowIsResized = false;
		GLFWwindow* window;

	
		
		
	};
}
