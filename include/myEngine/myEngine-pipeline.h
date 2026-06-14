#pragma once 
#include<string>
#include<array>
#include"readFile.h"

#include"myEngine-device.h"

namespace myEngine
{
	struct pipelineConfigInfo
	{
		pipelineConfigInfo() = default;
		pipelineConfigInfo(const pipelineConfigInfo&) = delete;
		pipelineConfigInfo& operator=(const pipelineConfigInfo&) = delete;
		
		//仅占位
		VkViewport tempViewport = { 0,0,0,0,0,1 };
		VkRect2D tempScissor = { {0,0},{1,1} };


		VkPipelineViewportStateCreateInfo  viewportInfo = {};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
		VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
			
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkRenderPass renderPass = VK_NULL_HANDLE;
			
		std::array<VkDynamicState, 2> dynamicStateEnables = {};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
			
		uint32_t subpass = 0;
	};
	class pipeline
	{
		//优化方向：创建一个基类管线，结合动态设置，可以采用array或unordered_map存储
		

	public:
		
		pipeline(Device& device, const std::string vertexShaderPath, const std::string fragmentShaderPath, pipelineConfigInfo& configInfo);
		
		pipeline(const pipeline&) = delete;
		pipeline& operator=(const pipeline&) = delete;
		
		~pipeline()
		{
			vkDestroyPipeline(device.getLogicalDevice(), graphicPipeline, nullptr);
		}

		void commandBindGraphicPipeline(VkCommandBuffer commandBuffer);

	private:
		Device& device;
		
		VkPipeline graphicPipeline;
		
		void defaultConfigInfoSet(pipelineConfigInfo& configInfo);
		
		void createGraphicPipeline(const std::string vertShaderPath, const std::string fragShaderPath, pipelineConfigInfo& configInfo);
		
		VkShaderModule createShaderModule(const std::vector<char>& code);
		
	};
}