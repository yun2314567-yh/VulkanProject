#include "myEngine/myEngine-Camera.h"

namespace myEngine
{
	void Camera::setCameraInfo(glm::vec3 position , glm::vec3 up 
		)
	{

		pos = position;
		
		front = glm::vec3(0,0,-1);
		Zoom = 45.f;
      // normalize and ensure the camera's up vector points upward in world Y
		this->up = glm::normalize(up);
		
	}

	glm::mat4 Camera::getViewMat()
	{
       // use the camera's up vector instead of a hard-coded world up so
		// the camera Y direction is consistent with `pos` updates
		return glm::lookAtRH(pos, pos+front, up);
	}


	void Camera::camraMovementController(CameraMovement moveFlag, float& deltaTime)
	{
		float moveSpeed = 1.0 * defaultSpeed * deltaTime;
		if (moveFlag == FRONT)
			pos += moveSpeed * front;
       if (moveFlag == LEFT)
			pos -= moveSpeed * glm::normalize(glm::cross(front, up));
		if (moveFlag == RIGHT)
			pos += moveSpeed * glm::normalize(glm::cross(front, up));
        if(moveFlag==BACK)
			pos-= moveSpeed * front;
        // 正常语义：UP 增加 Y，DOWN 减少 Y
		if (moveFlag == UP)
			pos.y += moveSpeed;
		if (moveFlag == DOWN)
			pos.y -= moveSpeed;
	}

	

	void Camera::cameraProccessMouse_CallBack(float xOffset, float yOffset, GLboolean constrainPitch)
	{
     xOffset *= -defaultSensitivity;
		yOffset *= defaultSensitivity;

		

		glm::quat deltaY = glm::angleAxis(glm::radians(xOffset), glm::vec3(0, 1,0 ));
		glm::quat deltaX = glm::angleAxis(glm::radians(yOffset), glm::vec3(1, 0, 0));

		orientation = deltaY * orientation*deltaX  ;
		orientation = glm::normalize(orientation);

		front = orientation * glm::vec3(0, 0, -1);
		right = orientation * glm::vec3(1, 0, 0);
		up = orientation * glm::vec3(0, 1, 0);

		//updateCameraPos();
	}

	

	void Camera::cameraProccessScroll_callBack(float yOff)
	{
		Zoom -= yOff;
		if (Zoom <= 1.f)
			Zoom = 1.f;
		if (Zoom >= 100.f)
			Zoom = 100.f;
	}
}