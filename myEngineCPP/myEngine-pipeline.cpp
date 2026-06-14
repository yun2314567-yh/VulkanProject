#include"myEngine/myEngine-pipeline.h"
#include"myEngine/myEngine-model.h"
#include<cassert>
namespace myEngine
{
	pipeline::pipeline(Device& device, const std::string vertexShaderPath,
		const std::string fragmentShaderPath, pipelineConfigInfo& configInfo):device{device}
	{
		defaultConfigInfoSet(configInfo);
		createGraphicPipeline(vertexShaderPath, fragmentShaderPath, configInfo);
	}


	void pipeline::createGraphicPipeline(const std::string vertShaderPath,const std::string fragShaderPath,pipelineConfigInfo& configInfo)
	{
		auto shaderVS = readFile(vertShaderPath);
		auto shaderFS = readFile(fragShaderPath);

		VkShaderModule shader_v = createShaderModule(shaderVS);
		VkShaderModule shader_f = createShaderModule(shaderFS);

		VkPipelineShaderStageCreateInfo vertInfo = {};
		vertInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertInfo.module = shader_v;
		vertInfo.pName = "main";     //一个vertexShader可以包含多个main，此处指定调用的部分
		vertInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragInfo = {};
		fragInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragInfo.module = shader_f;
		fragInfo.pName = "main";     //同理
		fragInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderStageInfo[] = { vertInfo ,fragInfo };

		auto bindDescription = Vertex::getBindingDescription();
		auto attributeDescription = Vertex::getAttributeDescriptions();


		VkPipelineVertexInputStateCreateInfo vertInputInfo = {};//顶点输入配置
		vertInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertInputInfo.vertexBindingDescriptionCount = 1;
		vertInputInfo.pVertexBindingDescriptions = &bindDescription;
		vertInputInfo.vertexAttributeDescriptionCount = attributeDescription.size();
		vertInputInfo.pVertexAttributeDescriptions = attributeDescription.data();


		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStageInfo;
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &configInfo.viewportInfo;
		pipelineInfo.pVertexInputState = &vertInputInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.subpass = configInfo.subpass;
		pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;//允许基于基类管线创建派生管线
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(device.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicPipeline) != VK_SUCCESS)
			throw std::runtime_error("图形管线创建失败");

		vkDestroyShaderModule(device.getLogicalDevice(), shader_f, nullptr);
		vkDestroyShaderModule(device.getLogicalDevice(), shader_v, nullptr);
	}
	
	
	VkShaderModule pipeline::createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo creataInfo = {};
		creataInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		creataInfo.codeSize = code.size();
		creataInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device.getLogicalDevice(), &creataInfo, nullptr, &shaderModule) != VK_SUCCESS)
			throw std::runtime_error("fail to create shaderMode");

		return shaderModule;
	}

	void pipeline::commandBindGraphicPipeline(VkCommandBuffer commandBuffer)
	{
		assert(graphicPipeline != VK_NULL_HANDLE && "图像管线未设置");
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline);
	}


	void pipeline::defaultConfigInfoSet(pipelineConfigInfo& configInfo)
	{
		
		
			
		//窗口设置
		configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		configInfo.viewportInfo.viewportCount = 1;
		configInfo.viewportInfo.pViewports = &configInfo.tempViewport;
		configInfo.viewportInfo.scissorCount = 1;
		configInfo.viewportInfo.pScissors = &configInfo.tempScissor;

		//图元设置
		configInfo.inputAssemblyInfo.sType= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
		configInfo.inputAssemblyInfo.topology= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		//光栅化设置
		configInfo.rasterizationInfo.sType= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizationInfo.lineWidth = 1.;
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        // Use counter-clockwise front face to match typical model winding (CCW)
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		configInfo.rasterizationInfo.depthBiasClamp = 0;
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0;
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0;

		//颜色混合附件设置
		configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE;

		//颜色混合设置
		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;

		//深度缓冲和模板缓冲设置
		configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.maxDepthBounds = 1.f;
		configInfo.depthStencilInfo.minDepthBounds = 0.f;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		
		//多重采样设置
		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.f;
		configInfo.multisampleInfo.pSampleMask = nullptr;
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;

		
		//设定启用的动态状态
		configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		
		//动态状态
		configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
		configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
	}
}