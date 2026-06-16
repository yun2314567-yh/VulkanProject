

#include<map>
#include<set>
#include"myEngine/myEngine-device.h"



namespace myEngine
{
	

	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
	VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
	 VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME,
	 VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
	 VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
	 VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
	 VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME
	 
	};
	
	Device::Device(Window& window) :_window(window)
	{
		if (volkInitialize() != VK_SUCCESS) {
			throw std::runtime_error("Failed to initialize volk!");
		}
		createInstance();
		volkLoadInstance(instance);
		setupDebugCallBack();
		window.createWindowSurface(instance, surface);
		pickPhysicalDevice();
		createLogicalDevice();
		volkLoadDevice(logicalDevice);
		createCommandPool();
		
		
	}



	void Device::createInstance()
	{
		if (enableValidatiobLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("启用校验层，但不支持！");
		}




		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		std::cout << "available extensions:\n";
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
		}


		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;


		VkInstanceCreateInfo instanceInfo = {};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);


		auto extension = getRequiredExtensions();
		instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extension.size());
		instanceInfo.ppEnabledExtensionNames = extension.data();


		if (enableValidatiobLayers) {
			instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instanceInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			instanceInfo.enabledLayerCount = 0;
			instanceInfo.ppEnabledLayerNames = nullptr;
		}

		VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);

		if (result != VK_SUCCESS) {
			throw std::runtime_error("Vulkan实例创建失败");
		}
	}
	void Device::setupDebugCallBack()
	{
		if (!enableValidatiobLayers)return;

		VkDebugUtilsMessengerCreateInfoEXT debugSetInfo = {};
		debugSetInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugSetInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugSetInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugSetInfo.pfnUserCallback = debugCallback;
		debugSetInfo.pUserData = nullptr;

		if (CreateDebugUtilsMessengerEXT(instance, &debugSetInfo, nullptr, &callback) != VK_SUCCESS) {
			throw std::runtime_error("Debug回调设置失败");
		}
	}
	void Device::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
			throw std::runtime_error("无可用GPU");

		
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		std::cout << "available GPUs:\n";

		std::multimap<int, VkPhysicalDevice> candidates;//按照第一个值自动升序排列

		for (const auto& device : devices) {
			physicalDevice = device;
			findQueueFamilies();
			if (indices.isComplete())
			{
				int score = rateDeviceSuitability();
				VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = {};
				shaderObjectFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT;
				shaderObjectFeatures.pNext = nullptr;

				//VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicStateFeatures = {};
				//dynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;

				VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
				deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				deviceFeatures2.pNext = &shaderObjectFeatures;
			    
				//shaderObjectFeatures.pNext = &dynamicStateFeatures;

				vkGetPhysicalDeviceFeatures2(device, &deviceFeatures2);
				if(shaderObjectFeatures.shaderObject!=VK_TRUE)
					throw std::runtime_error("不支持shaderObject特性，无法使用Vulkan的shaderObject功能");
					

				candidates.insert(std::make_pair(score, device));
			}
		}

		if (candidates.rbegin()->first > 0) {//反向访问 
			physicalDevice = candidates.rbegin()->second;
			findQueueFamilies();
		}
		else {
			throw std::runtime_error("无合适的GPU");
		}


	}
	void Device::createLogicalDevice()
	{
		float queuePriority = 1.0f;

		std::set<int> uniqueQueueFamilies = { indices.graphicsFamilys,indices.presentFamilys };//去重，自动升序

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};

		for (int queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures PhysicalDeviceFeatures = {};
		PhysicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
		PhysicalDeviceFeatures.depthClamp = VK_TRUE;
		PhysicalDeviceFeatures.depthBiasClamp = VK_TRUE;
		

		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceInfo.pEnabledFeatures = &PhysicalDeviceFeatures;
		deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

		VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = {};
		shaderObjectFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT;
		shaderObjectFeatures.shaderObject = VK_TRUE;
		

		VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Featrues = {};
		dynamicState3Featrues.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
		dynamicState3Featrues.extendedDynamicState3ColorWriteMask = VK_TRUE;
		dynamicState3Featrues.extendedDynamicState3LogicOpEnable = VK_TRUE;
		dynamicState3Featrues.extendedDynamicState3PolygonMode = VK_TRUE;
		

		VkPhysicalDeviceColorWriteEnableFeaturesEXT colorWriteFeatures = {};
		colorWriteFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT;
		colorWriteFeatures.colorWriteEnable = VK_TRUE;

		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {};
		dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
		dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
		

		VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertexInputFeatures = {};
		vertexInputFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
		vertexInputFeatures.vertexInputDynamicState = VK_TRUE;

		VkPhysicalDeviceDescriptorIndexingFeaturesEXT desIndexFeatures = {};
		desIndexFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
		desIndexFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		desIndexFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
		desIndexFeatures.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
		desIndexFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;

		VkPhysicalDeviceDepthBiasControlFeaturesEXT depthBiasFeatures = {};
		depthBiasFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT;
		depthBiasFeatures.depthBiasControl = VK_TRUE;

		deviceInfo.pNext = &dynamicState3Featrues;
		dynamicState3Featrues.pNext = &dynamicRenderingFeatures;
		dynamicRenderingFeatures.pNext = &colorWriteFeatures;
		colorWriteFeatures.pNext = &vertexInputFeatures;
		vertexInputFeatures.pNext = &shaderObjectFeatures;
		shaderObjectFeatures.pNext = &desIndexFeatures;
		desIndexFeatures.pNext = &depthBiasFeatures;

		if (enableValidatiobLayers) {
			deviceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			deviceInfo.enabledLayerCount = 0;
			deviceInfo.ppEnabledLayerNames = nullptr;
		}

		if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
			throw std::runtime_error("逻辑设备创建失败");
		}

		vkGetDeviceQueue(logicalDevice, indices.graphicsFamilys, 0, &graphicsQueue);
		vkGetDeviceQueue(logicalDevice, indices.presentFamilys, 0, &presentQueue);
	}
	void Device::createCommandPool()
	{
		VkCommandPoolCreateInfo commandPoolInfo = {};

		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = indices.graphicsFamilys;
		commandPoolInfo.flags= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(logicalDevice, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
			throw std::runtime_error("指令池创建失败");
	}

	void Device::createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkDeviceSize size, VkBuffer& targetBuffer, VkDeviceMemory& targetBufferMemory)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;//按字节
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.flags = 0;

		if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &targetBuffer) != VK_SUCCESS)
			throw std::runtime_error("缓冲区创建失败");

		VkMemoryRequirements memRequire;
		vkGetBufferMemoryRequirements(logicalDevice, targetBuffer, &memRequire);



		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequire.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequire.memoryTypeBits, propertyFlags);

		if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &targetBufferMemory) != VK_SUCCESS)
			throw std::runtime_error("缓冲区内存分配失败");

		vkBindBufferMemory(logicalDevice, targetBuffer, targetBufferMemory, 0);

	}
	void Device::createImage(const uint32_t& texWidth, const uint32_t& texHeight, const VkImageType& imageType, const VkFormat& format,
		const VkImageTiling& tiling, const VkImageUsageFlags& usage, const VkSharingMode& sharingMode,
		const VkSampleCountFlagBits& sampleWay, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = imageType;
		imageInfo.extent.width = texWidth;
		imageInfo.extent.height = texHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = sharingMode;
		imageInfo.samples = sampleWay;

		if (vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
			throw std::runtime_error("图像创建失败");

		VkMemoryRequirements memRequire = {};

		vkGetImageMemoryRequirements(logicalDevice, image, &memRequire);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequire.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequire.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
			throw std::runtime_error("图像内存分配失败");

		vkBindImageMemory(logicalDevice, image, imageMemory, 0);
	}
	
	void Device::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,VkCommandBuffer command)
	{
		
		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;
		bool commandIsNULL = false;
		
		if (command == VK_NULL_HANDLE)
		{
			command = beginSingleTimeCommands();
			commandIsNULL = true;
		}
		
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		if (format == VK_FORMAT_D32_SFLOAT ||
			format == VK_FORMAT_D16_UNORM ||
			format == VK_FORMAT_X8_D24_UNORM_PACK32 ||
			format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
			format == VK_FORMAT_D24_UNORM_S8_UINT)
		{
			// 深度格式+模板
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (hasStencilComponent(format))
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
		{
			// 颜色格式
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}

		else if(oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}

		else if(oldLayout== VK_IMAGE_LAYOUT_PRESENT_SRC_KHR&& newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}

		else if ( oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL&& newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

			sourceStage= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			
		}
		else 
		{
			throw std::invalid_argument("不支持的布局转换");
		}

		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;



		vkCmdPipelineBarrier(command, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if(commandIsNULL)
		endSingleTimeCommands(command);


	}
	
	VkImageView Device::createImageView(const VkImage& image, const VkFormat& format, VkImageAspectFlags aspectMask)
	{
		VkImageViewCreateInfo ivInfo = {};
		ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivInfo.image = image;
		ivInfo.format = format;
		ivInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ivInfo.subresourceRange.aspectMask = aspectMask;
		ivInfo.subresourceRange.baseArrayLayer = 0;
		ivInfo.subresourceRange.baseMipLevel = 0;
		ivInfo.subresourceRange.layerCount = 1;
		ivInfo.subresourceRange.levelCount = 1;

		VkImageView imageView;

		if (vkCreateImageView(logicalDevice, &ivInfo, nullptr, &imageView) != VK_SUCCESS)
			throw std::runtime_error("图像视图创建失败");

		return imageView;
	}

	VkFormat Device::findDepthFormat()
	{
		VkFormat r = findDepthSupport({ VK_FORMAT_D32_SFLOAT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

		if (r == VK_FORMAT_UNDEFINED)
			throw std::runtime_error("深度缓冲不支持");

		return r;
	}


	void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer targetBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyInfo = {};
		copyInfo.size = size;

		vkCmdCopyBuffer(commandBuffer, srcBuffer, targetBuffer, 1, &copyInfo);

		endSingleTimeCommands(commandBuffer);//结束写入
	}

	void Device::copyBuffer2Image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		VkBufferImageCopy copyInfo = {};
		copyInfo.bufferOffset = 0;
		copyInfo.bufferRowLength = 0;
		copyInfo.bufferImageHeight = 0;
		copyInfo.imageExtent.width = width;
		copyInfo.imageExtent.height = height;
		copyInfo.imageExtent.depth = 1;
		copyInfo.imageOffset = { 0,0,0 };
		copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyInfo.imageSubresource.baseArrayLayer = 0;
		copyInfo.imageSubresource.mipLevel = 0;
		copyInfo.imageSubresource.layerCount = 1;
		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
		endSingleTimeCommands(commandBuffer);
	}

	VkCommandBuffer Device::beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		
		vkQueueSubmit(graphicsQueue, 1, &submitInfo,VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
	}

	void Device::findQueueFamilies()
	{

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamilys = i;
			}
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.presentFamilys = i;
			}

			if (indices.isComplete()) {
				break;
			}
			i++;
		}
	}

	bool Device::checkDeviceExtensionSupport()
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}

	SwapChainSupportDetails Device::querySwapChainSupport()
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.features);
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}
	
	

	uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperty;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperty);
		for (uint32_t i = 0; i < memProperty.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperty.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}
		throw std::runtime_error("无合适的内存");

	}

	VkFormat Device::findDepthSupport(const std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		VkFormat r = VK_FORMAT_UNDEFINED;
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features ? r = format :
				((tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) ? r = format : r = VK_FORMAT_UNDEFINED);
		}

		return r;
	}

	bool Device::checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);

		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> Device::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (enableValidatiobLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	int Device::rateDeviceSuitability() {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		int score = 0;
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}
		score += deviceProperties.limits.maxImageDimension2D;
		if (!deviceFeatures.geometryShader) {
			return 0;
		}
		return score;
	}


	VkResult Device::CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pCallback
	)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pCallback);

		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void Device::DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT callback,
		const VkAllocationCallbacks* pAllocator
	)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
			func(instance, callback, pAllocator);
	}

}