#include "myEngine/myEngine-RenderSystem.h"

namespace myEngine
{
	RenderSystem::RenderSystem(Device& device) :device{ device }
	{
		//createPipelineLayout(descriptorSetLayouts);
		//createPipeline(renderPass);
	}
	RenderSystem::~RenderSystem()
	{
		// RenderSystem does not own any pipeline layout; destruction handled by Material
	}



	void RenderSystem::renderModel(PipelineType pipelineType, VkCommandBuffer commandBuffer, Model* model, VkDescriptorSet shadowSet)
	{
		//std::cout << "RenderSystem::renderModel called. pipelineLayout=" << pipelineLayout << " mvpSet=" << mvpDescritorSet << " model=" << model << "\n";
		 /*if (renderPipeline)
			 std::cout << " renderPipeline exists\n";
		 else
			 std::cout << " renderPipeline null\n";*/
		switch (pipelineType)
		{
		case myEngine::DEFAULT:
			setDefaultPipelineConfigInfo(commandBuffer);
			break;
		case myEngine::SHADOWPASS:
			setShadowMapPipelineConfigInfo(commandBuffer);
			break;
		default:
			setDefaultPipelineConfigInfo(commandBuffer);
			break;
		}




		model->draw(commandBuffer, pipelineType, shadowSet);
	}

	void  RenderSystem::setDefaultPipelineConfigInfo(VkCommandBuffer command)
	{
		uint32_t sampleMask = 0x1;

		auto bindDescription = Vertex::getBindingDescription2();
		auto attributeDescription = Vertex::getAttributeDescriptions2();

		vkCmdSetVertexInputEXT(command, 1, &bindDescription, static_cast<uint32_t>(attributeDescription.size()), attributeDescription.data());


		vkCmdSetAlphaToCoverageEnableEXT(command, MY_BOOL_FALSE);

		vkCmdSetRasterizationSamplesEXT(command, VK_SAMPLE_COUNT_1_BIT);

		vkCmdSetPrimitiveRestartEnableEXT(command, MY_BOOL_FALSE);
		vkCmdSetPrimitiveTopologyEXT(command, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		vkCmdSetRasterizerDiscardEnableEXT(command, MY_BOOL_FALSE);

		vkCmdSetSampleMaskEXT(command, VK_SAMPLE_COUNT_1_BIT, &sampleMask);



		vkCmdSetColorBlendEnableEXT(command, 0, 1, &MY_BOOL_FALSE);
		assert(vkCmdSetLogicOpEnableEXT != nullptr && "扩展函数指针为空");


		vkCmdSetLogicOpEnableEXT(command, MY_BOOL_FALSE);


		vkCmdSetColorWriteEnableEXT(command, 1, &MY_BOOL_TRUE);

		VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		vkCmdSetColorWriteMaskEXT(command, 0, 1, &colorWriteMask);


		vkCmdSetDepthTestEnableEXT(command, MY_BOOL_TRUE);
		vkCmdSetDepthWriteEnableEXT(command, MY_BOOL_TRUE);

		vkCmdSetDepthClampEnableEXT(command, MY_BOOL_FALSE);
		//VkDepthClampRangeEXT depthClampRange = {0.0f, 1.0f};
		//vkCmdSetDepthClampRangeEXT(command, VK_DEPTH_CLAMP_MODE_USER_DEFINED_RANGE_EXT, &depthClampRange);

		vkCmdSetDepthBiasEnable(command, MY_BOOL_FALSE);
		vkCmdSetDepthCompareOpEXT(command, VK_COMPARE_OP_LESS);

		vkCmdSetDepthBoundsTestEnableEXT(command, MY_BOOL_FALSE);

		// Use counter-clockwise as the front face (matches common model winding from exporters)
		vkCmdSetFrontFaceEXT(command, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		vkCmdSetCullModeEXT(command, VK_CULL_MODE_BACK_BIT);

		vkCmdSetLineWidth(command, 1.0f);

		vkCmdSetPolygonModeEXT(command, VK_POLYGON_MODE_FILL);

		vkCmdSetStencilTestEnableEXT(command, MY_BOOL_FALSE);

	}

	void RenderSystem::setShadowMapPipelineConfigInfo(VkCommandBuffer command)
	{
		uint32_t sampleMask = 0x1;

		auto bindDescription = Vertex::getBindingDescription2();
		auto attributeDescription = Vertex::getAttributeDescriptions2();

		vkCmdSetVertexInputEXT(command, 1, &bindDescription, static_cast<uint32_t>(attributeDescription.size()), attributeDescription.data());


		vkCmdSetAlphaToCoverageEnableEXT(command, MY_BOOL_FALSE);

		vkCmdSetRasterizationSamplesEXT(command, VK_SAMPLE_COUNT_1_BIT);

		vkCmdSetPrimitiveRestartEnableEXT(command, MY_BOOL_FALSE);
		vkCmdSetPrimitiveTopologyEXT(command, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		vkCmdSetRasterizerDiscardEnableEXT(command, MY_BOOL_FALSE);

		vkCmdSetSampleMaskEXT(command, VK_SAMPLE_COUNT_1_BIT, &sampleMask);



		vkCmdSetColorBlendEnableEXT(command, 0, 1, &MY_BOOL_FALSE);
		assert(vkCmdSetLogicOpEnableEXT != nullptr && "扩展函数指针为空");


		vkCmdSetLogicOpEnableEXT(command, MY_BOOL_FALSE);


		vkCmdSetColorWriteEnableEXT(command, 1, &MY_BOOL_FALSE);

		VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		vkCmdSetColorWriteMaskEXT(command, 0, 1, &colorWriteMask);


		vkCmdSetDepthTestEnableEXT(command, MY_BOOL_TRUE);
		vkCmdSetDepthWriteEnableEXT(command, MY_BOOL_TRUE);

		vkCmdSetDepthClampEnableEXT(command, MY_BOOL_TRUE);
		//VkDepthClampRangeEXT depthClampRange = {0.0f, 1.0f};
		//vkCmdSetDepthClampRangeEXT(command, VK_DEPTH_CLAMP_MODE_USER_DEFINED_RANGE_EXT, &depthClampRange);

		vkCmdSetDepthBiasEnable(command, MY_BOOL_TRUE);

		VkDepthBiasInfoEXT biasEXT = {};
		biasEXT.sType = VK_STRUCTURE_TYPE_DEPTH_BIAS_INFO_EXT;
		biasEXT.depthBiasClamp = 2.f;
		biasEXT.depthBiasConstantFactor = 0.05f;
		biasEXT.depthBiasSlopeFactor = 1.25f;
		vkCmdSetDepthBias2EXT(command, &biasEXT);

		vkCmdSetDepthCompareOpEXT(command, VK_COMPARE_OP_LESS_OR_EQUAL);

		vkCmdSetDepthBoundsTestEnableEXT(command, MY_BOOL_FALSE);

		// Use counter-clockwise as the front face (matches common model winding from exporters)
		vkCmdSetFrontFaceEXT(command, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		vkCmdSetCullModeEXT(command, VK_CULL_MODE_BACK_BIT);

		vkCmdSetLineWidth(command, 1.0f);

		vkCmdSetPolygonModeEXT(command, VK_POLYGON_MODE_FILL);

		vkCmdSetStencilTestEnableEXT(command, MY_BOOL_FALSE);
	}
}

