#include"myEngine/myEngine-Gui.h"

namespace myEngine
{
	GUI::GUI(Window& window, Device& device, Renderer& renderer) : _window(window), _device(device), _renderer(renderer)
	{
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000;
		poolInfo.poolSizeCount = std::size(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		if (vkCreateDescriptorPool(_device.getLogicalDevice(), &poolInfo, nullptr, &GUIpool) != VK_SUCCESS)
			throw std::runtime_error("GUI的描述符池创建失败");

		ImGui::CreateContext();

		io = &ImGui::GetIO();

		io->ConfigFlags |= ImGuiConfigFlags_DockingEnable| ImGuiConfigFlags_ViewportsEnable;
		
		ImGui_ImplVulkan_LoadFunctions(
			VK_API_VERSION_1_3,
			[](const char* function_name, void* user_data) -> PFN_vkVoidFunction {
				// 从 Volk 获取函数指针
				return vkGetInstanceProcAddr((VkInstance)user_data, function_name);
			},
			(void*)_device.getInstance()
		);
		
		ImGui_ImplGlfw_InitForVulkan(_window.getWindow(), true);


		


		init_info.Instance = _device.getInstance();
		init_info.Device = _device.getLogicalDevice();
		init_info.PhysicalDevice = _device.getPhysicalDevice();
		init_info.Queue = _device.getGraphicsQueue();
		init_info.QueueFamily = _device.getQueueFamilyIndices().graphicsFamilys;
		init_info.MinImageCount = 3;
		init_info.ImageCount = 3;
		init_info.DescriptorPool = GUIpool;
		init_info.UseDynamicRendering = true;
		init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		VkFormat swapChainFormat = _renderer.getSwapChainImageFormat();
		VkPipelineRenderingCreateInfo renderingCreateInfo = {};
		renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
		renderingCreateInfo.colorAttachmentCount = 1;
		renderingCreateInfo.pColorAttachmentFormats = &swapChainFormat;
		renderingCreateInfo.depthAttachmentFormat = _device.findDepthFormat();
		
		init_info.PipelineInfoMain.PipelineRenderingCreateInfo = renderingCreateInfo;

		ImGui_ImplVulkan_Init(&init_info);

		 
		

		


	}

	VkExtent2D GUI::getCurrentViewSize()
	{
		ImVec2 viewSize;
		if (ImGui::Begin("view", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration))
		{
			viewSize = ImGui::GetContentRegionAvail();
		}
		ImGui::End();

		return { static_cast<uint32_t>(viewSize.x), static_cast<uint32_t>(viewSize.y) };
	}


	void GUI::updateDock()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		
		dockSpaceId = ImGui::GetID("MyDockSpace");
		// 必须每帧调用 DockSpaceOverViewport，传入该 ID
		ImGui::DockSpaceOverViewport(dockSpaceId, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

		
		if (!initialized)
		{
			initialized = true;
			// 清除该ID下已有的节点（如果有）
			ImGui::DockBuilderRemoveNode(dockSpaceId);
			ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockSpaceId, viewport->Size);

			ImGuiID dockLeft, dockRight, dockTop, dockBottom;
			ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.7f, &dockRight, &dockLeft);
			ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Up, 0.5f, &dockTop, &dockBottom);

