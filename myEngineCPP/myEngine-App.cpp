#include<cassert>
#include<chrono>
#include"myEngine/myEngine-App.h"
#include"myEngine/myEngine-buffer.h"
#include"myEngine/myEngine-RenderSystem.h"
namespace myEngine
{
	


	App::App()
	{
		

		
		glfwSetWindowUserPointer(_window.getWindow(), this);

		glfwSetCursorPosCallback(_window.getWindow(), mouse_CallBack_Static);//鼠标

		glfwSetScrollCallback(_window.getWindow(), scroll_callBack_Static);//滚轮

		glfwSetFramebufferSizeCallback(_window.getWindow(), frameBufferResizeCallBack);//窗口大小改变

		glfwSetDropCallback(_window.getWindow(), drop_CallBack_Static);//拖拽输入
		
		setMouseState(false);//切换鼠标状态，默认显示鼠标，按Tab键切换，然后可以操作视角
		
		_gui = std::make_unique<GUI>(_window, _device, _renderer);//创建UI系统

		

	}


	void App::run()
	{
		
		//材质初始化
		defaultMaterial = std::make_shared<Material>(_device, texTypes);


		//std::cout << "App::run: textureSetLayout = " << textureSetLayout->getDescriptorSetLayout() << "\n";
		
		//深度纹理设置
		

		//深度纹理描述符设置
		auto shadowPool = DescriptorPool::Builder(_device)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMEBUFFERS_IN_SAMETIME)
			.setmaxSets(MAX_FRAMEBUFFERS_IN_SAMETIME+1)
			.setFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
			.build();
		auto shadowLayout = DescriptorSetLayout::Builder(_device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.setBindingFlags(std::vector<myTextureType>{0}, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
			.build();
		DescriptorWriter shadowLayoutWriter(*shadowLayout, *shadowPool);

		VkDescriptorSet shadowSet[3];

		
		
		
		
		//用于渲染模型
		offRenderer = std::make_unique<offScreenRenderer>(_device, _window.getExtent2D(), VK_FORMAT_R8G8B8A8_SRGB,false);
		offRenderer->setAndCreateDepthImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		offRenderer->setAndCreateSampler();
		//用于渲染阴影贴图
		shadowRenderer = std::make_unique<offScreenRenderer>(_device,VkExtent2D{ 2560,2560 }, VK_FORMAT_R8G8B8A8_SRGB, true);
		shadowRenderer->setAndCreateDepthImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT| VK_IMAGE_USAGE_SAMPLED_BIT);
		shadowRenderer->setAndCreateSampler(VK_FILTER_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_FALSE, VK_COMPARE_OP_NEVER);
		
		VkDescriptorImageInfo shadowImage = { shadowRenderer->getSampler(),shadowRenderer->getDepthImageView(),VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		for (int i = 0; i < 3; i++)
		{
			
			shadowLayoutWriter.writeImage(0, &shadowImage);
			shadowLayoutWriter.build(shadowSet[i]);
		}
		

		//渲染系统
		RenderSystem _renderSystem(_device);

		//将离屏渲染器的输出绑定到GUI上
		_gui->initTextureID(offRenderer->getSampler(), offRenderer->getColorImageView());


		//模型和UBO设置
		UBO model_ubo,ubo_floor;
		
		std::vector<std::shared_ptr<Material>> materials{ defaultMaterial };
		
		std::unique_ptr<Model>_model = std::make_unique<Model>("model",_device, path, materials);
		std::unique_ptr<Model> floor = std::make_unique<Model>("floor",_device, "floor.obj", materials);
		
		

		_models.push_back(std::move(_model));
		_models.push_back(std::move(floor));

		
		auto currentTime = std::chrono::high_resolution_clock::now();
		
		model_ubo.model=glm::rotate(model_ubo.model, glm::radians(180.f), glm::vec3(0, 0, -1));
		//ubo_floor.model= glm::rotate(ubo_floor.model, glm::radians(180.f), glm::vec3(0, 0, -1));

		//主循环
		while (!_window.shouldBeClosed())
		{
			
			auto newTime = std::chrono::high_resolution_clock::now();

			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();

			currentTime = newTime;
			deltaTime = frameTime;
			
			//光照信息设置
			model_ubo.lightPos = glm::vec3(-2,6, 0);

			
			model_ubo.lightView = glm::lookAtRH(model_ubo.lightPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			model_ubo.lightProj = glm::orthoRH_ZO(-4.f, 4.f, -4.f, 4.f, near, 20.f);
			//model_ubo.lightProj[1][1] *= -1.f;
			ubo_floor.lightPos = model_ubo.lightPos;
			ubo_floor.lightView = model_ubo.lightView;
			ubo_floor.lightProj = model_ubo.lightProj;
			
			//处理外部输入
			processInput(_window.getWindow());

			
			//UI系统更新，包括处理拖拽输入等
			_gui->newFrame( hasdroppedFile, droppedFilePath,_models[0].get());
			

			//设置子窗口：模型渲染，模型信息，着色器信息
			_gui->updateDock();
			
			
			//更新模型渲染窗口大小，如果窗口大小改变了，就重建离屏渲染器
			VkExtent2D viewSize = _gui->getCurrentViewSize();

			if (viewSize.width != offRenderer->getExtent().width ||
				viewSize.height != offRenderer->getExtent().height)
			{
				vkDeviceWaitIdle(_device.getLogicalDevice());

				offRenderer = std::make_unique<offScreenRenderer>(
					_device, viewSize, VK_FORMAT_R8G8B8A8_SRGB);
				offRenderer->setAndCreateDepthImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
				offRenderer->setAndCreateSampler();

				shadowRenderer = std::make_unique<offScreenRenderer>(_device, VkExtent2D{ 2560,2560 }, VK_FORMAT_R8G8B8A8_SRGB, true);
				shadowRenderer->setAndCreateDepthImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
				shadowRenderer->setAndCreateSampler(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_FALSE, VK_COMPARE_OP_NEVER);

				_gui->initTextureID(offRenderer->getSampler(), offRenderer->getColorImageView());
			}
			
			//model_ubo.proj = glm::orthoRH_ZO(-20.f, 20.f, -20.f, 20.f, 0.1f, 30.f);
			model_ubo.proj = glm::perspectiveRH_ZO(glm::radians(_camera.getZoom()), viewSize.width / float(viewSize.height), near, far);
			model_ubo.proj[1][1] *= -1.f;
			model_ubo.camPos = _camera.getCameraPos();
			
			ubo_floor.camPos = model_ubo.camPos;
			ubo_floor.proj = model_ubo.proj;

			//开始帧
			if (auto commandBuffer = _renderer.beginFrame())
			{
				
				int frameIndex = _renderer.getCurrentFrameIndex();

				
				model_ubo.view = _camera.getViewMat();
				//model_ubo.view = glm::lookAtRH(model_ubo.lightPos+glm::vec3(0,20*glm::sin(glm::radians(glfwGetTime())), 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
				ubo_floor.view = model_ubo.view;
				if (!defaultMaterial) {
					throw std::runtime_error("defaultMaterial is null before updateUBO");
				}

                // Ensure UBOs are passed to the correct named sets: model -> model_ubo, floor -> ubo_floor
				defaultMaterial->updateUBO("model", frameIndex, &model_ubo);
				defaultMaterial->updateUBO("floor", frameIndex, &ubo_floor);
				

				
				
				//std::cout << "camPos" << model_ubo.camPos.x <<" " << model_ubo.camPos.y <<" "<< model_ubo.camPos.z << std::endl;
				//传递fence，用于shader的热重载
				for (auto& it : _models)
				{
					it->setFence(_renderer.getSwapChainCurrentFence());
					//_model->getShaderManager().hotReload(_renderer.getSwapChainCurrentFence());
				}
				
				//阴影pass

				_device.transitionImageLayout(shadowRenderer->getDepthImage(), shadowRenderer->getDepthImageFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,commandBuffer);
				shadowRenderer->beginPass(commandBuffer);				

				for (auto& it : _models)
				{
					it->updateAllMeshesPipelineLayout({ });
					_renderSystem.renderModel(SHADOWPASS, commandBuffer,  it.get());
				}

				shadowRenderer->endPass(commandBuffer);

				_device.transitionImageLayout(shadowRenderer->getDepthImage(), shadowRenderer->getDepthImageFormat(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,commandBuffer);

				shadowImage = { shadowRenderer->getSampler(),shadowRenderer->getDepthImageView(),VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
				shadowLayoutWriter.writeImage(0, &shadowImage);
				shadowLayoutWriter.update(shadowSet[frameIndex]);
				


				//正式pass
				_device.transitionImageLayout(offRenderer->getColorImage(), offRenderer->getColorImageFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);
				
				offRenderer->beginPass(commandBuffer);
				//std::cout << "shadow: " << shadowSet[frameIndex] << std::endl;
				for (auto& it : _models)
				{
					it->updateAllMeshesPipelineLayout({shadowLayout->getDescriptorSetLayout()});
					it->setShadowLayout(shadowLayout->getDescriptorSetLayout());
					_renderSystem.renderModel(DEFAULT, commandBuffer,  it.get(),shadowSet[frameIndex]);
				}
				

				offRenderer->endPass(commandBuffer);
				_device.transitionImageLayout(offRenderer->getColorImage(), offRenderer->getColorImageFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);
				

				//绘制三个子窗口：模型渲染，模型信息，着色器信息
				_gui->drawViewWindow(viewSize);
				_gui->drawModelInfoWindow(_models[0].get());
				_gui->drawShaderInfoWindow();

				
				//主窗口渲染
				_device.transitionImageLayout(_renderer.getSwapChainCurrentImage(), _renderer.getSwapChainImageFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);

				_renderer.beginRenderPass(commandBuffer);
				
				_gui->render(commandBuffer);

				_renderer.endRenderPass(commandBuffer);

				_device.transitionImageLayout(_renderer.getSwapChainCurrentImage(), _renderer.getSwapChainImageFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, commandBuffer);
				_renderer.endFrame();
				
				

				//每帧结束时更新模型的着色器，检查是否有热重载的shader，如果有就更新
				for (auto& it : _models)
				{
					it->updateAllShader();
				}
			}

			
			glfwPollEvents();

		}

		vkDeviceWaitIdle(_device.getLogicalDevice());
		        
		_models.clear(); 
		offRenderer.reset();
		_gui.reset();
				
		defaultMaterial.reset();
			
		glfwTerminate();
	}
	
	


	void App::frameBufferResizeCallBack(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
		if (app)
		{
			app->_window.setWindowSize(width, height);
			app->_window.setWindowResize(true);
		}
	}

	void App::setMouseState(bool inView)
	{
		if(inView) glfwSetInputMode(_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		else glfwSetInputMode(_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		mouseInView = inView;

		if (inView) {
			glfwSetCursorPos(_window.getWindow(), lastX, lastY);
		}
	}
	
	

	void App::mouse_CallBack_Static(GLFWwindow* window, double xPos, double yPos)
	{
		App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
		if (app)
			app->mouse_CallBack(window, xPos, yPos);
	}

	void App::processInput(GLFWwindow* window)
	{
		float cameraSpeed = 20.5f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			_camera.camraMovementController(FRONT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			_camera.camraMovementController(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			_camera.camraMovementController(RIGHT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			_camera.camraMovementController(BACK, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			_camera.camraMovementController(UP, deltaTime);
		
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			_camera.camraMovementController(DOWN, deltaTime);


		if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !keyToggleProcessed) {
			manualOverride = !manualOverride;          // 切换手动模式
			if (manualOverride) {
				
				setMouseState(true);
			}
			else {
				
				setMouseState(false);// 立即根据当前 UI 状态决定是否释放
			}
			keyToggleProcessed = true;
		}
		if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) {
			keyToggleProcessed = false;
		}

	}

	void App::mouse_CallBack(GLFWwindow* window, double xPos, double yPos)
	{
		if (!mouseInView) return;
		float xpos = static_cast<float>(xPos);
		float ypos = static_cast<float>(yPos);
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xOffset = xpos - lastX;
		float yOffset = lastY - ypos;
		lastX = xPos;
		lastY = yPos;

		_camera.cameraProccessMouse_CallBack(xOffset,yOffset);
	}
	
	void App:: scroll_callBack_Static(GLFWwindow* window, double xOff, double yOff)
	{
		App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
		if (app)
			app->scroll_callBack(window, xOff, yOff);
	}

	void App::scroll_callBack(GLFWwindow* window, double xOff, double yOff)
	{
		_camera.cameraProccessScroll_callBack(static_cast<float>(yOff));
	}

	void App::drop_CallBack_Static(GLFWwindow* window, int count, const char** paths)
	{
		App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
		if (!app || count == 0)return;

		// For GUI-targeted handling, store pending drop info and let GUI consume it when
		// the user hovers the appropriate input element. This allows dropping onto a
		// specific texture field in the modelInfo window.
		app->droppedFilePath = std::string(paths[0]);
		app->hasdroppedFile = true;

	}
}