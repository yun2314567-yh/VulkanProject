#include"myEngine/myEngine-window.h"
namespace myEngine
{
	 void Window::initWindow()
	{
		glfwInit();
		glfwInitVulkanLoader((PFN_vkGetInstanceProcAddr)vkGetInstanceProcAddr);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

		

		
		//frameBufferResizeCallBack(window, width, height);
	}

	 void Window::createWindowSurface(VkInstance& instance, VkSurfaceKHR& surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("窗口表面创建失败");
		}
	}

	
}