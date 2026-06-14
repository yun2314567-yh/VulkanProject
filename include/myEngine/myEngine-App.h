#pragma once



#include"myEngine-Renderer.h"
#include"myEngine-device.h"
#include"myEngine-window.h"
#include"myEngine-Descriptor.h"
#include"myEngine-model.h"
#include"myEngine-Camera.h"
#include"myEngine-Gui.h"
#include"myEngine-offScreenRenderer.h"
namespace myEngine
{
class App
{
public:
	
	void setCameraInfo(glm::vec3 position = glm::vec3(0, 0, 0), glm::vec3 up = glm::vec3(0, 1,0 ))
	{
		_camera.setCameraInfo(position, up );
	}

	void setPerspectionMatInfo( float near, float far)
	{
		
		this->near = near;
		this->far = far;
	}

	void setModelInfo(std::string path)
	{
		this->path = path;
	}

	

	bool isDocked = false;
	static constexpr int WIDTH = 1920;//窗口大小（不是渲染模型窗口的大小）
	static constexpr int HEIGHT = 1080;

	App();

	void run();
	

private:
	std::vector<myTextureType> texTypes{ DIFFUSE,NORMAL};
	//std::vector<myTextureType> floorTypes{ DIFFUSE };
	
	float near;//近平面
	float far;//远平面

	bool mouseInView = false;// 鼠标是否在渲染窗口内
	
	bool manualOverride = false;   // 是否处于手动锁定模式
	bool keyToggleProcessed = false;// 用于防止按键切换时的重复触发

	std::string path;//模型路径

	
	bool firstMouse = true;// 用于首次鼠标输入的标志
	Window _window{ WIDTH,HEIGHT,"TEXT" };
	Device _device{ _window };
	Renderer _renderer{ _window,_device };
	Camera _camera;
	
	

	std::shared_ptr<Material> defaultMaterial;
	std::vector<std::unique_ptr<Model>> _models;
	
	std::unique_ptr<offScreenRenderer> shadowRenderer;
	std::unique_ptr<offScreenRenderer> offRenderer;//渲染模型的离屏渲染器
	
	std::unique_ptr<GUI> _gui;

	float deltaTime = 0.f;

	void setMouseState(bool inView);
	void processInput(GLFWwindow* window);

	float lastX = WIDTH / 2;
	float lastY = HEIGHT / 2;
	
	std::string droppedFilePath;
	bool hasdroppedFile = false;
	
	void mouse_CallBack(GLFWwindow* window, double xPos, double yPos);
	static void mouse_CallBack_Static(GLFWwindow* window, double xPos, double yPos);
	void scroll_callBack(GLFWwindow* window, double xOff, double yOff);
	static void  scroll_callBack_Static(GLFWwindow* window, double xOff, double yOff);

	static void frameBufferResizeCallBack(GLFWwindow* window, int width, int height);
	static void drop_CallBack_Static(GLFWwindow* window, int count, const char** paths);

	
	

	
 };

}