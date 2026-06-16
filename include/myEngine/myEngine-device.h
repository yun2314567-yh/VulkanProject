#pragma once
#include<vector>
#include<stdexcept>
#include<iostream>

#define VK_NO_PROTOTYPES
#include<volk.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include"myEngine-window.h"
namespace myEngine
{

	static const VkBool32 MY_BOOL_TRUE = VK_TRUE;

	static const VkBool32 MY_BOOL_FALSE = VK_FALSE;

	const int MAX_FRAMEBUFFERS_IN_SAMETIME = 3;

	

	enum myTextureType :uint32_t
	{

		DIFFUSE = 0,
		NORMAL = 1,
		ROUGHNESS = 2,
		SPECULAR = 3,
		HEIGHT = 4,
		SHADOWMAP = 5,
		MAX_TEXTURE_COUNT
	};

	enum PipelineType
	{
		DEFAULT,
		SHADOWPASS

	};
	
	struct UBO
	{
		glm::mat4 model{ 1.f };
		glm::mat4 view{ 1.f };
		glm::mat4 proj{ 1.f };		
		glm::mat4 lightProj{ 1. };
		glm::mat4 lightView{ 1.f };		
		glm::vec3 camPos{ 0.f };
		glm::vec3 lightPos;
	};

	struct QueueFamilyIndex
	{
		int graphicsFamilys = -1;
		int presentFamilys = -1;
		bool isComplete()
		{
			return graphicsFamilys >= 0 && presentFamilys >= 0;
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR features;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

#ifdef NODEBUG
	const bool enableValidatiobLayers = false;

#else
	const bool enableValidatiobLayers = true;

#endif // NODEBUG



	class Device
	{
	public:
		Device(Window& window);
		

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(const Device&&) = delete;
		Device& operator=(const Device&&) = delete;

		VkInstance getInstance() { return instance; }
		VkQueue getGraphicsQueue() { return graphicsQueue; }
		VkQueue getPresentQueue() { return presentQueue; }
		VkDevice getLogicalDevice() { return logicalDevice; }
		VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
		VkSurfaceKHR getSurface() { return surface; }

		QueueFamilyIndex getQueueFamilyIndices() { return indices; }

		VkCommandPool getCommandPool() { return commandPool; }
		
		
		
		void createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkDeviceSize size, VkBuffer& targetBuffer, VkDeviceMemory& targetBufferMemory);

		VkFormat findDepthFormat();
		
		void copyBuffer(VkBuffer srcBuffer, VkBuffer targetBuffer, VkDeviceSize size);

		void copyBuffer2Image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		void createImage(const uint32_t& texWidth, const uint32_t& texHeight, const VkImageType& imageType, const VkFormat& format,
			const VkImageTiling& tiling, const VkImageUsageFlags& usage, const VkSharingMode& sharingMode,
			const VkSampleCountFlagBits& sampleWay, VkImage& image, VkDeviceMemory& imageMemory);
		
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,VkCommandBuffer command=VK_NULL_HANDLE);

		VkImageView createImageView( const VkImage& image, const VkFormat& format, VkImageAspectFlags aspectMask);

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		SwapChainSupportDetails querySwapChainSupport();

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);

		~Device()
		{
			vkDeviceWaitIdle(logicalDevice);

			vkDestroySurfaceKHR(instance, surface, nullptr);
			vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
			vkDestroyDevice(logicalDevice, nullptr);

			if (enableValidatiobLayers)
				DestroyDebugUtilsMessengerEXT(instance, callback, nullptr);
			
			vkDestroyInstance(instance, nullptr);
		}


	private:
		void createInstance();
		void setupDebugCallBack();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();

		

		VkInstance instance;

		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
		
		VkSurfaceKHR surface;

		VkQueue graphicsQueue;
		VkQueue presentQueue;

		VkCommandPool commandPool;

		VkDebugUtilsMessengerEXT callback;
		QueueFamilyIndex indices;

		Window& _window;



		

		VkFormat findDepthSupport(const std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		
		
		int rateDeviceSuitability();

	    static bool checkValidationLayerSupport();
		static std::vector<const char*> getRequiredExtensions();
		
		bool checkDeviceExtensionSupport(); 

		void findQueueFamilies();
		
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		}

		VkResult CreateDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pCallback
		);
	    
		static bool hasStencilComponent(VkFormat format)
		{
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

		void DestroyDebugUtilsMessengerEXT(
			VkInstance instance,
			VkDebugUtilsMessengerEXT callback,
			const VkAllocationCallbacks* pAllocator
		);

		
	};
}
