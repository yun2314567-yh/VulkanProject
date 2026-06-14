#pragma once
#include<volk.h>

#include <stdexcept>
#include <string>
#include"imgui/imgui.h"
#include"imgui/imgui_impl_glfw.h"
#include"imgui/imgui_impl_vulkan.h"
#include"imgui/imgui_internal.h"

#include"myEngine-window.h"
#include"myEngine-device.h";
#include"myEngine-Renderer.h"
#include"myEngine-model.h"
namespace myEngine
{
	class GUI
	{
	public:
		GUI(Window& window,Device& device,Renderer& renderer);
		~GUI()
		{ 
			vkDeviceWaitIdle(_device.getLogicalDevice());
			
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			
			vkDestroyDescriptorPool(_device.getLogicalDevice(), GUIpool, nullptr);
		}

		VkExtent2D getCurrentViewSize();

		void updateDock();

		void initTextureID(VkSampler sampler, VkImageView imageView);
		
		//bool isViewWindowHovered() { return m_viewWindowHovered; }

		void drawViewWindow(VkExtent2D extent);
		void drawModelInfoWindow(Model* model);
		void drawShaderInfoWindow();

		int getSelectedMeshIndex() { return selectedMeshIndex; }

		void newFrame( bool& hasdroppedFile, std::string& droppedFilePath,Model* model=nullptr);

		void render(VkCommandBuffer command);
	private:
		std::string textureTypeToString(myTextureType texType);

		Window& _window;
		Device& _device;
		Renderer& _renderer;
		ImGuiIO* io=nullptr;
		VkDescriptorSet viewTextureID=VK_NULL_HANDLE;
		
		ImGuiID dockSpaceId;

		bool initialized = false;
		int selectedMeshIndex = -1;
		
		bool m_viewWindowHovered = false;

		// Pending OS drop stored between frames; newFrame will populate this and
		// drawModelInfoWindow will apply it to the hovered item.
		bool pendingDrop = false;
		std::string pendingDropPath;
		
		
		ImGui_ImplVulkan_InitInfo init_info = {};
		VkDescriptorPool GUIpool;
	};
}