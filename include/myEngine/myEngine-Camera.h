#pragma once
#define VK_NO_PROTOTYPES
#include<volk.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace myEngine
{
	enum CameraMovement
	{
		FRONT,
		LEFT,
		RIGHT,
		BACK,
		UP,
		DOWN
	};

	class Camera
	{
	public:
		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;
		Camera() {
			pos = glm::vec3(0, 0, 0);
			front = glm::vec3(0, 0, -1);
			up = glm::vec3(0, 1,0 );
			Zoom = 45.f;
			
		};

		void setCameraInfo(glm::vec3 position , glm::vec3 up );

		glm::vec3 getCameraPos() { return pos; }
		glm::mat4 getViewMat();

		float defaultSpeed = 1;
		float defaultSensitivity=0.1;
		void camraMovementController(CameraMovement moveFlag, float& deltaTime);
		void cameraProccessMouse_CallBack(float xOffset, float yOffset, GLboolean constrainPitch=true);
		void cameraProccessScroll_callBack(float yOff);
		float getZoom() { return Zoom; }

	private:
		glm::quat orientation = glm::quat{ 1,0,0,0 };

		glm::vec3 pos;
		glm::vec3 front;
		glm::vec3 up;
		//glm::vec3 up_World;
		glm::vec3 right;

		float Zoom;

		

		

		
	};
}