#pragma once
#include"myEngine-device.h"
//#include"myEngine-pipeline.h"
#include"myEngine-model.h"

namespace myEngine
{
	

	class RenderSystem
	{
	public:
		RenderSystem(Device& device);
		~RenderSystem();


		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;


		void renderModel(PipelineType pipelineType, VkCommandBuffer commandBuffer,  Model* model, VkDescriptorSet shadowSet = VK_NULL_HANDLE);

	private:
		/*void createPipelineLayout(const std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
		void createPipeline(VkRenderPass renderPass);*/

		void setDefaultPipelineConfigInfo(VkCommandBuffer command);
		void setShadowMapPipelineConfigInfo(VkCommandBuffer command);
		

		Device& device;
		
	};
}