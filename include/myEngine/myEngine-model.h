#pragma once
#include<vector>
#include<string>
#include<array>
#include<filesystem>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>


#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include"myEngine-Descriptor.h"
#include"myEngine-buffer.h"
#include"myEngine-device.h"
#include"myEngine-texture.h"
#include"myEngine/myEngine-Material.h"
namespace myEngine
{
	

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 texCoord;
		glm::vec3 normal; 
		glm::vec3 T;
		float biTangentFilp;
		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;//作用于vkcmdvertexBuffer
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static VkVertexInputBindingDescription2EXT getBindingDescription2() {
			VkVertexInputBindingDescription2EXT bindingDescription{};
			bindingDescription.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
			bindingDescription.binding = 0;//作用于vkcmdvertexBuffer
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			bindingDescription.divisor = 1; 
			
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription2EXT, 5> getAttributeDescriptions2() {
			std::array<VkVertexInputAttributeDescription2EXT, 5> attributeDescriptions{};
			attributeDescriptions[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			

			attributeDescriptions[1].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

			attributeDescriptions[2].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, normal);

			attributeDescriptions[3].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(Vertex, T);

			attributeDescriptions[4].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
			attributeDescriptions[4].binding = 0;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = VK_FORMAT_R32_SFLOAT;
			attributeDescriptions[4].offset = offsetof(Vertex, biTangentFilp);

			return attributeDescriptions;
		}

		static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};

			// 位置：vec3，偏移量为 0
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			// 颜色：vec3，偏移量在位置之后
			

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, normal);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(Vertex, T);

			attributeDescriptions[4].binding = 0;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = VK_FORMAT_R32_SFLOAT;
			attributeDescriptions[4].offset = offsetof(Vertex, biTangentFilp);

			return attributeDescriptions;
		}
	};

	

	struct Mesh
	{
		std::string meshName;
		std::unique_ptr<Buffer> vertexBuffer;
		std::unique_ptr<Buffer> indexBuffer;
		uint32_t indexCount;
		VkDescriptorSet textureDescriptorSet;
		
		std::shared_ptr<Material> material;
		std::unordered_map<myTextureType, std::string> texturePaths;
		

		

	};

	class Model
	{
	public:
		


		Model(std::string modelName,Device& device, const std::string& path, 
			std::vector<std::shared_ptr<Material>>& materials
			);
		
		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		Model(const Model&&) = delete;
		Model&& operator=(const Model&&) = delete;
		
		~Model()
		{
			modelMeshes.clear();
			materialLists.clear();
			materials.clear();
		}

		void updateAllMeshesPipelineLayout(std::deque<VkDescriptorSetLayout> setLayouts);
		
		void setFence(VkFence fence) { this->fence = fence; }

		size_t getMeshCount() { return modelMeshes.size(); }

		std::string getMeshName(size_t index);
		

		Mesh& getMesh(size_t idx) { return modelMeshes[idx]; }

		const std::unordered_map<std::string, std::shared_ptr<Material>>& getMaterials() { return materials; }

		void updateDescriptorSet(size_t meshIndex, VkDescriptorSet newDescriptorSet);
		
		void setShadowLayout(VkDescriptorSetLayout setLayout) { shadowSetLayout = setLayout; }


		void updateAllShader();

		void setMeshTexture(size_t meshIndex, const myTextureType texType, const std::string newTexturePath);

		void setAllMeshSpecificTexture(myTextureType texType, const std::string path);

		void reloadMeshShader(size_t meshIndex, bool isVertexShader, const std::string newShaderPath);

		uint32_t getIndexCount(size_t index);
		

		uint32_t getVertexCount(size_t index);
		

		const std::unordered_map<std::string, std::shared_ptr<Material>>& getMaterials() const { return materials; }

		void draw(VkCommandBuffer commandBuffer,PipelineType type,  VkDescriptorSet shadowSet=VK_NULL_HANDLE);

	private:
		struct SmoothKey
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 texCoords;

			bool operator==(const SmoothKey& other) const {
				const float epsilon = 1e-6f;
				return glm::all(glm::epsilonEqual(position, other.position, epsilon)) &&
					glm::all(glm::epsilonEqual(normal, other.normal, epsilon)) &&
					glm::all(glm::epsilonEqual(texCoords, other.texCoords, epsilon));
			}
		};

		struct SmoothKeyHash {
			size_t operator()(const SmoothKey& k) const {
				auto hashFloat = std::hash<float>{};
				size_t h = hashFloat(k.position.x);
				h = h * 31 + hashFloat(k.position.y);
				h = h * 31 + hashFloat(k.position.z);
				h = h * 31 + hashFloat(k.normal.x);
				h = h * 31 + hashFloat(k.normal.y);
				h = h * 31 + hashFloat(k.normal.z);
				h = h * 31 + hashFloat(k.texCoords.x);
				h = h * 31 + hashFloat(k.texCoords.y);
				return h;
			}
		};

		std::string modelName;


		Device& device;
		

		VkDescriptorSetLayout shadowSetLayout;

		std::string directory;
		
		std::vector<std::shared_ptr<Material>> materialLists;
		std::unordered_map<std::string,std::shared_ptr<Material>> materials;
		
		

		VkFence fence;

		
		
		std::vector<Mesh> modelMeshes;
		

		void loadModel(const std::string path);

		void processNode(aiNode* node,const aiScene*& scene);
		Mesh generateMesh(aiMesh*& mesh,const aiScene*& scene,const std::string meshName);

		
		
		
		
		
		

		std::unique_ptr<Buffer> createVertexBuffer(std::vector<Vertex> vertices);
		std::unique_ptr<Buffer> createIndexBuffer(std::vector<uint32_t> indices);

	};
}