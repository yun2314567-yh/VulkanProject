#pragma once
#include<filesystem>
#include<deque>
#include<unordered_map>
#include"myEngine-device.h"
namespace myEngine
{
	class ShaderManager
	{
	public:
		ShaderManager(Device& device) :device{ device } {}
		
		 ShaderManager(const ShaderManager&) = delete;
		 ShaderManager& operator=(const ShaderManager&) = delete;
		 
		 VkShaderEXT getORcompileShader(const std::string& shaderPath, VkShaderStageFlagBits stage, std::vector<VkDescriptorSetLayout> descriptorSetLayouts);

		 void hotReload(VkFence deleteFence);

		 void forceReload(const std::string& shaderPath, VkShaderStageFlagBits stage, const std::vector<VkDescriptorSetLayout>& setLayouts,VkFence fence);

		 void update();

		 ~ShaderManager();

	private:
		struct ShaderInfo
		{
			VkShaderEXT shader;
			VkShaderStageFlagBits stage;
			std::filesystem::file_time_type latestWriteTime;
			std::vector<VkDescriptorSetLayout> setLayouts;
		}; 

		struct penderDeleteShader
		{
			VkShaderEXT penderShader;
			VkFence deleteFence;
		};

		std::unordered_map<std::string, ShaderInfo> shaderCaches;
		std::deque<penderDeleteShader> pendingDeleteShaders;

		void pushPendingDeleteShader(VkShaderEXT shader, VkFence deleteFence);


		Device& device;
		VkShaderEXT compileShader(const std::string& shaderPath, VkShaderStageFlagBits stage, std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
		
	};

}