			ImGui::DockBuilderDockWindow("view", dockRight);
			ImGui::DockBuilderDockWindow("modelInfo", dockTop);
			ImGui::DockBuilderDockWindow("shaderInfo", dockBottom);
			ImGui::DockBuilderFinish(dockSpaceId);
		}
	}

	void GUI::initTextureID(VkSampler sampler, VkImageView imageView)
	{
		if (viewTextureID != VK_NULL_HANDLE)
		{
			
			ImGui_ImplVulkan_RemoveTexture(viewTextureID);
		}
		viewTextureID = ImGui_ImplVulkan_AddTexture(
			sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void GUI::drawModelInfoWindow(Model* model)
	{
		
		if (model == nullptr) {
			
			return;
		}

		ImGui::Begin("modelInfo");
		
		if (model == nullptr) {
			ImGui::Text("No model loaded.");
			ImGui::End();
			return;
		}
		if (_window.getWindow() == nullptr) {
			std::cerr << "GUI::drawModelInfoWindow: Window handle is null!" << std::endl;
			return;
		}
		


		size_t meshCount = model->getMeshCount();
		ImGui::Text("Meshes: %zu", meshCount);

		for (size_t i = 0; i < model->getMeshCount(); ++i) {
			Mesh& mesh = model->getMesh(i);
			std::string label = mesh.meshName.empty() ? ("Mesh " + std::to_string(i)) : mesh.meshName;
			// 使用 TreeNodeEx 并添加 ImGuiTreeNodeFlags_Selected 标志来高亮当前选中的 Mesh
			ImGuiTreeNodeFlags flags = (selectedMeshIndex == i) ? ImGuiTreeNodeFlags_Selected : 0;
			if (ImGui::TreeNodeEx(label.c_str(), flags)) {
				if (ImGui::IsItemClicked()) {
					selectedMeshIndex = i;   // 记录选中
				}

				// --- 路径输入框（保留原有）---
				char vertPath[256];
				strcpy_s(vertPath, mesh.material->vertShaderPath.c_str());
				if (ImGui::InputText("Vertex Shader", vertPath, sizeof(vertPath))) {
					model->reloadMeshShader(i, true, vertPath);
				}
                // Accept drag-drop on Vertex Shader input
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("VERTEX_SHADER_PATH")) {
						if (payload->Data && payload->DataSize > 0) {
							const char* data = reinterpret_cast<const char*>(payload->Data);
							std::string path(data, data + payload->DataSize - 1);
							model->reloadMeshShader(i, true, path);
						}
					}
					ImGui::EndDragDropTarget();
				}
                // If an OS drop is pending and the user dropped onto this input, apply it
				if (pendingDrop && ImGui::IsItemHovered()) {
					std::string fname = std::filesystem::path(pendingDropPath).filename().string();
					if (fname.find("_vert.spv") != std::string::npos) {
						try { model->reloadMeshShader(i, true, pendingDropPath); } catch (...) {}
						pendingDrop = false; pendingDropPath.clear();
					}
				}
				char fragPath[256];
				strcpy_s(fragPath, mesh.material->fragShaderPath.c_str());
				if (ImGui::InputText("Fragment Shader", fragPath, sizeof(fragPath))) {
					model->reloadMeshShader(i, false, fragPath);
				}
				// Accept drag-drop on Fragment Shader input
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FRAGMENT_SHADER_PATH")) {
						if (payload->Data && payload->DataSize > 0) {
							const char* data = reinterpret_cast<const char*>(payload->Data);
							std::string path(data, data + payload->DataSize - 1);
							model->reloadMeshShader(i, false, path);
						}
					}
					ImGui::EndDragDropTarget();
				}
				// If OS drop is pending and hovered apply to fragment shader
				if (pendingDrop && ImGui::IsItemHovered()) {
					std::string fname = std::filesystem::path(pendingDropPath).filename().string();
					if (fname.find("_frag.spv") != std::string::npos) {
						try { model->reloadMeshShader(i, false, pendingDropPath); } catch (...) {}
						pendingDrop = false; pendingDropPath.clear();
					}
				}

				for (auto& it : mesh.texturePaths)
				{
					char texPath[256];
					strcpy_s(texPath, it.second.c_str());
					std::string type =  textureTypeToString(it.first);
					
                    if (ImGui::InputText(type.c_str(), texPath, sizeof(texPath))) {
						std::string ext = std::filesystem::path(texPath).extension().string();
						if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
							model->setMeshTexture(i, it.first, texPath);
					}
					// Accept drag-drop on this texture input to set specific texture type
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_PATH")) {
							if (payload->Data && payload->DataSize > 0) {
								const char* data = reinterpret_cast<const char*>(payload->Data);
								std::string path(data, data + payload->DataSize - 1);
								// validate extension
								std::string ext = std::filesystem::path(path).extension().string();
								if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
									try {
										model->setMeshTexture(i, it.first, path);
									} catch (const std::exception &e) {
										std::cerr << "Failed to set texture from drop: " << e.what() << std::endl;
									}
								}
							}
						}
						ImGui::EndDragDropTarget();
					}
                    // If OS drop is pending and this texture input is hovered, apply it to this texture type
					if (pendingDrop && ImGui::IsItemHovered()) {
						std::string fname = std::filesystem::path(pendingDropPath).filename().string();
						std::string ext = std::filesystem::path(pendingDropPath).extension().string();
						if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
							try { model->setMeshTexture(i, it.first, pendingDropPath); } catch (...) {}
							pendingDrop = false; pendingDropPath.clear();
						}
					}
				}
				

				ImGui::TreePop();
			}
		}
		ImGui::End();
	}

	

	void GUI::drawViewWindow(VkExtent2D extent)
	{
		
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1)); // 纯黑背景，完全不透明
		ImGui::Begin("view", nullptr, ImGuiWindowFlags_NoScrollbar);
	    
		if (viewTextureID) {
			ImVec2 avail = ImGui::GetContentRegionAvail();
			
			std::cout << "FrameBuffer: " << io->DisplayFramebufferScale.x << " " << io->DisplayFramebufferScale.y << std::endl;
			std::cout << "avail: " << avail.x << " " << avail.y << std::endl;
			ImGui::Image((ImTextureID)viewTextureID, ImVec2(extent.width / io->DisplayFramebufferScale.x, extent.height / io->DisplayFramebufferScale.y));
		}
		else {
			ImGui::Text("No texture");
		}
		
		ImGui::End();
		
		m_viewWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)
			&& ImGui::GetHoveredID() == (ImGuiID)viewTextureID;

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}

	void GUI::drawShaderInfoWindow()
	{
		
		ImGui::Begin("shaderInfo");
		ImGui::Text("ShaderINFO");
		ImGui::End();
	}

	
	
	void GUI::newFrame(bool& hasdroppedFile,std::string & droppedFilePath,Model* model)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		if (model == nullptr) return;

        if (hasdroppedFile && !droppedFilePath.empty()) {
			
			// 存储拖拽信息，等待用户将其拖到对应输入框时再应用
			pendingDrop = true;
			pendingDropPath = droppedFilePath;
			// 根据文件扩展名自动判断类型（可选）
			std::string filename = std::filesystem::path(droppedFilePath).filename().string();
			std::string payloadType;
			if (filename.find("_vert.spv") != std::string::npos) {
				payloadType = "VERTEX_SHADER_PATH";
			}
			else if (filename.find("_frag.spv") != std::string::npos) {
				payloadType = "FRAGMENT_SHADER_PATH";
			}
			else {
				// 其他文件作为纹理处理（可以扩展图片格式判断）
				payloadType = "TEXTURE_PATH";
			}

			// 设置拖拽源
			if (ImGui::BeginDragDropSource(0)) {
				ImGui::SetDragDropPayload(payloadType.c_str(), droppedFilePath.c_str(), droppedFilePath.size() + 1);
				ImGui::Text("%s", droppedFilePath.c_str());  // 拖拽时显示的文件名
				ImGui::EndDragDropSource();
			}

            
			// 让用户拖拽到对应输入框时才应用，避免误操作
			hasdroppedFile = false;
			droppedFilePath.clear();
		}
	}

	void GUI::render(VkCommandBuffer command)
	{
		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();
		if (!drawData) throw std::runtime_error("gui无绘制数据");

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command);
	}


	std::string GUI::textureTypeToString(myTextureType texType)
	{
		static const char* names[] = {
		"DIFFUSE_Texture",
		"NORMAL_Texture",
		"ROUGHNESS_Texture",
		"SPECULAR_Texture",
		"HEIGHT_Texture",
		"MAX_TEXTURE_COUNT"
		};
		uint32_t idx = static_cast<uint32_t>(texType);
		if (idx <= static_cast<uint32_t>(myTextureType::MAX_TEXTURE_COUNT)) {
			return names[idx];
		}
		return "Unknown";
	}
}