#pragma once
#include"myEngine-window.h"

namespace myEngine
{
	


	class Engine
	{
	public:
		int windowWidth = 800;
		int windowHeight = 600;
		
		void run()
		{
			while (!window.shouldBeClosed())
			{
				glfwPollEvents();
			}
		}

	private:
		void createSyncObjects();

		VkSurfaceKHR surface;

		Window window{windowWidth,windowHeight,"Vulkan"};
	};

	
}



