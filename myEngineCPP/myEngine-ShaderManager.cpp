#include"myEngine/myEngine-ShaderManager.h"
#include"myEngine/readFile.h"
namespace myEngine
{
	ShaderManager::~ShaderManager()
	{
		for(auto& [path,info]:shaderCaches)
			vkDestroyShaderEXT(device.getLogicalDevice(), info.shader, nullptr);
	}

	VkShaderEXT ShaderManager::getORcompileShader(const std::string& shaderPath, VkShaderStageFlagBits stage, std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
	{
		auto temShader = shaderCaches.find(shaderPath);

		if(temShader != shaderCaches.end())
		{
			if (temShader->second.setLayouts == descriptorSetLayouts)
			{
				return temShader->second.shader;
			}
			else
			{
				vkDestroyShaderEXT(device.getLogicalDevice(), temShader->second.shader, nullptr);
				shaderCaches.erase(temShader);
			}
		}

		VkShaderEXT newshader = compileShader(shaderPath, stage, descriptorSetLayouts);
		
		shaderCaches[shaderPath] = { newshader,stage, std::filesystem::last_write_time(shaderPath), descriptorSetLayouts };

		return newshader;
	}

	void ShaderManager::hotReload(VkFence deleteFence)
	{
		for(auto& [path,info]:shaderCaches)
		{
			auto currentTime = std::filesystem::last_write_time(path);
			if(currentTime != info.latestWriteTime)
			{
				pushPendingDeleteShader(info.shader, deleteFence);
				info.shader = compileShader(path,info.stage, info.setLayouts);
				info.latestWriteTime = currentTime;
			}
		}
	}

	void ShaderManager::forceReload(const std::string& shaderPath, VkShaderStageFlagBits stage, const std::vector<VkDescriptorSetLayout>& setLayouts,VkFence fence)
	{
		auto temShader = shaderCaches.find(shaderPath);
		if(temShader != shaderCaches.end())
		{
			pushPendingDeleteShader(temShader->second.shader, fence);
			
			shaderCaches.erase(temShader);
		}
		
		shaderCaches[shaderPath] = { compileShader(shaderPath, stage, setLayouts),stage, std::filesystem::last_write_time(shaderPath), setLayouts };
	}

    VkShaderEXT ShaderManager::compileShader(const std::string& shaderPath, VkShaderStageFlagBits stage,std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
    {
		auto shaderCode = readFile(shaderPath);
		std::cout << "compileShader: path=" << shaderPath << " stage=" << stage << " setLayoutsCount=" << descriptorSetLayouts.size() << "\n";
		for (size_t i = 0; i < descriptorSetLayouts.size(); ++i) {
			std::cout << "  setLayout[" << i << "] = " << descriptorSetLayouts[i] << "\n";
		}

		VkShaderModuleCreateInfo shaderModuleInfo = {};
		shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleInfo.codeSize = shaderCode.size();
		shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		VkShaderModule shaderModule;

		if (vkCreateShaderModule(device.getLogicalDevice(), &shaderModuleInfo, nullptr, &shaderModule) != VK_SUCCESS)
			throw std::runtime_error("着色器模块创建失败");

		VkShaderCreateInfoEXT shaderCreateInfo = {};
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
		shaderCreateInfo.stage = stage;
		shaderCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
		shaderCreateInfo.codeSize = shaderCode.size();
		shaderCreateInfo.pCode = shaderCode.data();
		shaderCreateInfo.pName = "main";
		shaderCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		shaderCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	
        // For shader-object linking: if this is a vertex shader and a fragment shader
		// will follow, set nextStage to include the fragment bit. Otherwise leave 0.
		shaderCreateInfo.nextStage = (stage == VK_SHADER_STAGE_VERTEX_BIT) ? VK_SHADER_STAGE_FRAGMENT_BIT : 0;

		VkShaderEXT shader;

		if (vkCreateShadersEXT(device.getLogicalDevice(), 1, &shaderCreateInfo, nullptr, &shader) != VK_SUCCESS)
			throw std::runtime_error("着色器创建失败");

		std::cout << " compileShader: created shader object " << shader << " for path=" << shaderPath << "\n";

		vkDestroyShaderModule(device.getLogicalDevice(), shaderModule, nullptr);

		return shader;
    }

	void ShaderManager::update()
	{
		auto it = pendingDeleteShaders.begin();
		while(it!= pendingDeleteShaders.end())
		{
			VkResult result = vkGetFenceStatus(device.getLogicalDevice(), it->deleteFence);
			if(result == VK_SUCCESS)
			{
				vkDestroyShaderEXT(device.getLogicalDevice(), it->penderShader, nullptr);
				it = pendingDeleteShaders.erase(it);
			}
			else if(result == VK_NOT_READY)
				++it;
			else 
				throw std::runtime_error("设备丢失");
		}
	}


	void ShaderManager::pushPendingDeleteShader(VkShaderEXT shader, VkFence deleteFence)
	{
		pendingDeleteShaders.push_back({ shader, deleteFence });
	}
}