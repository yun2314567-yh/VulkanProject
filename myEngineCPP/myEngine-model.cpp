#include"myEngine/myEngine-model.h"
#include"stb_image.h"
#include <iostream>
namespace myEngine
{
	Model::Model(std::string modelName,Device& device, const std::string& path
	             , std::vector<std::shared_ptr<Material>>& materials
		) :
		modelName(modelName),device(device), materialLists(materials)
	{
		
		
		loadModel(path);
	}

	void Model::loadModel(const std::string path)
	{
		Assimp::Importer importer;
		const aiScene* scene=importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals|aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mMeshes)
			throw std::runtime_error("模型未被加载!");

		//获取文件所在目录
		directory = path.substr(0, path.find('/'));

		processNode(scene->mRootNode,scene);
	}

	void Model::draw(VkCommandBuffer commandBuffer,PipelineType type,VkDescriptorSet shadowSet)
	{
     //std::cout << "Model::draw called. mesh count: " << modelMeshes.size() << "\n";
		VkDeviceSize offsets[] = { 0 };

		

		for (int i = 0; i < modelMeshes.size(); i++)
		{
          /*std::cout << " Drawing mesh idx=" << i << " name='" << modelMeshes[i].meshName << "' indexCount=" << modelMeshes[i].indexCount
				<< " vertexBufferSize=" << modelMeshes[i].vertexBuffer->getBufferSize()
				<< " indexBuffer=" << modelMeshes[i].indexBuffer->getBuffer() << " vertexBuffer=" << modelMeshes[i].vertexBuffer->getBuffer()
				<< " descriptorSet=" << modelMeshes[i].textureDescriptorSet << " pipelineLayout=" << pipelineLayout << "\n";
              */
			int count = modelMeshes[i].vertexBuffer->getBufferSize() / sizeof(Vertex);
			
			VkBuffer vertexBuffer = modelMeshes[i].vertexBuffer->getBuffer();
			VkBuffer indexBuffer = modelMeshes[i].indexBuffer->getBuffer();

			modelMeshes[i].material->getShaderManager()->hotReload(fence);
			VkShaderEXT vertexShader;
			VkShaderEXT fragmentShader;
			if (type == SHADOWPASS)
			{
				vertexShader = modelMeshes[i].material->getShaderManager()->getORcompileShader("compiledShaders/shadow_vert.spv", VK_SHADER_STAGE_VERTEX_BIT, {modelMeshes[i].material->getTextureSetLayout() ,modelMeshes[i].material->getUBOSetLayout() });
				fragmentShader = modelMeshes[i].material->getShaderManager()->getORcompileShader("compiledShaders/shadow_frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, { modelMeshes[i].material->getTextureSetLayout() ,modelMeshes[i].material->getUBOSetLayout() });
			}	
			
			else
			{
				vertexShader = modelMeshes[i].material->getShaderManager()->getORcompileShader(modelMeshes[i].material->vertShaderPath, VK_SHADER_STAGE_VERTEX_BIT, { modelMeshes[i].material->getTextureSetLayout() ,modelMeshes[i].material->getUBOSetLayout(),  shadowSetLayout });
				fragmentShader = modelMeshes[i].material->getShaderManager()->getORcompileShader(modelMeshes[i].material->fragShaderPath, VK_SHADER_STAGE_FRAGMENT_BIT, { modelMeshes[i].material->getTextureSetLayout() ,modelMeshes[i].material->getUBOSetLayout(), shadowSetLayout });
			}
			  
			
			//std::cout << "  Shaders: vert=" << vertexShader << " frag=" << fragmentShader << "\n";

			std::array<VkShaderEXT, 2> shaders = { vertexShader,fragmentShader};
			std::array<VkShaderStageFlagBits, 2> stages = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };

			

			vkCmdBindShadersEXT(commandBuffer, 2, stages.data(), shaders.data());
		   vkCmdBindVertexBuffers(commandBuffer,0,1, &vertexBuffer, offsets);
		   //std::cout << "Bound vertex buffer: " << vertexBuffer << ", size: " << modelMeshes[i].vertexBuffer->getBufferSize() << std::endl;
		   vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelMeshes[i].material->getPipelineLayout()
			   ,0,1, &modelMeshes[i].textureDescriptorSet,0,nullptr);
		   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelMeshes[i].material->getPipelineLayout(), 1, 1, modelMeshes[i].material->getCurrentUBOSet(modelName), 0, nullptr);
		   
		   if (shadowSet != VK_NULL_HANDLE)
		   {
			   std::cout << "bind：" << shadowSet << std::endl;
			   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelMeshes[i].material->getPipelineLayout()
				   , 2, 1, &shadowSet, 0, nullptr);
		   }
		   //std::cout << "Executing vkCmdDrawIndexed with count " << modelMeshes[i].indexCount << std::endl;
		   vkCmdDrawIndexed(commandBuffer, modelMeshes[i].indexCount, 1, 0, 0, 0);
		   //vkCmdDraw(commandBuffer, count, 1, 0, 0);
		}
	}

	void Model::processNode(aiNode* node,const aiScene*& scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];//node的Meshes放索引，scene放数据

			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			std::string meshName = material->GetName().C_Str();

			

			modelMeshes.push_back(generateMesh(mesh, scene,meshName));
		}

		for (unsigned i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh Model::generateMesh(aiMesh*& mesh, const aiScene*& scene,const std::string meshName)
	{
		std::vector<Vertex> v;
		std::vector<uint32_t> id;
		
		std::vector<glm::vec3> tangents(mesh->mNumFaces);//按面存
		std::vector<glm::vec3> bitangents(mesh->mNumFaces);

		std::unordered_map<SmoothKey, std::vector<int>, SmoothKeyHash> group;

		//std::unordered_map<unsigned int, glm::vec3> tangents;
		//std::unordered_map<unsigned int, glm::vec3> bitangents;
		//std::unordered_map<unsigned int, int> total;
		for (unsigned i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			glm::vec3 p[3] = {};
			glm::vec2 t[3] = {};
			
			for (unsigned j = 0; j < face.mNumIndices; j++)
			{

				id.push_back(face.mIndices[j]);
				p[j] = glm::vec3{ mesh->mVertices[face.mIndices[j]].x,mesh->mVertices[face.mIndices[j]].y,mesh->mVertices[face.mIndices[j]].z };
				t[j] = glm::vec2{ mesh->mTextureCoords[0][face.mIndices[j]].x,mesh->mTextureCoords[0][face.mIndices[j]].y };
				SmoothKey key = { p[j],glm::vec3{mesh->mNormals[face.mIndices[j]].x,mesh->mNormals[face.mIndices[j]].y,mesh->mNormals[face.mIndices[j]].z},t[j] };

				group[key].push_back(i);
			}

			glm::vec3 dE1 = p[1] - p[0];
			glm::vec3 dE2 = p[2] - p[0];
			glm::vec2 dT1 = t[1] - t[0];
			glm::vec2 dT2 = t[2] - t[0];

			float dU1 = dT1.x, dV1 = dT1.y;
			float dU2 = dT2.x, dV2 = dT2.y;
			float f = dU1 * dV2 - dU2 * dV1;
			
			if (fabs(f) < 1e-8f) {
				tangents[i] = glm::vec3(1.0f, 0.0f, 0.0f);
				bitangents[i] = glm::vec3(0, 1, 0);
				continue;
			}
			
			glm::vec3 T, B;
				
			T = (dV2 * dE1 - dV1 * dE2) / f * glm::length(glm::cross(dE1,dE2));
				
			B = (dU1 * dE2 - dU2 * dE1) / f * glm::length(glm::cross(dE1, dE2));
			

			tangents[i] = T;
			bitangents[i] = B;
			
		}

		

		
		for (unsigned i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex temVertex;
			glm::vec3 vt(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			glm::vec3 nt(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

            temVertex.pos = vt;
			temVertex.normal = nt;

			SmoothKey key;
			key.position = vt;
			key.normal = nt;


			if (mesh->mTextureCoords[0])
			{
				glm::vec2 tt(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

				temVertex.texCoord = tt;
				key.texCoords = tt;
			}
			else
				temVertex.texCoord = glm::vec2(0, 0);	

			auto it = group.find(key);
			if (it == group.end())continue;

			glm::vec3 avg_T{ 0 };
			glm::vec3 avg_B{ 0 };

			for (int j : it->second)
			{
				avg_T += tangents[j];
				avg_B += bitangents[j];
			}

			glm::vec3 N = glm::normalize(nt);
			glm::vec3 T = glm::normalize(avg_T - glm::dot(N, avg_T) * N);
			glm::vec3 B = glm::normalize(glm::cross(N, T));
			float flip = glm::dot(B, avg_B) >= 0.0 ? 1. : -1.;

			temVertex.T = T;
			temVertex.biTangentFilp = flip;

			v.push_back(temVertex);
		}	

		



		/*if (!v.empty()) {
			std::cout << "generateMesh: first vertex pos = (" << v[0].pos.x << ", " << v[0].pos.y << ", " << v[0].pos.z << ")\n";
		}
		std::cout << "generateMesh: indices count = " << id.size() << ", first up to 10: ";
		for (size_t k = 0; k < id.size() && k < 10; ++k) std::cout << id[k] << " ";
		std::cout << "\n";*/
	    
		auto& mat = materialLists[0];

		
		
		VkDescriptorSet descritorSet = {};
		
		Mesh newMesh;

		mat->allocatePool(descritorSet);

		for (auto type : mat->getTextureTypes())
		{
			auto defaultTex = mat->getDefaultTexture(type);
			VkDescriptorImageInfo imageInfo = defaultTex->descriptorImageInfoBuild();
			
			mat->updateTexture(descritorSet, type, imageInfo);
			newMesh.texturePaths[type] = "";
		}

		
		newMesh.meshName = meshName;
		newMesh.vertexBuffer = std::move(createVertexBuffer(v));
		newMesh.indexBuffer = std::move(createIndexBuffer(id));
		newMesh.indexCount = id.size();
		newMesh.material = mat;
		newMesh.textureDescriptorSet = descritorSet;

		materials[meshName] = mat;
		
		


		return newMesh;
		
	}

	

	void Model::setMeshTexture(size_t meshIndex,const myTextureType texType, const std::string newTexturePath)
	{
		if (meshIndex > modelMeshes.size()) throw std::runtime_error("mesh索引超限");

		Mesh& mesh = modelMeshes[meshIndex];

		auto newTexture= mesh.material->getOrLoadTexture(texType,newTexturePath);



		
		VkDescriptorImageInfo newTextureInfo = newTexture->descriptorImageInfoBuild();
		mesh.material->updateTexture(mesh.textureDescriptorSet, texType, newTextureInfo);
		

		mesh.texturePaths[texType] = newTexturePath;
	}

	void Model::updateAllMeshesPipelineLayout(std::deque<VkDescriptorSetLayout> setLayouts)
	{
		
		for (auto& it : modelMeshes)
		{
			
			it.material->createPipelineLayout(setLayouts);
		}
	}

	void Model::setAllMeshSpecificTexture(myTextureType texType, const std::string path)
	{
		for (size_t i = 0; i < modelMeshes.size(); i++)
		{
			setMeshTexture(i, texType, path);
		}
	}

	void Model::updateAllShader()
	{
		for (auto& it : modelMeshes)
		{
			it.material->getShaderManager()->update();
		}
	}

	void Model::reloadMeshShader(size_t meshIndex, bool isVertexShader, const std::string newShaderPath)
	{
		if (meshIndex > modelMeshes.size()) throw std::runtime_error("mesh索引超限");

		Mesh& mesh = modelMeshes[meshIndex];

		VkShaderStageFlagBits stage = isVertexShader ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;

		if (isVertexShader)
			mesh.material->vertShaderPath = newShaderPath;
		else
			mesh.material->fragShaderPath = newShaderPath;


		mesh.material->getShaderManager()->forceReload(newShaderPath, stage, { mesh.material->getUBOSetLayout(),mesh.material->getTextureSetLayout()}, fence);
		
		if (!mesh.texturePaths.empty()) {
			setMeshTexture(meshIndex, DIFFUSE, mesh.texturePaths[DIFFUSE]);
			setMeshTexture(meshIndex, NORMAL, mesh.texturePaths[NORMAL]);
		}
		else {
			// 如果纹理路径为空，强制设为粉红色
			setMeshTexture(meshIndex, DIFFUSE, "");
			setMeshTexture(meshIndex, NORMAL, "");
		}

	}

	std::string Model::getMeshName(size_t index)
	{
		assert(index < modelMeshes.size() && "Mesh index out of bounds");
		return modelMeshes[index].meshName;
	}

	uint32_t Model::getIndexCount(size_t index)
	{
		assert(index < modelMeshes.size() && "Mesh index out of bounds");
		return modelMeshes[index].indexCount;
	}

	uint32_t Model::getVertexCount(size_t index)
	{
		assert(index < modelMeshes.size() && "Mesh index out of bounds");
		return modelMeshes[index].vertexBuffer->getBufferSize() / sizeof(Vertex);
	}


	void Model::updateDescriptorSet(size_t meshIndex, VkDescriptorSet newDescriptorSet)
	{
		assert(meshIndex < modelMeshes.size() && "Mesh index out of bounds");
		modelMeshes[meshIndex].textureDescriptorSet = newDescriptorSet;
	}

	

	

	

	std::unique_ptr<Buffer> Model::createVertexBuffer( std::vector<Vertex> vertices)
	{ 
		VkDeviceSize size = sizeof(vertices[0]) * vertices.size();
		Buffer stageBuffer(device, size, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);
		stageBuffer.mapMemory();
		stageBuffer.writeInBuffer(vertices.data(), size);

		auto vertexBuffer = std::make_unique<Buffer>(device, size, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
		
		device.copyBuffer(stageBuffer.getBuffer(), vertexBuffer->getBuffer(), size);


		return vertexBuffer;
	}

	std::unique_ptr<Buffer> Model ::createIndexBuffer(std::vector<uint32_t> indices)
	{
		VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();


		Buffer stageBuffer(device, indexBufferSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);
		stageBuffer.mapMemory();
		stageBuffer.writeInBuffer(indices.data(), indexBufferSize);

		auto indexBuffer = std::make_unique<Buffer>(device, indexBufferSize, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);

		device.copyBuffer(stageBuffer.getBuffer(), indexBuffer->getBuffer(), indexBufferSize);

		return indexBuffer;
	}

	
}